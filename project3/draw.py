#!/bin/python
'''
    File name: draw.py
    Author: Bae Jiun, Maybe
    Date created: 10/27/2017
    Requirements:
        - matplotlib
'''

from subprocess import Popen, PIPE
from os import environ
import matplotlib.pyplot as plt

TARGET = "run"
RANGE = [1, 2, 4, 8, 16, 32]
print ("Execute target file:", TARGET)
print ("Execute range:", RANGE)

def execute(count):
    def parse(out):
        if ':' in out: return int(out.split(':')[1])
        return int(out)

    process = Popen(["./" + TARGET, str(count)], stdout=PIPE, stderr=PIPE)
    stdout, stderr = process.communicate()
    if stderr:
        raise Exception('Error while running {} {}'.format(TARGET, count), stderr)
    return parse(stdout)

fig = plt.figure()
for x, y in zip(RANGE, map(execute, RANGE)):
    print ("Result update count: {} in thread count: {}".format(y, x))
    plt.scatter(x, y, color='BLACK')

# change y range if you need
# plt.ylim((25,250))
plt.xlabel("thread")
plt.ylabel("update")

plt.savefig('testplot.png')
if 'DISPLAY' in environ:
    plt.show()
plt.close()
