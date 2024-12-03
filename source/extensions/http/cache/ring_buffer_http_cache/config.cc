
#include "source/extensions/http/cache/ring_buffer_http_cache/ring_buffer_http_cache.h"
#include "source/extensions/filters/http/cache/http_cache.h"
#include "absl/synchronization/mutex.h"
#include <memory>

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

class CacheManager : public Singleton::Instance {
public:
  CacheManager() = default;

  std::shared_ptr<RingBufferHttpCache> get(RbCacheProtoConfig config) {

    std::shared_ptr<RingBufferHttpCache> cache;
    const auto& key = config.cache_name();
    {
      absl::ReaderMutexLock read_lock(&mutex_);
      auto it = caches_.find(key);
      if (it != caches_.end()) {
        cache = it->second.lock();
      }
    }

    if (!cache) {
      cache = std::make_shared<RingBufferHttpCache>(config);
      {
        absl::WriterMutexLock write_lock(&mutex_);
        caches_[key] = cache;
      }
    } else if (!Protobuf::util::MessageDifferencer::Equals(cache->getConfig(), config)) {
      throw EnvoyException(fmt::format("RingBufferHttpCache identifiers are not unique",
                                       cache->getConfig().DebugString(), config.DebugString()));
    }

    return cache;
  }

private:
  absl::Mutex mutex_;
  absl::flat_hash_map<std::string, std::weak_ptr<RingBufferHttpCache>>
      caches_ ABSL_GUARDED_BY(mutex_);
};

SINGLETON_MANAGER_REGISTRATION(ring_buffer_http_cache_singleton);

class RingBufferHttpCacheFactory : public HttpCacheFactory {
public:
  // From UntypedFactory
  std::string name() const override { return std::string{RingBufferHttpCache::extension_name}; }
  // From TypedFactory
  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<RbCacheProtoConfig>();
  }
  // From HttpCacheFactory
  std::shared_ptr<HttpCache>
  getCache(const envoy::extensions::filters::http::cache::v3::CacheConfig& filter_config,
           Server::Configuration::FactoryContext& context) override {
    RbCacheProtoConfig config;
    THROW_IF_NOT_OK(MessageUtil::unpackTo(filter_config.typed_config(), config));
    std::shared_ptr<CacheManager> caches =
        context.serverFactoryContext().singletonManager().getTyped<CacheManager>(
            SINGLETON_MANAGER_REGISTERED_NAME(ring_buffer_http_cache_singleton), &createManager);
    return caches->get(config);
  }

private:
  static std::shared_ptr<Singleton::Instance> createManager() {
    return std::make_shared<CacheManager>();
  }
};

static Registry::RegisterFactory<RingBufferHttpCacheFactory, HttpCacheFactory> register_;

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
