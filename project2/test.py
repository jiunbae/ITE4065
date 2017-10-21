"""
This is testing program to test result of project2

:author Sangchul,Lee
:email scott824@naver.com
"""

import sys

def main():
    """
    get two main arguments
    First, number of thread
    Second, number of record
    """
    num_of_thread = int(sys.argv[1])
    num_of_record = int(sys.argv[2])
    lines = []

    # read thread[i].txt result text file
    for i in xrange(1, num_of_thread + 1):
        with open('thread' + str(i) + '.txt', 'r') as f:
            for line in f:
                lines.append(line)

    # sort by commit_id
    lines.sort(key=lambda line: int(line.split()[0]))

    records = [100 for _ in xrange(num_of_record + 1)]

    # main logic to check records
    for line in lines:
        commit_id, i, j, k, i_result, j_result, k_result = map(int, line.split())
        read = records[i]
        records[j] += read + 1
        records[k] -= read
        if i_result == records[i] and j_result == records[j] and k_result == records[k]:
            print str(commit_id), 'commit result is successful!'
        else:
            print str(commit_id), 'commit result is failed.'
            print 'real: record', str(i), '=', records[i], 'actual: record', str(i), '=', i_result
            print 'real: record', str(j), '=', records[j], 'actual: record', str(j), '=', j_result
            print 'real: record', str(k), '=', records[k], 'actual: record', str(k), '=', k_result
    print 'test finish'

if __name__ == '__main__':
    main();