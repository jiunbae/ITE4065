#ifndef ATOMIC_SNAPSHOT_HPP
#define ATOMIC_SNAPSHOT_HPP

#include <vector>
#include <shared_mutex>
#include <valarray>
#include <initializer_list>

#include <register.hpp>
#include <pool.hpp>

// valarray
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3639.html

namespace atomic {
    template <typename T>
    class Snapshot {
    public:

		/*
			StampedSnap: Extends from Atomic MRSW Register
			containe stamp and snap to store snapshot with stamp
		*/
		class StampedSnap : public Register<T, std::shared_timed_mutex> {
		public:

			using stamp_label = long long;

			StampedSnap(T value, stamp_label stamp = stamp_label(0), const std::valarray<T>& snap = std::valarray<T>()) noexcept
				: Register<T, std::shared_timed_mutex>(value), snap(snap), stamp(stamp) {
			}

			StampedSnap(const StampedSnap& ssnap) noexcept
				: Register<T, std::shared_timed_mutex>(ssnap.value), stamp(0) {
				snap = ssnap.snap;
			}

			std::valarray<T> get_snap() const {
				return snap;
			}

			stamp_label get_stamp() const {
				return stamp;
			}

		private:
			std::valarray<T> snap;
			stamp_label stamp;
		};

		Snapshot(size_t n)
            : table(new StampedSnap(0), n), moved(n, false) {
        }

		// update value called from tid-thread and increase stamp
        void update(size_t tid, T value) {
			std::valarray<T> snap = scan();
			StampedSnap* old_value = table[tid];
			StampedSnap* new_value = new StampedSnap(value, old_value->get_stamp() + 1, snap);
			table[tid] = new_value;
        }

		// take snapshot and check thread move
		std::valarray<T> scan() {
			std::valarray<StampedSnap*> old_value = table;
			std::valarray<StampedSnap*> new_value;
			moved.clear();

			while (true) {
				new_value = table;
				for (size_t i = 0; i < table.size(); ++i) {
					if (old_value[i]->get_stamp() != new_value[i]->get_stamp()) {
						if (moved[i]) {
							return old_value[i]->get_snap();
						} else {
							moved[i] = true;
							old_value = new_value;
							continue;
						}
					}
				}

				return capture_value(new_value);
			}
		}
    private:
		std::valarray<StampedSnap*> table;
		std::vector<bool> moved;

		static std::valarray<T> capture_value(const std::valarray<StampedSnap*>& ary) {
			std::vector<T> capture;
			for (const auto& snap : ary) {
				capture.emplace_back(snap->get());
			}
			return std::valarray<T>(capture.data(), capture.size());
		}
    };
}
#endif