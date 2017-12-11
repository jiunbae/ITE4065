This page for documentation Project 4: Scalable LockManager

# Objective
There are several performance bottlenecks in the lock manager of Innodb storage engine, which is the default in [mariadb](https://mariadb.org/) 10.2. 
Innodb's lock manager is the first bottleneck, aiming to improve the performance of mariadb by making this part scalable.

![mariaDB](https://mariadb.org/wp-content/uploads/2015/10/mariadb-usa-inc.png)

**This project is divided into two major milestones.**

- [x] [Milestone 1](Project4#milestone-1): Resolve performance bottlenecks of read-only transaction (due date: Dec 6, 2017)
- [ ] [Milestone 2](Project4#milestone-2): Resolve performance bottlenecks of read/write transaction (due date: Dec 23, 2017)

# Analysis of lock manager

# Milestone 1

Milestone 1 resolve performance bottlenecks of read-only transaction. Describe the process through how resolve the bottleneck below.

***Implementation of milestone 1 can not guarantee the correctness of the write operation. Therefore, when you run a test using `sysbench`, prepare process is safe to run before changes to this project.***

## The reality of the bottleneck


## Read Process

The Process of read operation is as follow.

![lock_mutex](https://hconnect.hanyang.ac.kr/2017_ITE4065_10074/2017_ITE4065_2015004584/raw/c249746dad0b6dce730d58513350bc5c66be08b0/project4/images/lock_mutex.png)

*Figure 1. Read-only workload structure*

## Solution

In order to solve the performance degradation caused by global lock, the lock for the record is managed as a lock-free linked list.

**My solution is separated by macros. See block of `#ifdef ITE4065` to `#endif`.**

### Lock Insertion

Requests coming in by read-only workload call `lock_rec_lock` via `lock_clust_rec_read_check_and_lock` function(see Figure 1). The original design is to hold the lock before `lock_rec_lock` and release it after acquire lock. However, to avoid that locking, change the root to point `lock_rec_lock` to the lock insert in the lock-free list. 

*Optional: implemented a [generic lock-free linked list](https://gist.github.com/MaybeS/7835ac602ed5c7bbb5758f652b250982) for testing.*

In the modified implementation, it is executed according to the following function call.

- `lock_clust_rec_read_check_and_lock`
- `lock_rec_lock`
- `lock_rec_lock_fast`
- `RecLock::create`
- `RecLock::lock_alloc`, `RecLock::lock_add`
- `lock_rec_insert_to_tail`

First of all, `RecLock::lock_alloc` uses `ut_malloc_nokey` to allocate and initialize memory for `lock_t*`.
In `RecLock::lock_add`, call `lock_rec_insert_to_tail` to append lock to lock-free list and add to trx_locks.
`lock_rec_insert_to_tail` is main implementation of latch-free design, using lock-free list to append new lock to tail of list. This process is handled using `__sync_lock_test_and_set` changes the hash of the previous node to point to current node. In case of tail of list is null, this means head is empty, so change head to point to current node. Order is guaranteed even if tail changes are requested by multiple threads because test_and_set(`__sync_lock_test_and_set`) is atomic operation and serialize contention.

### Lock Deletion

When a commit query comes in, the locks released by the transaction are released. If release lock immediately, Other threads that hold the lock can cause problems especially race. So, using **logical deletion** and **physical deletion** separately. 

**logical deletion** is changing state of lock to false which owner of the lock wants to release. And searching node in list of lock to detach logically deleted lock from list. If find the marked(logically deleted) node, using compare_and_swap(`__sync_bool_compare_and_swap`) to change node's hash pointing to next. So, other thread can not read the marked node even if contention. If compare_and_swap is successed, change next node's stamp to global timestamp. *global timestamp guarantee that bigger than running transactions stamp.* 

Since timestamp can be used in all transactions, it is managed by `trx_sys` and incremented by fetch_and_add(`__sync_fetch_and_add`) every time each commit.

### Garbage Collection

Even if performed logically deletion to lock, can't guarantee all of other threads not holding the deleted lock. That is why using **logical deletion** and **physical deletion** separately. Logical deletion just marking the node to say "This node will be released!" and detach node from lock list, attach to garbage collection list. In attach to garbage collection list using compare_and_set prevent race condition.

**physical deletion** is performed by the victim thread. Victim find timestamp which has minimum timestamp in running transactions and release lock iterating garbage collection list which timestamp is lower than minimum timestamp. Victim can wait with spinning if tail is changed while changing pointer of node. Especially tail is not pointing current node and next node's hash is null it's mean other thread change tail before changing pointer. *check gc_hash and `lock0lock.cc:3266` on `lock_t`*.

This implementation is not very good for performance, As the number of nodes to be deleted increases, the victim thread waits for garbage collection to increase significantly. This can be solved by creating a special thread for garbage collection.  However, I have not implemented it because can not specifically determine the execution or end time of a specific thread in the current situation without affecting other tasks,.

# Milestone 2

Not now,

# Results

Test on Linux System using sysbench

Ubuntu 14.04.5 LTS
Intel(R) Xeon(R) CPU E5-2650 v2 @ 2.6GHz
Sysbench 1.0.10

you can check the original result log on `project4/results`.

![test result](https://hconnect.hanyang.ac.kr/2017_ITE4065_10074/2017_ITE4065_2015004584/raw/7c8cbbf7ae58f2127f80e307d5926059e4590a9d/project4/results/result.png)

* Image 1, Test result *

And you can compare perf log between base and enhanced model.

![perf_base]
(https://hconnect.hanyang.ac.kr/2017_ITE4065_10074/2017_ITE4065_2015004584/raw/b074030c91a6b75b5114944a96d9c0dcb3e5724b/project4/images/perf_base.png)

* Image 2, Perf record of base model *

![perf_enhanced](https://hconnect.hanyang.ac.kr/2017_ITE4065_10074/2017_ITE4065_2015004584/raw/b074030c91a6b75b5114944a96d9c0dcb3e5724b/project4/images/perf_enhanced.png)

* Image 3, Perf record of enhanced model *
