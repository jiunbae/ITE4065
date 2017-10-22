#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <queue>
#include <list>
#include <deque>

#include <mutex>

namespace thread {
	namespace safe {
		/*
			Mutex, implements Reader Writer Lock.

			It is compatible with std::mutex, std::shared_mutex, std::shared_timed_mutex
			and can be used with std::~_lock and std::lock_guard provided by the C++ standard.

			Also, ensure sequential execution and distinguish ownership using thread_id(tid).
		*/
		class Mutex {
		public:
			Mutex() noexcept
				: mutex(), reader(), writer(), writing(false), reader_count(0) {
			}

			~Mutex() noexcept {	
			}

			/*
				a.k.a. writer lock, 
				It is compatible with other mutexes in the C++ standard by calling function `lock`.
			*/
			void lock(size_t tid = 0) {
				std::unique_lock<std::mutex> lock(mutex);

				waiter.emplace_back(true, tid);
				do {
					writer.wait(lock, [&w = this->waiter, tid](){
						return w.front() == std::make_pair(true, tid);
					});
				} while (writing);
				writing = true;
				while (reader_count > 0)
					reader.wait(lock);
			}

			/*
				try lock return true if can lock, or false
			*/
			bool try_lock(size_t tid = 0) {
				std::lock_guard<std::mutex> lock(mutex);

				if (writing || 0 < reader_count)
					return false;
				else {
					waiter.emplace_back(true, tid);

					writing = true;
					return true;
				}
			}

			/*
				release writing lock, notify another writer
			*/
			void unlock() {
				{
					std::lock_guard<std::mutex> lock(mutex);

					writing = false;
					for (auto it = waiter.begin(); it != waiter.end(); ++it)
						if (it->first == true) {
							waiter.erase(it);
							break;
						}
				}

				writer.notify_all();
			}

			/*
				a.k.a. reader lock,
				It is compatible with other mutexes in the C++ standard by calling function `lock`.
			*/
			void lock_shared(size_t tid = 0) {
				std::unique_lock<std::mutex> lock(mutex);

				waiter.emplace_back(false, tid);
				do {
					writer.wait(lock, [&w = this->waiter, tid]() {
						for (const auto& i : w) {
							if (i.first == true)
								break;
							else if (i == std::make_pair(false, tid))
								return true;
						}
						return false;
					});
				} while (writing);

				reader_count += 1;
			}

			/*
				try shared lock return true if can shared_lock
			*/
			bool try_lock_shared(size_t tid = 0) {
				std::lock_guard<std::mutex> lock(mutex);

				if (writing || reader_count == max_reader)
					return false;
				else {
					waiter.emplace_back(false, tid);

					reader_count += 1;
					return true;
				}
			}

			/*
				relase reader lock
				When there are no more readers, notify another
			*/
			void unlock_shared() {
				size_t local_reader_count;
				bool local_writing;

				{
					std::lock_guard<std::mutex> _Lock(mutex);

					for (auto it = waiter.begin(); it != waiter.end(); ++it)
						if (it->first == false) {
							waiter.erase(it);
							break;
						}

					local_reader_count = --reader_count;
					local_writing = writing;
				}

				if (local_writing && local_reader_count == 0)
					reader.notify_one();
				else if (!local_writing)
					writer.notify_all();
			}

		private:
			std::mutex mutex;
			std::condition_variable reader;
			std::condition_variable writer;

			// true: writer, false: reader
			std::deque<std::pair<bool, size_t>> waiter;

			bool writing;
			size_t reader_count;
			static const size_t max_reader = -1;
		};
	}
}

#endif
