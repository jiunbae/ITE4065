#include <vector>

#include <pool.hpp>

namespace atomic {
    template <typename T>
    class Snapshot {
    public:

		class StampedSnap {
		public:

			using stamp_label = long long;

			StampedSnap(T value) noexcept
				: stamp(0), value(value) {
			}

			StampedSnap(stamp_label stamp, T value, const std::vector<T>& snap) noexcept
				: stamp(stamp), value(value), snap(snap) {
			}

			StampedSnap(const StampedSnap& ssnap) noexcept
				: stamp(0) {
				snap = std::vector<T>(ssnap.snap.begin(), ssnap.snap.end());
			}

			T get_value() const {
				return value;
			}

			std::vector<T> get_snap() const {
				return snap;
			}

			stamp_label get_stamp() const {
				return stamp;
			}

		private:
			T value;
			std::vector<T> snap;
			stamp_label stamp;
		};

		Snapshot(size_t capacity)
            : n(capacity), table(capacity, new StampedSnap(0)) {
        }

        void update(size_t tid, T value) {
			std::vector<T> snap = scan();
			StampedSnap* old_value = table[tid];
			StampedSnap* new_value = new StampedSnap(old_value->get_stamp() + 1, value, snap);
			table[tid] = new_value;
        }

		std::vector<T>& scan() {
			std::vector<bool> moved(n, false);
			std::vector<StampedSnap*> old_value = collect();
			std::vector<StampedSnap*> new_value;

			while (true) {
				new_value = collect();
				for (size_t i = 0; i < n; ++i) {
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
				std::vector<T> result;
				for (const auto& snap : new_value) {
					result.emplace_back(snap->get_value());
				}
				return result;
			}
		}
    private:
		size_t n;
		std::vector<StampedSnap*> table;

		std::vector<StampedSnap*> collect() {
			std::vector<StampedSnap*> copy(table.begin(), table.end());
			return copy;
		}
    };
}