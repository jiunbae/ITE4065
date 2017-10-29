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

namespace thread {
    class Pool {
    public:
		/*
			Init thread::Pool, set n workers to ready for works.
			use ::push to new task on thread::Pool
		*/
        Pool(size_t threads_n = std::thread::hardware_concurrency()) : stop(false) {
            if (!threads_n) throw std::invalid_argument("more than zero threads expected");

            this->workers.reserve(threads_n);
            for (; threads_n; --threads_n) {
                this->workers.emplace_back([this, tid = threads_n] {
                    while (true) {
                        std::function<void()> task = nullptr;
						std::function<void(size_t)> idtask = nullptr;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock, [this] {
                                return this->stop || this->tasks.size() || this->idtasks.size();
                            });
                            if (this->stop && this->tasks.empty() && this->idtasks.empty())
                                return;
							
							if (this->tasks.size()) {
								task = std::move(this->tasks.front());
								this->tasks.pop();
							} else if (this->idtasks.size()) {
								idtask = std::move(this->idtasks.front());
								this->idtasks.pop();
							}
                        }
						if (task != nullptr)
							task();
						else
							idtask(tid - 1);
                    }
                });
            }
        }

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
        Pool(Pool&&) = delete;
        Pool& operator=(Pool&&) = delete;

		/*
			push function to thread::Pool parameter with emplaced bind

			By implicitly binding the given parameters at this time and returning results to std::future
			Utilizing a different point in time between parameter input and result it becomes a more elegant code.

			function std::bind and cast to std::shared_ptr for efficiency
			packaged_task is performed in parallel on shared memory resources.
		*/
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

		/*
			push function to thread::Pool parameter with thread_id

			It is difficult to bind the parameters to get the thread ID as an argument.

			function packaged and cast to std::shared_ptr for efficiency
			packaged_task is performed in parallel on shared memory resources.
		*/
        template <typename F>
        std::future<typename std::result_of<F(size_t)>::type> push(F&& f) {
            using packaged_task_t = std::packaged_task<typename std::result_of<F(size_t)>::type(size_t)>;

            std::shared_ptr<packaged_task_t> task(new packaged_task_t(
				std::forward<F>(f)
            ));

            auto res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->idtasks.emplace([task](size_t tid) {
					(*task)(tid);
				});
            }

            this->condition.notify_one();
            return res;
        }

		// release resources
        virtual ~Pool() {
            this->stop = true;
            this->condition.notify_all();
            for (std::thread& worker : this->workers)
                worker.join();
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
		std::queue<std::function<void(size_t)>> idtasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        std::atomic_bool stop;
    };
}

#endif
