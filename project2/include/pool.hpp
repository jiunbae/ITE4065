#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

#include <memory>
#include <functional>
#include <stdexcept>

namespace thread {
    class Pool {
    public:
        Pool(size_t threads_n = std::thread::hardware_concurrency()) : stop(false) {
            if (!threads_n) throw std::invalid_argument("more than zero threads expected");

            this->workers.reserve(threads_n);
            for (; threads_n; --threads_n) {
                this->workers.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock, [this] {
                                return this->stop || !this->tasks.empty();
                            });
                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
        Pool(Pool&&) = delete;
        Pool& operator=(Pool&&) = delete;

        template <typename F, typename... Args>
        std::future<typename std::result_of<F(Args...)>::type> push(F&& f, Args&&... args) {
            using packaged_task_t = std::packaged_task<typename std::result_of<F(Args...)>::type()>;

            std::shared_ptr<packaged_task_t> task(new packaged_task_t(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            ));

            auto res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->tasks.emplace([task]() { (*task)(); });
            }

            this->condition.notify_one();
            return res;
        }

        template <typename F>
        std::future<typename std::result_of<F(size_t)>::type> push(F&& f) {
            using packaged_task_t = std::packaged_task<typename std::result_of<F(size_t)>::type()>;

            std::shared_ptr<packaged_task_t> task(new packaged_task_t(
                std::bind(std::forward<F>(f), std::forward<size_t>(tasks.size() % workers.size()))
            ));

            auto res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->tasks.emplace([task]() { (*task)(); });
            }

            this->condition.notify_one();
            return res;
        }

        virtual ~Pool() {
            this->stop = true;
            this->condition.notify_all();
            for (std::thread& worker : this->workers)
                worker.join();
        }
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        std::atomic_bool stop;
    };
}

#endif
