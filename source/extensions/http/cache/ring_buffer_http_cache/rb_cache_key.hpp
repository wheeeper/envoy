#pragma once

#include <utility>
#include "absl/types/optional.h"
#include "source/extensions/filters/http/cache/http_cache.h"
#include "source/extensions/filters/http/cache/key.pb.h"
#include "source/common/protobuf/protobuf.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

// Structure for RingBuferHttpCache key
struct RbCacheKey {
public:
  RbCacheKey() = default;

  ~RbCacheKey() = default;

  RbCacheKey(Key key) : key(key){};

  RbCacheKey(Key key, std::string vary_identifier) : key(key), vary_id(vary_identifier){};

  RbCacheKey(const RbCacheKey& other) {
    key = other.key;
    vary_id = other.vary_id;
  }

  RbCacheKey& operator=(const RbCacheKey& other) {
    if (&other != this) {
      key = other.key;
      vary_id = other.vary_id;
    }
    return *this;
  }

  RbCacheKey(RbCacheKey&& other) noexcept {
    key = std::move(other.key);
    vary_id = std::move(other.vary_id);
  }

  RbCacheKey& operator=(RbCacheKey&& other) noexcept {
    key = std::move(other.key);
    vary_id = std::move(other.vary_id);
    return *this;
  }

  bool operator==(const RbCacheKey& rhs) const {
    return RbCacheKey::compareWholeKeys(*this, rhs);
  }

  bool operator !=(const RbCacheKey& rhs) const {
    return !(*this == rhs);
  }

  static bool compareOriginalKeys(const RbCacheKey& first, const RbCacheKey& second) {
    return Protobuf::util::MessageDifferencer::Equals(first.key, second.key);
  }

  static bool compareWholeKeys(const RbCacheKey& first, const RbCacheKey& second) {
    return compareOriginalKeys(first.key, second.key) &&
           first.vary_id == second.vary_id;
  }

  Key key;
  absl::optional<std::string> vary_id;
};

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy