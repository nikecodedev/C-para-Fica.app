#pragma once

#include <cstddef>

namespace engine {
namespace core {

/**
 * Fixed-size ring buffer. No dynamic allocation.
 * Single producer, single consumer. Not thread-safe.
 */
template <typename T, size_t Capacity>
class RingBuffer {
public:
    static constexpr size_t CAPACITY = Capacity;

    RingBuffer() : head_(0), size_(0) {}

    /** Push item. Overwrites oldest if full. */
    void push(const T& item) {
        data_[head_] = item;
        head_ = (head_ + 1) % Capacity;
        if (size_ < Capacity) ++size_;
    }

    /** Number of items stored. */
    size_t size() const { return size_; }

    /** True if empty. */
    bool empty() const { return size_ == 0; }

    /** Access by index: 0 = oldest, size()-1 = newest. */
    const T& operator[](size_t i) const {
        return data_[(head_ + Capacity - size_ + i) % Capacity];
    }

    T& operator[](size_t i) {
        return data_[(head_ + Capacity - size_ + i) % Capacity];
    }

    /** Oldest sample. */
    const T* oldest() const {
        if (empty()) return nullptr;
        return &(*this)[0];
    }

    /** Newest sample. */
    const T* newest() const {
        if (empty()) return nullptr;
        return &(*this)[size_ - 1];
    }

    /** Clear buffer. */
    void clear() {
        head_ = 0;
        size_ = 0;
    }

private:
    T data_[Capacity];
    size_t head_;
    size_t size_;
};

}  // namespace core
}  // namespace engine
