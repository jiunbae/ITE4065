from subprocess import Popen, PIPE

import matplotlib.pyplot as plt

TARGET = "run"
EXECUTE_PARAM = "./"
RANGE = [1, 2, 4, 8, 16, 32]
print ("Execute target file:", TARGET)
print ("Execute range:", RANGE)

def execute(count):
    process = Popen([EXECUTE_PARAM + TARGET, str(count)], stdout=PIPE, stderr=PIPE)
    stdout, stderr = process.communicate()
    if stderr:
        raise Exception('Error while running {} {}'.format(TARGET, count), stderr)
    return int(stdout[8:])

fig = plt.figure()
for x, y in zip(RANGE, map(execute, RANGE)):
    print ("Result update count: {} in thread count: {}".format(y, x))
    plt.scatter(x, y, s=10, color='BLACK')

plt.xlabel("thread")
plt.ylabel("update")
plt.show()
plt.savefig('testplot.png')
plt.close()
