#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>

template <typename T> class mpsc {
    std::deque<T> queue_;
    std::mutex mtx_;
    std::condition_variable cnd_;

public:
    static std::shared_ptr<mpsc> unbounded();
    void send(const T &val);
    void send(T &&val);
    T recv();
    std::optional<T> recv_or_timeout(std::chrono::microseconds timeout);
};

template <typename T> std::shared_ptr<mpsc<T>> mpsc<T>::unbounded() {
    return std::make_shared<mpsc<T>>();
}

template <typename T> void mpsc<T>::send(const T &val) {
    {
        std::lock_guard lk(mtx_);
        queue_.push_back(val);
    }
    cnd_.notify_one();
}

template <typename T> void mpsc<T>::send(T &&val) {
    {
        std::lock_guard lk(mtx_);
        queue_.push_back(std::move(val));
    }
    cnd_.notify_one();
}

template <typename T> T mpsc<T>::recv() {
    std::unique_lock lk(mtx_);
    cnd_.wait(lk, [this] { return !queue_.empty(); });
    lk.unlock();
    std::optional<T> val;
    {
        std::lock_guard<T> guard(mtx_);
        val = queue_.pop_front();
    }
    return val.value();
}

template <typename T>
std::optional<T> mpsc<T>::recv_or_timeout(std::chrono::microseconds timeout) {
    std::unique_lock lk(mtx_);
    bool has_value =
        cnd_.wait_for(lk, timeout, [this] { return !queue_.empty(); });
    if (!has_value)
        return {};
    lk.unlock();
    std::optional<T> val;
    {
        std::lock_guard guard(mtx_);
        val = std::move(queue_.front());
        queue_.pop_front();
    }
    return val;
}
