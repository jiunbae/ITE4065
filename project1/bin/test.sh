#!/bin/bash

make clean
make all

# INIT TEST
TEST_PATH="test/data/small"
TEST_INIT="$TEST_PATH.init"
TEST_WORK="$TEST_PATH.work"
TEST_RESULT="$TEST_PATH.result"

WORK="./marker"
RUN="./run"
REF="./ref"

# TEST COUNT LOAD
TEST_NUM_FILE="log/count"
TEST_NUM=0
if [ ! -f "$TEST_NUM_FILE" ]
then
    echo "file does't exist"
    echo "$TEST_NUM" > $TEST_NUM_FILE
else
    TEST_NUM=$(<$TEST_NUM_FILE)
fi

# Utility init
TEST_FILE="log/LAB $TEST_NUM-$(date +%s)"

TEST_INFO="LAB $TEST_NUM
LAB_TIME: $(date +%s)
TEST_MESSAGE: $1
"

# RUN TEST
RESULT_RUN=`$WORK $TEST_INIT $TEST_WORK $TEST_RESULT $RUN`
RESULT_REF=`$WORK $TEST_INIT $TEST_WORK $TEST_RESULT $REF`

echo "$TEST_INFO $RESULT_RUN" > "$TEST_FILE-RUN"
echo "$TEST_INFO $RESULT_REF" > "$TEST_FILE-REF"

echo "$RESULT_RUN"

echo $((TEST_NUM + 1)) > $TEST_NUM_FILE

