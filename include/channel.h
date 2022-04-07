#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

template <typename T> class mpsc {
    std::deque<T> queue_;
    std::mutex mtx_;
    std::condition_variable cnd_;

    mpsc();

public:
    static std::shared_ptr<mpsc> unbounded();
    void send(const T &val);
    T recv();
};
