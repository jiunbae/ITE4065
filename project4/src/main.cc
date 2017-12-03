#define UNIV_INNOCHECKSUM

#include <lf0lst.h>

class block_t;

typedef UT_LIST_NODE_T(block_t) block_node_t;
typedef UT_LIST_BASE_NODE_T(block_t) block_list_t;

class block_t {
    int value;
public:
    block_node_t m_node;
    block_list_t m_list;

    block_t(int value)
        : value(value) {
    }
};

int main() {
    block_list_t    m_list;
    UT_LIST_INIT(m_list, &block_t::m_node);

    // add
    block_t*        block = new block_t(23);

    UT_LIST_ADD_LAST(m_list, block);
}