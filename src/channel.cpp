#include "channel.h"
#include <optional>

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

template <typename T> T mpsc<T>::recv() {
    std::unique_lock lk(mtx_);
    cnd_.wait(lk, [this] { return !queue_.empty(); });
    lk.unlock();
    std::optional<T> val;
    {
        std::lock_guard<T> guard(mtx_);
        val = queue_.pop_front();
    }
    return val;
}
