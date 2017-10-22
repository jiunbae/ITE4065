#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <queue>
#include <list>
#include <deque>

#include <mutex>

namespace thread {
	namespace safe {
		class Mutex {
		public:
			Mutex() noexcept
				: mutex(), reader(), writer(), writing(false), reader_count(0) {
			}

			~Mutex() noexcept {	
			}

			void lock(size_t tid) {
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

			bool try_lock(size_t tid) {
				std::lock_guard<std::mutex> lock(mutex);
				if (writing || 0 < reader_count)
					return false;
				else {
					waiter.emplace_back(true, tid);

					writing = true;
					return true;
				}
			}

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

			void lock_shared(size_t tid) {
				std::unique_lock<std::mutex> lock(mutex);
				waiter.emplace_back(false, tid);

				do {
					writer.wait(lock, [&w = this->waiter, tid]() {
						for (const auto& i : w)
							if (i.first == true) break;
							else if (i == std::make_pair(false, tid)) return true;
						return false;
					});
				} while (writing);

				reader_count += 1;
			}

			bool try_lock_shared(size_t tid) {
				std::lock_guard<std::mutex> lock(mutex);
				if (writing || reader_count == max_reader)
					return false;
				else {
					waiter.emplace_back(false, tid);

					reader_count += 1;
					return true;
				}
			}

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

			// false when reader,
			std::deque<std::pair<bool, size_t>> waiter;

			bool writing;
			size_t reader_count;
			static const size_t max_reader = -1;

			bool readable() {
				return !writing;
			}

			bool writable() {
				return !writing;
			}
		};
	}
}

#endif
