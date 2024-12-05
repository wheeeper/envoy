#pragma once

#include <utility>
#include "source/extensions/filters/http/cache/http_cache.h"
#include "source/common/protobuf/protobuf.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

struct ResponseData {
  ResponseData() = default;

  ~ResponseData() = default;

  ResponseData(const ResponseData& other) {
    response_headers =
        other.response_headers
            ? Http::createHeaderMap<Http::ResponseHeaderMapImpl>(*other.response_headers)
            : Http::ResponseHeaderMapPtr{};
    metadata = other.metadata;
    body = other.body;
    trailers = other.trailers
                    ? Http::createHeaderMap<Http::ResponseTrailerMapImpl>(*other.trailers)
                    : Http::ResponseTrailerMapPtr{};
  }

  ResponseData& operator=(const ResponseData& other) {
    if (&other != this) {
      response_headers =
          other.response_headers
              ? Http::createHeaderMap<Http::ResponseHeaderMapImpl>(*other.response_headers)
              : Http::ResponseHeaderMapPtr{};
      metadata = other.metadata;
      body = other.body;
      trailers = other.trailers
                      ? Http::createHeaderMap<Http::ResponseTrailerMapImpl>(*other.trailers)
                      : Http::ResponseTrailerMapPtr{};
    }
    return *this;
  }

  ResponseData(ResponseData&& other) noexcept {
    response_headers = std::move(other.response_headers);
    metadata = std::move(other.metadata);
    body = std::move(other.body);
    trailers = std::move(other.trailers);
  }
  ResponseData& operator=(ResponseData&& other) noexcept {
    response_headers = std::move(other.response_headers);
    metadata = std::move(other.metadata);
    body = std::move(other.body);
    trailers = std::move(other.trailers);

    return *this;
  }

  Http::ResponseHeaderMapPtr response_headers;
  ResponseMetadata metadata;
  std::string body;
  Http::ResponseTrailerMapPtr trailers;
};


} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy