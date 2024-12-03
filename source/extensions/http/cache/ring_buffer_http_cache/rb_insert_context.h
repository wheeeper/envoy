#pragma once

#include "source/extensions/filters/http/cache/http_cache.h"
#include "source/extensions/http/cache/ring_buffer_http_cache/response_data.hpp"
#include "source/extensions/http/cache/ring_buffer_http_cache/rb_lookup_context.h"
#include "source/common/buffer/buffer_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

class RingBufferHttpCache;

class RbInsertContext : public InsertContext {
public:
  RbInsertContext(RingBufferHttpCache& cache, Event::Dispatcher& dispatcher,
                  LookupContextPtr&& lookup_context);

  virtual ~RbInsertContext();

  void insertHeaders(const Http::ResponseHeaderMap& response_headers,
                     const ResponseMetadata& metadata, InsertCallback insert_success,
                     bool end_stream) override;

  void insertBody(const Buffer::Instance& chunk, InsertCallback ready_for_next_chunk,
                  bool end_stream) override;

  void insertTrailers(const Http::ResponseTrailerMap& trailers,
                      InsertCallback insert_complete) override;

  void onDestroy() override;

private:
  void post(InsertCallback callback, bool result);

  bool commit();

  RingBufferHttpCache& cache_;

  Event::Dispatcher& dispatcher_;

  LookupContextPtr lookup_context_;

  ResponseData cache_entry_;

  std::shared_ptr<bool> cancelled_ = std::make_shared<bool>(false);

  Buffer::OwnedImpl body_buffer_;

  bool committed_ = false;

  // Key key_;
  // const VaryAllowList& vary_allow_list_;
};

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy