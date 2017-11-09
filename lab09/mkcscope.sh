#!/bin/sh
rm -rf cscope.files
find . \( -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.ic' -o -name '*.h' -o -name '*.hpp' -o -name '*.s' -o -name '*.S' \) -print > cscope.files
cscope -b -q -k
