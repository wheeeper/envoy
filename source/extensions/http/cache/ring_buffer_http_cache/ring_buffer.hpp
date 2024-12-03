#include <stdexcept>
#include <iterator>
#include <concepts>

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cache {
namespace RingBufferHttpCache {

template <typename T>
concept has_assign_operator = requires(T a, T b) {
  { a = b } -> std::same_as<T&>;
};

template <typename T>
requires std::default_initializable<T> && std::copy_constructible<T> && has_assign_operator<T>
class RingBuffer {
public:
  RingBuffer() = delete;
 
  // Constructor to initialize buffer with a given capacity
  RingBuffer(size_t cap) : capacity_(cap) {
    if (capacity_ == 0) {
      throw std::invalid_argument("Capacity must be greater than 0");
    }
    buffer_ = new T[capacity_];
  }

  // Destructor to free the allocated memory
  ~RingBuffer() { delete[] buffer_; }

  // Push an element to the buffer
  void push(const T& value) {
    if (size_ == capacity_) {
      throw std::overflow_error("Buffer is full");
    }

    buffer_[tail_] = value;
    tail_ = (tail_ + 1) % capacity_; // Wrap around if it goes past capacity
    ++size_;
  }

  // Pop an element from the buffer
  void pop() {
    if (size_ == 0) {
      throw std::underflow_error("Buffer is empty");
    }

    head_ = (head_ + 1) % capacity_; // Wrap around if it goes past capacity
    --size_;
  }

  // Check the current size of the buffer
  constexpr size_t size() const { return size_; }

  // Check if the buffer is empty
  bool empty() const { return size_ == 0; }

  // Check if the buffer is full
  bool full() const { return size_ == capacity_; }

  auto get(size_t idx) -> decltype(auto) {
    if (idx >= size_) {
      throw std::overflow_error("Index is greater than size");
    }

    return buffer_[(head_ + idx) % capacity_];
  }

private:
  T* buffer_;       // The array that holds the data
  size_t capacity_; // Maximum number of elements that can be stored
  size_t size_ = 0; // Current number of elements in the buffer
  size_t head_ = 0; // Index for the next write position
  size_t tail_ = 0; // Index for the next read position
};

} // namespace RingBufferHttpCache
} // namespace Cache
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy