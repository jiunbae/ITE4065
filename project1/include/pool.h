#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <list>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <functional>

namespace Thread {
    namespace Safe {
        template<typename T>
        class Queue {
        public:
            Queue() : _queue(), qutex(), cond(), _size(0) {}
            virtual ~Queue() {}

            virtual void push(const T& element) {
                {
                    std::unique_lock<std::mutex> lock(qutex);
                    _push(element);
                    cond.notify_all();
                }
            }
            virtual const T& pop() {
                T& ret = nullptr;
                {
                    std::unique_lock<std::mutex> lock(qutex);
                    wait(lock);
                    ret = _pop();
                }
                return ret;
            }

            size_t size() const { return _size; }

        protected:
            void wait(std::unique_lock<std::mutex>& lock) {
                while (_queue.empty())
                    cond.wait(lock);
            }

            void _push(const T& element) {
                _queue.push_back(element);
                ++_size;
            }

            T& _pop() {
                if (!_queue.empty()) {
                    T& ret = _queue.front();
                    _queue.pop_front();
                    --_size;
                    return ret;
                }
                return nullptr;
            }

            std::list<T> _queue;
            std::mutex qutex;
            std::condition_variable cond;
            size_t _size;
        };
    }

    class Pool {
    public:
        Pool(size_t);
        ~Pool();
        void wait() {
            {
                std::unique_lock<std::mutex> lock(qutex);
                cond.wait(lock, [this]() -> bool {
                    return tasks.empty();
                });
            }
        }

        size_t size() { return _size; }

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
                            if (this->stop && this->tasks.empty())
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
