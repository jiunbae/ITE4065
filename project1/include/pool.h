#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

namespace Thread {
    class Pool {
    public:
        Pool(size_t);
        ~Pool();
        int size() { return _size; }
        template <class F, class... Args>
        auto push(F&& f, Args&&... args) 
            -> std::future<typename std::result_of<F(Args...)>::type>;
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;    
        std::mutex qutex;
        std::condition_variable cond;
        size_t _size;
        bool stop;
    };
     
    inline Pool::Pool(size_t size) : _size(size), stop(false) {
        for (size_t i = 0; i < size; ++i)
            workers.emplace_back([this]() {
                    for(;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->qutex);
                            this->cond.wait(lock, [this]() -> bool {
                                return this->stop || !this->tasks.empty();
                            });
                            if(this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
            );
    }

    template <class F, class... Args>
    auto Pool::push(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
            
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(qutex);
            tasks.emplace([task]() {
                (*task)();
            });
        }
        cond.notify_one();
        return res;
    }

    inline Pool::~Pool() {
        {
            std::unique_lock<std::mutex> lock(qutex);
            stop = true;
        }
        cond.notify_all();
        for (auto& worker: workers)
            worker.join();
    }
}

#endif