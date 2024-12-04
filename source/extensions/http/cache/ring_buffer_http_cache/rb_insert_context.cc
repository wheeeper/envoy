#include "source/extensions/http/cache/ring_buffer_http_cache/rb_insert_context.h"
#include "rb_lookup_context.h"
#include "source/extensions/http/cache/ring_buffer_http_cache/ring_buffer_http_cache.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

RbInsertContext::RbInsertContext(RingBufferHttpCache& cache, Event::Dispatcher& dispatcher,
                                 LookupContextPtr&& lookup_context)
    : cache_(cache), dispatcher_(dispatcher), lookup_context_(std::move(lookup_context)){};

RbInsertContext::~RbInsertContext() { onDestroy(); }

void RbInsertContext::insertHeaders(const Http::ResponseHeaderMap& response_headers,
                                    const ResponseMetadata& metadata, InsertCallback insert_success,
                                    bool end_stream) {
  ASSERT(!committed_);
  cache_.notifyInsertStart(static_cast<const RbLookupContext&>(*lookup_context_).getKey());
  cache_entry_.response_headers =
      Http::createHeaderMap<Http::ResponseHeaderMapImpl>(response_headers);
  cache_entry_.metadata = metadata;
  if (end_stream) {
    post(std::move(insert_success), commit());
  } else {
    post(std::move(insert_success), true);
  }
}

void RbInsertContext::insertBody(const Buffer::Instance& chunk, InsertCallback ready_for_next_chunk,
                                 bool end_stream) {
  ASSERT(!committed_);
  ASSERT(ready_for_next_chunk || end_stream);

  body_buffer_.add(chunk);
  if (end_stream) {
    post(std::move(ready_for_next_chunk), commit());
  } else {
    post(std::move(ready_for_next_chunk), true);
  }
}

void RbInsertContext::insertTrailers(const Http::ResponseTrailerMap& trailers,
                                     InsertCallback insert_complete) {
  ASSERT(!committed_);
  cache_entry_.trailers = Http::createHeaderMap<Http::ResponseTrailerMapImpl>(trailers);
  post(std::move(insert_complete), commit());
}

void RbInsertContext::onDestroy() { *cancelled_ = true; }

void RbInsertContext::post(InsertCallback cb, bool result) {
  dispatcher_.post([cb = std::move(cb), result = result, cancelled = cancelled_]() mutable {
    if (!*cancelled) {
      std::move(cb)(result);
    }
  });
}

bool RbInsertContext::commit() {
  cache_entry_.body = body_buffer_.toString();
  const auto& rb_lookup_context = static_cast<const RbLookupContext&>(*lookup_context_);
  auto result = cache_.insert(rb_lookup_context, std::move(cache_entry_));
  cache_.notifyInsertEnd(rb_lookup_context.getKey());
  return result;
}

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy