
#include "source/extensions/filters/http/cache/http_cache.h"
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {

class RingBufferHttpCache : public HttpCache, public Singleton::Instance {
private:
public:
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
};

} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy