#include <iostream>
#include <vector>
#include <string>

#include <argparser.hpp>
#include <transaction.hpp>

int main(int argc, char * argv[]) {
    arg::Parser parser;
    
    parser.argument("N", "thread count");
    parser.argument("R", "record count");
    parser.argument("E", "global execution order");

    parser.parse(argc, argv);

    // create operator
    transaction::Operator op(parser.get<size_t>("N"),
                                parser.get<size_t>("R"),
                                parser.get<transaction::int64>("E"));

    op.process();
}
