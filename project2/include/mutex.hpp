#include <mutex>

namespace thread {
	namespace safe {
		class Mutex {
		public:
			Mutex() noexcept
				: mutex(), reader(), writer(), reader_count(0), writing(false) {	
			}

			~Mutex() noexcept {	
			}

			void lock() {
				std::unique_lock<std::mutex> lock(mutex);
				while (writing) writer.wait(lock);
				writing = true;
				while (0 < reader_count) reader.wait(lock);
			}

			void unlock() {
				{
					std::lock_guard<std::mutex> lock(mutex);
					writing = false;
				}
				writer.notify_all();
			}

			void lock_shared() {
				std::unique_lock<std::mutex> lock(mutex);
				while (writing || reader_count == max_reader) writer.wait(lock);
				++reader_count;
			}

			void unlock_shared() {	
				size_t local_reader_count;
				bool local_writing;

				{
					std::lock_guard<std::mutex> _Lock(mutex);
					local_reader_count = --reader_count;
					local_writing = writing;
				}

				if (local_writing && local_reader_count == 0)
					reader.notify_one();
				else if (!local_writing && local_reader_count == max_reader - 1)
					writer.notify_all();
			}

		private:
			std::mutex mutex;
			std::condition_variable reader;
			std::condition_variable writer;
			bool writing;
			size_t reader_count;
			static const size_t max_reader = -1;
		};
	}
}