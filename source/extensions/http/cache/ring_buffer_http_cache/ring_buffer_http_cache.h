#pragma once

#include "rb_lookup_context.h"
#include "source/extensions/filters/http/cache/http_cache.h"
#include "envoy/extensions/http/cache/ring_buffer_http_cache/v3/ring_buffer_http_cache.pb.h"
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
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
  constexpr static absl::string_view extension_name =
      "envoy.extensions.http.cache.ring_buffer_http_cache";

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

  // Lookups cache entry
  // Returns absl::optional<ResponseData> if request found, otherwise absl::nullopt
  absl::optional<ResponseData> lookup(const LookupRequest& request);

  // Inserts entry to cache
  // Returns true if succesfull, otherwise false
  bool insert(const RbLookupContext& lookup_context, ResponseData&& cache_entry);

  // Notifys cache that insert is being prepared
  void notifyInsertStart(Key key);

  // Notifys cache that insert has finished
  void notifyInsertEnd(Key key);

  // Gets cache config
  const RbCacheProtoConfig& getConfig() const;

private:
  using RingBufferType = RingBuffer<std::pair<RbCacheKey, ResponseData>>;

  using KeyCompareFuncType = std::function<bool(const RbCacheKey&, const RbCacheKey&)>;

  // Searches internal storage ring buffer for entry
  // Takes RbCacheKey key, for which is searched and CompareFuncType function, which is used for
  // comparison of keys Returns absl::optional<size_t> with entry index if found, otherwise
  // absl::nullopt
  absl::optional<size_t> searchBuffer(const RbCacheKey& key, KeyCompareFuncType comparator);

  // Gets cache entry info based on lookup request
  // Returns absl::optional<std::pair<RbCacheKey, size_t>> with found key and index position,
  // otherwise absl::nullopt
  absl::optional<std::pair<RbCacheKey, size_t>> getCacheEntryInfo(const LookupRequest& request);

  absl::optional<std::string> getVaryId(const LookupRequest& request,
                                        const ResponseData& response_headers);

  // Ring buffer cache
  RingBufferType ring_buffer_ ABSL_GUARDED_BY(cache_mtx_);

  // Map of currently inserted keys with absl::Notification for synchronization between threads
  absl::flat_hash_map<Key, std::shared_ptr<absl::Notification>, MessageUtil, MessageUtil>
      currently_inserted_ ABSL_GUARDED_BY(insert_map_mtx_);

  RbCacheProtoConfig config_;

  absl::Mutex insert_map_mtx_;

  absl::Mutex cache_mtx_;
};

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy