#include <vector>

#define CHAR_START ('a')
#define CHAR_END ('z')
#define CHAR_SIZE (CHAR_END - CHAR_START)

#define DEFAULT_RESERVE_SIZE (2048)

namespace ahocorasick {
    using bType = std::vector<node>;    // block type
    using nType = int[26];              // node type
    using pType = std::string;          // pattern type
    using eType = char;                 // element type

    class Map {
    public:
        Map() { reserve(DEFAULT_RESERVE_SIZE); }
        Map(size_t size) { reserve(size); }
        ~Map() {}

        void resize(size_t size) {
            _resize_(fstates, size, int[CHAR_SIZE]);
            _resize_(fstates, size, int[CHAR_SIZE]);
        }

        void reserve(size_t size) {
            _reserve_(fstates, size);
            _reserve_(nstates, size);
        }

        node& operator[] (int index) {
            if (index < 0) {
                return fstates[-index];
            } else if (index > 0) {
                return nstates[index];
            } else {
                return istates;
            }
        }
    private:
        bType fstates;
        bType nstates;
        nType istates;

        void _resize_(bType& block, size_t size) {
            block.resize(size);
        }

        template<typename Init>
        void _resize_(bType& block, size_t size, Init initializer) {
            block.resize(size, initializer);
        }

        void _reserve_(bType& block, size_t size) {
            block.reserve(size);
        }
    };
}