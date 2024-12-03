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

  // ResponseData(Http::ResponseHeaderMapPtr&& response_headers, ResponseMetadata metadata,
  //              std::string body, Http::ResponseTrailerMapPtr&& trailers)
  //     : response_headers(std::move(response_headers)), metadata(metadata), body(body),
  //       trailers(std::move(trailers)) {}

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

  // const Http::ResponseHeaderMapPtr& getResponseHeaders() { return response_headers_; }

  // void setResponseHeaders(const Http::ResponseHeaderMapPtr& response_headers) {
  //   response_headers_ = Http::createHeaderMap<Http::ResponseHeaderMapImpl>(*response_headers);
  // }

  // void setResponseHeaders(Http::ResponseHeaderMapPtr&& response_headers) {
  //   response_headers_ = std::move(response_headers);
  // }

  // auto getMetadata() -> decltype(auto) { return metadata_; }

  // void setMetadata(const ResponseMetadata& metadata) { metadata_ = metadata; }

  // void setMetadata(ResponseMetadata&& metadata) { metadata_ = std::move(metadata); }

  // auto getBody() -> decltype(auto) { return body_; }

  // void setBody(const std::string& body) { body_ = body; }

  // void setBody(std::string&& body) { body_ = std::move(body); }

  // Http::ResponseTrailerMapPtr& getTrailers() { return trailers_; }

  // void setTrailers(const Http::ResponseTrailerMapPtr& trailers) {
  //   trailers_ = Http::createHeaderMap<Http::ResponseTrailerMapImpl>(*trailers);
  // }

  // void setTrailers(Http::ResponseTrailerMapPtr&& trailers) { trailers_ = std::move(trailers); }

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