
#include "source/extensions/http/cache/ring_buffer_http_cache/rb_lookup_context.h"
#include "source/extensions/http/cache/ring_buffer_http_cache/ring_buffer_http_cache.h"
#include "source/common/buffer/buffer_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

RbLookupContext::RbLookupContext(RingBufferHttpCache& cache, Event::Dispatcher& dispatcher,
                                 LookupRequest&& request)
    : cache_(cache), dispatcher_(dispatcher), request_(std::move(request)) {}

RbLookupContext::~RbLookupContext() { onDestroy(); }

void RbLookupContext::getHeaders(LookupHeadersCallback&& cb) {
  // Lookup in cache
  // If succesfull, returns copy of RbCacheEntry
  auto lookupRes = cache_.lookup(request_);

  LookupResult result;

  if (lookupRes) {

    auto entry = lookupRes.value();

    body_ = std::move(entry.body);
    trailers_ = std::move(entry.trailers);

    result = entry.response_headers
                 ? request_.makeLookupResult(std::move(entry.response_headers),
                                             std::move(entry.metadata), body_.size())
                 : LookupResult{};
  }

  bool end_stream = body_.empty() && trailers_ == nullptr;

  dispatcher_.post([result = std::move(result), cb = std::move(cb), end_stream,
                    cancelled = cancelled_]() mutable {
    if (!*cancelled) {
      std::move(cb)(std::move(result), end_stream);
    }
  });
}

void RbLookupContext::getBody(const AdjustedByteRange& range, LookupBodyCallback&& cb) {
  ASSERT(range.end() <= body_.length(), "Attempt to read past end of body.");
  auto result = std::make_unique<Buffer::OwnedImpl>(&body_[range.begin()], range.length());
  bool end_stream = trailers_ == nullptr && range.end() == body_.length();
  dispatcher_.post([result = std::move(result), cb = std::move(cb), end_stream,
                    cancelled = cancelled_]() mutable {
    if (!*cancelled) {
      std::move(cb)(std::move(result), end_stream);
    }
  });
}

void RbLookupContext::getTrailers(LookupTrailersCallback&& cb) {
  ASSERT(trailers_);
  dispatcher_.post(
      [cb = std::move(cb), trailers = std::move(trailers_), cancelled = cancelled_]() mutable {
        if (!*cancelled) {
          std::move(cb)(std::move(trailers));
        }
      });
}

void RbLookupContext::onDestroy() { *cancelled_ = true; }

Event::Dispatcher& RbLookupContext::getDispatcher() const { return dispatcher_; }

RingBufferHttpCache& RbLookupContext::getCache() const { return cache_; }

const LookupRequest& RbLookupContext::getReqeuest() const { return request_; }

Key RbLookupContext::getKey() const { return request_.key(); }

std::shared_ptr<bool> RbLookupContext::isCancelled() { return cancelled_; }

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy