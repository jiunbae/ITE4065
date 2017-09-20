#include <input.h>
#include <iostream>

using namespace std;

int main() {

    int x = read_int();
    int c = 0;
    cout << x << newl;
    for (int i = 0; i < x; i++) {
        c += read_int();
    }

    cout << c << newl;

    return 0;
}