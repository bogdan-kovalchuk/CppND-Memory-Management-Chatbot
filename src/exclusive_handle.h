#ifndef EXCLUSIVE_HANDLE_H
#define EXCLUSIVE_HANDLE_H

#include <functional>
#include <utility>
#include <cstddef>

template <typename T>
class ExclusiveHandle {
public:
    using CloneFn = std::function<T *(const T *)>;

    ExclusiveHandle() noexcept : ptr_(nullptr), clone_(nullptr) {}

    explicit ExclusiveHandle(T *raw, CloneFn clone = nullptr) noexcept
        : ptr_(raw), clone_(std::move(clone)) {}

    ~ExclusiveHandle() { reset(); }

    ExclusiveHandle(const ExclusiveHandle &o) : ptr_(nullptr), clone_(o.clone_) {
        if (o.ptr_ && clone_) {
            ptr_ = clone_(o.ptr_);
        }
    }

    ExclusiveHandle &operator=(const ExclusiveHandle &o) {
        if (this != &o) {
            ExclusiveHandle tmp(o);
            swap(tmp);
        }
        return *this;
    }

    ExclusiveHandle(ExclusiveHandle &&o) noexcept : ptr_(o.ptr_), clone_(std::move(o.clone_)) {
        o.ptr_ = nullptr;
    }

    ExclusiveHandle &operator=(ExclusiveHandle &&o) noexcept {
        if (this != &o) {
            reset();
            ptr_ = o.ptr_;
            clone_ = std::move(o.clone_);
            o.ptr_ = nullptr;
        }
        return *this;
    }

    T *get() const noexcept { return ptr_; }

    T *release() noexcept {
        T *tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    void reset(T *raw = nullptr) noexcept {
        delete ptr_;
        ptr_ = raw;
    }

    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    T &operator*() const { return *ptr_; }
    T *operator->() const { return ptr_; }

    void swap(ExclusiveHandle &o) noexcept {
        using std::swap;
        swap(ptr_, o.ptr_);
        swap(clone_, o.clone_);
    }

private:
    T *ptr_;
    CloneFn clone_;
};

template <typename T>
void swap(ExclusiveHandle<T> &a, ExclusiveHandle<T> &b) noexcept {
    a.swap(b);
}

#endif
