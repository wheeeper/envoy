#include "source/extensions/http/cache/ring_buffer_http_cache/ring_buffer_http_cache.h"
#include "envoy/extensions/http/cache/ring_buffer_http_cache/v3/ring_buffer_http_cache.pb.h"
#include "envoy/registry/registry.h"
#include "rb_cache_key.hpp"
#include "rb_insert_context.h"
#include "rb_lookup_context.h"
#include <memory>
#include <optional>

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

RingBufferHttpCache::RingBufferHttpCache(RbCacheProtoConfig config)
    : ring_buffer_(static_cast<size_t>(config.cache_size())), config_(config) {}

CacheInfo RingBufferHttpCache::cacheInfo() const {
  CacheInfo cache_info;
  cache_info.name_ = RingBufferHttpCache::extension_name;
  return cache_info;
}

const RbCacheProtoConfig& RingBufferHttpCache::getConfig() const { return config_; }

LookupContextPtr RingBufferHttpCache::makeLookupContext(LookupRequest&& request,
                                                        Http::StreamFilterCallbacks& callbacks) {
  return std::make_unique<RbLookupContext>(*this, callbacks.dispatcher(), std::move(request));
}

InsertContextPtr RingBufferHttpCache::makeInsertContext(LookupContextPtr&& lookup_context,
                                                        Http::StreamFilterCallbacks& callbacks) {
  return std::make_unique<RbInsertContext>(*this, callbacks.dispatcher(),
                                           std::move(lookup_context));
}

void RingBufferHttpCache::updateHeaders(const LookupContext& lookup_context,
                                        const Http::ResponseHeaderMap& response_headers,
                                        const ResponseMetadata& metadata,
                                        UpdateHeadersCallback on_complete) {

  const auto& rb_lookup_context = static_cast<const RbLookupContext&>(lookup_context);

  auto post_complete = [on_complete = std::move(on_complete),
                        &dispatcher = rb_lookup_context.getDispatcher()](bool result) mutable {
    dispatcher.post([on_complete = std::move(on_complete), result]() mutable {
      std::move(on_complete)(result);
    });
  };

  std::optional<std::pair<RbCacheKey, size_t>> lookup_result = std::nullopt;

  {
    absl::ReaderMutexLock read_lock(&mutex_);
    lookup_result = getCacheEntry(rb_lookup_context.getReqeuest());

    if (!lookup_result || !ring_buffer_.get(lookup_result.value().second).second.response_headers) {
      std::move(post_complete)(false);
      return;
    }
  }

  auto& entry_info = lookup_result.value();
  absl::WriterMutexLock write_lock(&mutex_);

  if (ring_buffer_.get(entry_info.second).first != entry_info.first) {
    std::move(post_complete)(false);
    return;
  }

  auto& cache_entry = ring_buffer_.get(entry_info.second).second;
  applyHeaderUpdate(response_headers, *cache_entry.response_headers);
  cache_entry.metadata = metadata;
  std::move(post_complete)(true);
}

absl::optional<std::pair<RbCacheKey, size_t>>
RingBufferHttpCache::getCacheEntry(const LookupRequest& request) {

  mutex_.AssertReaderHeld();

  RbCacheKey key(request.key());
  auto result = searchBuffer(key, RbCacheKey::compareOriginalKeys);

  if (result &&
      VaryHeaderUtils::hasVary(*ring_buffer_.get(result.value()).second.response_headers)) {

    auto vary_id = getVaryId(request, ring_buffer_.get(result.value()).second);

    if (!vary_id) {
      result = absl::nullopt;
    } else {
      key.vary_id = vary_id.value();
      result = searchBuffer(key, RbCacheKey::compareWholeKeys);
    }
  }

  return result ? absl::make_optional(std::make_pair(key, result.value())) : absl::nullopt;
}

absl::optional<std::string> RingBufferHttpCache::getVaryId(const LookupRequest& request,
                                                           const ResponseData& cached_entry) {

  auto vary_header_values = VaryHeaderUtils::getVaryValues(*(cached_entry.response_headers));
  ASSERT(!vary_header_values.empty());
  return VaryHeaderUtils::createVaryIdentifier(request.varyAllowList(), vary_header_values,
                                               request.requestHeaders());
}

absl::optional<size_t> RingBufferHttpCache::searchBuffer(const RbCacheKey& key,
                                                         CompareFuncType comparator) {
  mutex_.AssertReaderHeld();

  for (size_t i = 0; i < ring_buffer_.size(); ++i) {
    if (comparator(key, ring_buffer_.get(i).first)) {
      return absl::make_optional(i);
    }
  }
  return absl::nullopt;
}

absl::optional<ResponseData> RingBufferHttpCache::lookup(const LookupRequest& request) {
  absl::ReaderMutexLock lock(&mutex_);
  auto lookup_result = getCacheEntry(request);

  if (!lookup_result) {
    return absl::nullopt;
  }

  auto entry = ring_buffer_.get(lookup_result.value().second).second;
  return entry;
}

bool RingBufferHttpCache::insert(const RbLookupContext& lookup_context,
                                 ResponseData&& cache_entry) {

  RbCacheKey key(lookup_context.getKey());

  if (VaryHeaderUtils::hasVary(*cache_entry.response_headers)) {
    auto vary_id = getVaryId(lookup_context.getReqeuest(), cache_entry);
    key.vary_id = vary_id;
  }

  absl::WriterMutexLock write_lock(&mutex_);

  auto lookup_result = searchBuffer(key, RbCacheKey::compareWholeKeys);

  if (lookup_result) {
    ring_buffer_.get(lookup_result.value()).second = std::move(cache_entry);
  } else {
    if (ring_buffer_.full()) {
      ring_buffer_.pop();
    }

    ring_buffer_.push(std::make_pair(std::move(key), std::move(cache_entry)));
  }

  return true;
}

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
