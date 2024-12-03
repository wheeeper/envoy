#pragma once

#include "rb_lookup_context.h"
#include "source/extensions/filters/http/cache/http_cache.h"
#include "envoy/extensions/http/cache/ring_buffer_http_cache/v3/ring_buffer_http_cache.pb.h"
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "source/extensions/http/cache/ring_buffer_http_cache/ring_buffer.hpp"
#include "source/extensions/http/cache/ring_buffer_http_cache/response_data.hpp"
#include "source/extensions/http/cache/ring_buffer_http_cache/rb_cache_key.hpp"
#include "source/extensions/http/cache/ring_buffer_http_cache/rb_lookup_context.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

using RbCacheProtoConfig =
    envoy::extensions::http::cache::ring_buffer_http_cache::v3::RingBufferHttpCacheConfig;

class RingBufferHttpCache : public HttpCache, public Singleton::Instance {
public:
  constexpr static absl::string_view extension_name = "envoy.extensions.http.cache.ring_buffer_http_cache";

  RingBufferHttpCache(RbCacheProtoConfig config);

  // From HttpCache
  LookupContextPtr makeLookupContext(LookupRequest&& request,
                                     Http::StreamFilterCallbacks& callbacks) override;

  // From HttpCache
  InsertContextPtr makeInsertContext(LookupContextPtr&& lookup_context,
                                     Http::StreamFilterCallbacks& callbacks) override;

  // From HttpCache
  void updateHeaders(const LookupContext& lookup_context,
                     const Http::ResponseHeaderMap& response_headers,
                     const ResponseMetadata& metadata, UpdateHeadersCallback on_complete) override;

  // From HttpCache
  CacheInfo cacheInfo() const override;

  // Lookups entry in cache
  // Return entry if found, otherwise nullopt
  absl::optional<ResponseData> lookup(const LookupRequest& request);

  bool insert(const RbLookupContext& lookup_context, ResponseData&& cache_entry);

  const RbCacheProtoConfig& getConfig() const;

private:

  using RingBufferType = RingBuffer<std::pair<RbCacheKey, ResponseData>>;

  using CompareFuncType = std::function<bool(RbCacheKey, RbCacheKey)>;

  absl::optional<size_t> searchBuffer(const RbCacheKey& key, CompareFuncType comparator);

  absl::optional<std::pair<RbCacheKey, size_t>> getCacheEntry(const LookupRequest& request);

  absl::optional<std::string> getVaryId(const LookupRequest& request,
                                        const ResponseData& response_headers);
  RingBufferType ring_buffer_;

  RbCacheProtoConfig config_;

  absl::Mutex mutex_;
};

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy