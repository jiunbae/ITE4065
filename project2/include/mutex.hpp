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

			/*
				mutex lock, not try just wait for writing
			*/
			void lock() {
				std::unique_lock<std::mutex> lock(mutex);
				while (writing) writer.wait(lock);
				writing = true;
				while (0 < reader_count) reader.wait(lock);
			}

			/*
				mutex unlock, make lock_guard to set writing false
				and notify_all writers
			*/
			void unlock() {
				{
					std::lock_guard<std::mutex> lock(mutex);
					writing = false;
				}
				writer.notify_all();
			}

			/*
				mutex lock_shared for reader, wait for no writing or no reader
			*/
			void lock_shared() {
				std::unique_lock<std::mutex> lock(mutex);
				while (writing || reader_count == max_reader) writer.wait(lock);
				++reader_count;
			}

			/*
				mutex unlock_shared for reader, make lock_guard to set local writing, reader_count
				check local writing and reader_count to notify reader or writer
			*/
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