#include <iostream>
#include <vector>
#include <string>

#include <argparser.hpp>

int main(int argc, char * argv[]) {
    arg::Parser parser;
    
    parser.argument("N");
    parser.argument("R");
    parser.argument("E");

    parser.parse(argc, argv);

    int n = parser.get<int>("N");

    std::cout << n << std::endl; 
}
