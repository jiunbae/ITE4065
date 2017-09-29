#include <utility>
#include <algorithm>

#include <vector>
#include <array>
#include <queue>

#define CHAR_START ('a')
#define CHAR_END ('z')
#define CHAR_SIZE (CHAR_END - CHAR_START)

#define DEFAULT_RESERVE_SIZE (2048)

namespace ahocorasick {
    enum state {
        final = -1, init, normal,
    };

    using index_type = int;
    using index_unsinged_type = size_t;
    using element_type = char;                              // element type
    using pattern_type = std::string;                       // pattern type
    using node_type = std::array<index_type, CHAR_SIZE>;    // node type
    using block_type = std::vector<node_type>;              // block type

    bool operator==(const index_type& index, state state) {
        if (state::init == index) {
            return true;
        } else {
            return !(state > 0 ^ index > 0);
        }
    }

    class Map {
    public:
        class iterator {
        public: 
        private:
        };

        Map() { init(DEFAULT_RESERVE_SIZE); }
        Map(size_t size) { init(size); }
        ~Map() {}
        
        template<template<typename T, typename All = std::allocator<T>> typename _container, typename _item>
        _container<index_type>& insert(const _container<_item>& container) {
            _container<index_type> results;
            for (const auto& pattern : container) {
                results.emplace(_insert(pattern));
            }
            return results;
        }

        index_type insert(const pattern_type& pattern) {
            return _insert(pattern);
        }

        template<template<typename T, typename All = std::allocator<T>> typename _container, typename _item>
        void erase(const _container<_item>& container) {
            for (const auto& pattern : container) {
                erase(pattern);
            }
        }

        void erase(const pattern_type& pattern) {
            index_type state = init_state;
            index_unsinged_type prev;
            index_unsinged_type flag;

            for (size_t i = 0; i < pattern.size(); ++i) {
                prev = i;
                state = at(state)[pattern[i]];

                if (state == state::final) {
                    flag = i;
                }
            }

            for (const auto& element : pattern) {
                pstate = state;
                prev = element;
                state = at(state)[element];
                
                if (state == state::final) {
                    cstate = state;
                    crev = element;
                }
            }
            
            // 마지막 f-state부터 자신의 끝까지 pop 실행
            for (state = cstate; cstate != pstate;) {
                state = at(state)[element];
            }

            pop(cstate, crev);
        }

        void erase(const index_type& index) {
            
        }

        void resize(size_t size) {
            _resize(fstates, size);
            _resize(nstates, size);
        }

        void reserve(size_t size) {
            _reserve(fstates, size);
            _reserve(nstates, size);
        }

        node_type& at(int index) {
            if (index < 0) {
                return fstates[-index];
            }
            else if (index > 0) {
                return nstates[index];
            }
            else {
                return istates;
            }
        }

        node_type& operator[](int index) {
            if (index < 0) {
                return fstates[-index];
            }
            else if (index > 0) {
                return nstates[index];
            }
            else {
                return istates;
            }
        }

    private:
        block_type fstates;
        block_type nstates;
        node_type istates;
        size_t node_size;
        size_t final_size;
        std::queue<index_type> node_empty;
        std::queue<index_type> final_empty;
        const static index_type init_state = 0;

        void init(size_t size) {
            istates = std::array<index_type, CHAR_SIZE>();

            node_size = final_size = 0;

            reserve(size);
            resize(size);
        }

        index_type next(state state) {
            if (final_size == fstates.size())
                _resize(fstates, final_size * 2);

            if (node_size == nstates.size())
                _resize(nstates, node_size * 2);

            switch (state) {
                case (state::init): 
                    return 0;
                case (state::normal): {
                    if (!node_empty.empty()) {
                        index_type index = node_empty.front();
                        node_empty.pop();
                        return index;
                    }
                    return ++node_size;
                }
                case (state::final): {
                    if (!final_empty.empty()) {
                        index_type index = final_empty.front();
                        final_empty.pop();
                        return index;
                    }
                    return -1 * (++final_size);
                }
            }
        }


        void pop(index_type index, element_type element) {
            if (index < 0) {
                index_type index_next = at(index)[element];
                node_type& node_next = at(index_next);
                
                if (_is_empty(node_next)) {
                    at(index)[element] = init_state;
                } else {
                    index_type state = next(state::normal);

                    at(index)[element] = state;
                    std::swap(at(state), at(index_next));
                }
                final_empty.push(index_next);
            } else if (index > 0) {
                at(index)[element] = init_state;
                if (std::all_of(at(index).begin(), at(index).end(), [](element_type element) {
                    return element == init_state;
                })) {
                    node_empty.push(index);
                }
            }
        }

        index_type _insert(const pattern_type& pattern) {
            bool flag = false;
            index_type state = init_state;
            index_type pre_state = 0;
            element_type pre_elem = 0;

            for (const auto& element : pattern) {
                if (at(state)[element - CHAR_START] == 0) {
                    state = at(pre_state = state)[pre_elem = element - CHAR_START] = next(state::normal);
                    flag = true;
                }
                else {
                    state = at(pre_state = state)[pre_elem = element - CHAR_START];
                }
            }

            index_type ret = at(pre_state)[pre_elem] = next(state::final);
            std::swap(at(at(pre_state)[pre_elem]), at(state));
            std::fill(at(state).begin(), at(state).end(), -1);
            return ret;
        }

        bool _is_empty(const node_type& node, element_type flag=init_state) {
            return std::all_of(node.begin(), node.end(), [](element_type element) {
                return element == init_state;
            });
        }

        void _resize(block_type& block, size_t size) {
            block.resize(size, std::array<index_type, CHAR_SIZE>());
        }

        void _reserve(block_type& block, size_t size) {
            block.reserve(size);
        }
    };
}