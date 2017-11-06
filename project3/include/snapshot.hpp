#ifndef ATOMIC_SNAPSHOT_HPP
#define ATOMIC_SNAPSHOT_HPP

// Imp: C++17 feature!
// valarray is std::array with dynamic size in runtime
// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3639.html
#include <valarray>
#include <queue>

#include <functional>

#include <utility.hpp>
#include <register.hpp>
#include <pool.hpp>

namespace atomic {
    template <typename T>
    class Snapshot {
    public:

		/*
			StampedSnap
			containe stamp and snap to store snapshot with stamp
		*/
		class StampedSnap {
		public:

			using stamp_label = long long;

			StampedSnap(T value = T(0), stamp_label stamp = stamp_label(0), 
				const std::valarray<T>& snap = std::valarray<T>()) noexcept
				: value(value), snap(snap), stamp(stamp) {
			}

			StampedSnap(const StampedSnap& ssnap) noexcept
				: value(ssnap.value), stamp(0) {
				snap = ssnap.snap;
			}

			T read() const {
				return value;
			}

			std::valarray<T> get_snap() const {
				return snap;
			}

			stamp_label get_stamp() const {
				return stamp;
			}
		private:
			T value;
			std::valarray<T> snap;
			stamp_label stamp;
		};

		Snapshot(size_t n)
            : table(n), moved(false, n), trash(n) {
			for (auto& e : table) e = new StampedSnap();
			for (auto& e : trash) e = std::queue<StampedSnap*>();
        }

		~Snapshot() {
			for (auto& e : trash)
				util::queue_apply<StampedSnap*>(e, [](StampedSnap * snap) { delete snap; }, 0);
			for (auto it = std::begin(table); it != std::end(table); ++it)
				delete *it;
		}

		// update value called from tid-thread and increase stamp
        void update(size_t tid, T value) {
			// relase unused memory
			// This syntax is very exceptional for prevents the queue overflow
			// (I think do not need, queue is enouih big to ensure)
			util::queue_apply<StampedSnap*>(trash[tid], [](StampedSnap * snap) { delete snap; }, 100);

			std::valarray<T> snap = scan();
			StampedSnap* old_value = table[tid];
			StampedSnap* new_value = new StampedSnap(value, old_value->get_stamp() + 1, snap);
			table[tid] = new_value;

			// relase unused memory
			trash[tid].push(old_value);
        }

		// take snapshot and check thread move
		std::valarray<T> scan() {
			std::valarray<StampedSnap*> old_value = table;
			std::valarray<StampedSnap*> new_value;
			std::fill(std::begin(moved), std::end(moved), false);

			while (true) {
				new_value = table;

				bool flag = false;
				for (size_t i = 0; i < table.size(); ++i) {
					if (old_value[i]->get_stamp() != new_value[i]->get_stamp()) {
						if (moved[i]) {
							return old_value[i]->get_snap();
						} else {
							flag = moved[i] = true;
							old_value = new_value;
							break;
						}
					}
				}
				if (flag) continue;

				std::valarray<T> ret(new_value.size());
				for (size_t i = 0; i < ret.size(); ++i)
					ret[i] = new_value[i]->read();
				return ret;
			}
		}

    private:
		std::valarray<StampedSnap*> table;
		std::valarray<bool> moved;

		// resolv memeory issue
		std::valarray<std::queue<StampedSnap*>> trash;
    };
}
#endif