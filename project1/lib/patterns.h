#include <string>
#include <vector>

using TABLE = std::vector<std::vector<char>>;

class Patterns {
public:
    Patterns() {

    }
    Patterns(const std::vector<int>& patterns) {

    }
    
private:
    int init_state = 0;
    TABLE table;


}