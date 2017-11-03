#ifndef ATOMIC_SNAPSHOT_HPP
#define ATOMIC_SNAPSHOT_HPP

// Imp: C++14 feature!
// valarray is std::array with dynamic size in runtime
// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3639.html
#include <valarray>
#include <queue>

#include <functional>

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
            : table(n), gc(n), moved(false, n) {
			for (auto& e : table) e = new StampedSnap();
			for (auto& e : gc) e = std::queue<StampedSnap*>();
        }

		~Snapshot() {
			// relase unused memory
			for (auto& e : gc)
				release<StampedSnap*>(e, [](StampedSnap * snap) {
					delete snap;
				}, 0);
		}

		// update value called from tid-thread and increase stamp
        void update(size_t tid, T value) {
			// relase unused memory
			// This syntax is very exceptional for prevents the queue overflow
			if (gc[tid].size() > 1000)
				release<StampedSnap*>(gc[tid], [](StampedSnap * snap) {
				delete snap;
			}, 100);

			std::valarray<T> snap = scan();
			StampedSnap* old_value = table[tid];
			StampedSnap* new_value = new StampedSnap(value, old_value->get_stamp() + 1, snap);
			table[tid] = new_value;

			// relase unused memory
			gc[tid].push(old_value);
        }

		// take snapshot and check thread move
		std::valarray<T> scan() {
			std::valarray<StampedSnap*> old_value = table;
			std::valarray<StampedSnap*> new_value;
			std::fill(std::begin(moved), std::end(moved), false);

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
		std::valarray<std::queue<StampedSnap*>> gc;
		std::valarray<bool> moved;

		// util function, maybe in util.hpp next time
		static void repeat(const std::function<void(size_t)>& f, size_t count) {
			for (size_t i = 0; i < count; ++i)
				f(i);
		}

		// util function, maybe in util.hpp next time
		template <typename F>
		static void release(std::queue<F>& trash, const std::function<void(F)>& f, size_t size = size_t(0)) {
			while (trash.size() > size) {
				auto front = trash.front(); trash.pop();
				f(front);
			}
		}

		static std::valarray<T> capture_value(const std::valarray<StampedSnap*>& ary) {
			std::valarray<T> ret(ary.size());

			repeat([&ret, &ary](size_t i) -> void {
				ret[i] = ary[i]->read();
			}, ary.size());

			return ret;
		}
    };
}
#endif