#include "source/extensions/http/cache/ring_buffer_http_cache/ring_buffer_http_cache.h"
#include "envoy/extensions/http/cache/ring_buffer_http_cache/v3/ring_buffer_http_cache.pb.h"
#include "envoy/registry/registry.h"


namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {

constexpr absl::string_view Name = "envoy.extensions.http.cache.ring_buffer_http_cache";

CacheInfo RingBufferHttpCache::cacheInfo() const {
  CacheInfo cache_info;
  cache_info.name_ = Name;
  return cache_info;
}

SINGLETON_MANAGER_REGISTRATION(simple_http_cache_singleton);

class SimpleHttpCacheFactory : public HttpCacheFactory {
public:
  // From UntypedFactory
  std::string name() const override { return std::string(Name); }
  // From TypedFactory
  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<
        envoy::extensions::http::cache::ring_buffer_http_cache::v3::RingBufferHttpCacheConfig>();
  }
  // From HttpCacheFactory
  std::shared_ptr<HttpCache>
  getCache(const envoy::extensions::filters::http::cache::v3::CacheConfig&,
           Server::Configuration::FactoryContext& context) override {
    return context.serverFactoryContext().singletonManager().getTyped<RingBufferHttpCache>(
        SINGLETON_MANAGER_REGISTERED_NAME(simple_http_cache_singleton), &createCache);
  }

private:
  static std::shared_ptr<Singleton::Instance> createCache() {
    return std::make_shared<RingBufferHttpCache>();
  }
};

static Registry::RegisterFactory<SimpleHttpCacheFactory, HttpCacheFactory> register_;

} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
