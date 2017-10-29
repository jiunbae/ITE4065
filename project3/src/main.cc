#include <argparser.hpp>

int main(int argc, char * argv[]) {
    arg::Parser parser;
    
    parser.argument("N", "thread count");

    parser.parse(argc, argv);

    atomic::snapshot snap<parser.get<size_t>("N")>();
}