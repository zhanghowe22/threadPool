#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include <vector>
#include <functional>

class ThreadPool {
public:
    static ThreadPool& getInstance(int numThreads) {
        static ThreadPool instance(numThreads);
        return instance;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_stop = true;
        }

        m_cv.notify_all();
        for(auto& t : m_threads) {
            t.join();
        }
    }
    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        std::function<void()> task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_tasks.emplace(std::move(task));
        }

        m_cv.notify_one();
    }

    
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    ThreadPool(int numThreads) :m_stop(false) {
        for (int i = 0; i < numThreads; i++) {
            m_threads.emplace_back([this]{
                while (true) {
                    std::unique_lock<std::mutex> lock(m_mtx);
                    m_cv.wait(lock, [this] {
                        return !m_tasks.empty() || m_stop;
                    });

                if(m_stop && m_tasks.empty()) {
                    return;
                }

                std::function<void()> task(std::move(m_tasks.front()));
                m_tasks.pop();
                lock.unlock();
                task();

                }
            });
        }
    }

private:
    // 线程数组
    std::vector<std::thread> m_threads;
    // 任务队列
    std::queue<std::function<void()>> m_tasks;

    std::mutex m_mtx;

    std::condition_variable m_cv;

    bool m_stop;
};


#endif