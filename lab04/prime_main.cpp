#include <iostream>
#include <utility>
#include <math.h>
#include "operator.h"

using namespace std;

int main(int argc, char * argv[]) {
    std::ios_base::sync_with_stdio(false);
    
    int range_start;
    int range_end;

    Operator op = Operator(10);
    op.set([](int n) -> bool {
        if (n < 2)
            return false;

        for (int i = 2; i <= sqrt(n); i++)
            if (n % i == 0)
                return false;
        return true;
    });

    while (1) {
        cin >> range_start;
        if (range_start == -1) {
            break;
        }
        cin >> range_end;

        cout << "number of prime: " << op.run(range_start, range_end) << '\n';
    }
 
    return 0;
}

