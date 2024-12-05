#pragma once

#include "source/extensions/filters/http/cache/http_cache.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {


class RingBufferHttpCache;

class RbLookupContext : public LookupContext {
public:
  RbLookupContext(RingBufferHttpCache& cache, Event::Dispatcher& dispatcher,
                  LookupRequest&& request);

  ~RbLookupContext() override;

  // from LookupContext
  void getHeaders(LookupHeadersCallback&& cb) override;

  // from LookupContext
  void getBody(const AdjustedByteRange& range, LookupBodyCallback&& cb) override;

  // from LookupContext
  void getTrailers(LookupTrailersCallback&& cb) override;

  // from LookupContext
  void onDestroy() override;

  Event::Dispatcher& getDispatcher() const;

  RingBufferHttpCache& getCache() const;

  const LookupRequest& getReqeuest() const;

  Key getKey() const;

  std::shared_ptr<bool> isCancelled();

private:
  RingBufferHttpCache& cache_;

  Event::Dispatcher& dispatcher_;

  LookupRequest request_;

  std::shared_ptr<bool> cancelled_ = std::make_shared<bool>(false);

  std::string body_;

  Http::ResponseTrailerMapPtr trailers_ = nullptr;
};

using RbLookupContextPtr = std::unique_ptr<RbLookupContext>;

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy