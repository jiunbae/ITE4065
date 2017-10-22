This page for documentation Project2: Simple Two-phase locking with Readers-writer Lock

# Review
Implementation of Two-pahse locking with Reader-Writer Lock



### The latest C++ standard features

- `optional`
- `string_view`
- `Sturctured Binding`
- `If statement with initializer`
- ETC...

## Classes

The following classes are implemented.

- arg::Parser
- thread::safe::Mutex
- thread::safe::Counter
- thread::safe::Container
- thread::Pool
- transaction::transaction
- Logger
- util::Random

### arg::Parser

Parse arguements. 

**methods**

- `size_t argument(const std::string& name, const std::string& description = "")`
  Install new argument as name [with description].
- `void parse(int argc, char * argv[])`
  execute parse argument, after this you can get value of parameters.
- `T get(const std::string& name)`
  return value of named argument.

### thread::safe::Mutex

Mutex, implements Reader Writer Lock.

Initially, I tried the C++ standard `std::shared_mutex`, `std::shared_timed_mutex`. However for implement with given manual, needed to improve the mutex to ensure the lock acquisition order. Internally, it manages the requested lock queue using a `std::deque` called *waiter*.

**methods**

- `void lock(size_t tid = 0)`
  Hold the writer lock. Wait for *writer* and *reader* (`std::condition_variable`) to finish. When waiting for the *reader*, check the *writing* to let the *writer* know that it is waiting.
- `bool try_lock(size_t tid = 0)`
  Return `false` if busy, if not take lock and return `true`
- `void unlock()`
  Release mutex and notify other waiters. Removes itself from the *waiter* to indicate it is not on hold.
- `void lock_shared(size_t tid = 0)`
  Hold the reader lock. Wait for *writer* to finish *writing*. All readers prior to the earliest writer get the mutex.
- `bool try_lock_shared(size_t tid = 0)`
  Return `false` if busy, if not take lock and return `ture`
- `void unlock_shared()`
  Release mutex and notify other waiters. REmove itself from the *waiter* to indicate it is not on hold.

### thread::safe::Record

A record supports mutexes. You can **get**, **add**, or **reset** values with any template T value. Also **acquire**, **try_acquire** or **relase** a mutex for template M. And have one enum called ***Operator*** that represent `GET`, `ADD` operation.

Since ***record*** does not support **overflow** or **deadlock checking**, it is recommended to access through ***container*** described later.

**methods**

- `bool try_acquire(Operator op, size_t tid)`
  Try can take lock return success with ***Operator***
- `void acquire(Operator op, size_t tid)`
  Get the mutex lock with ***Operator***.
- `void release(Operator op)`
  Release mutex lock with ***Operator***

**Below methods are dangers using without acquire, release lock**

- `T get()`
  Return value
- `T add(T v)`
  Add v to value and return origin value.
- `T reset(T v = 0)`
  Reset v to value and return origin value.

### thread::safe::Container

***Conatiner*** is collection of ***thread::safe::Record***.

- Support build transaction, commit.
- Support ***Operation*** make task easier and simply.

#### Operation

Implementation of single Operation on Record. Using ***Operator*** enum defined with ***Record***. The constructor must determine which thread will perform which operation on which record.

**methods**

- `T execute(record* operand, T value = 0)`
  Execute ***Operation*** on *operand* with *value*. Not required *value* when `GET` ***Operation***. Keep origin value return from ***Record***, So that can return it later.
- `void undo()`
  Undo execute. Throw `std::bad_function_Call` **undo()** before **execute()**
- `void release()`
  Release ***Record*** which executed.

Using this ***Operation*** class, the ***Container*** is made up of a more readable and easier task.

**methods**

- `optional<size_t> transaction(size_t tid, size_t i, size_t j, size_t k)`
  Build transaction, make three ***Operation*** one `READ`, two `WRITE`. Proceed sequentially from the `READ` ***Operation***. If each ***Operation*** fails or deadlock occurs, the previously executed ***Operation*** is canceled and reverted. It can be canceled by deadlocked or  overflow. If all operations are executed, the *build_id* is returned. You can commit using *build_id*.
- `optional<size_t> commit(size_t build_id, const std::function<void(size_t, …)>& f)`
  It takes a *build_id* as argument and a function to process the commit. If the commit is successful, *count* is incremented by 1 and return *commit_id*
- **private** `bool assert_deadlock(size_t thread_id, size_t record_id)`
  It analyzes the dependencies based on the requests of the waiting *records* and checks whether the deadlock occurs.

### thread::Pool

A nice and simple thread pool that supports C++ standard threads. With `std::future`, you can infer the expected type of result ahead of time when the job is added, and you can get the job done and get the results. When the task is added, parameters can be seamlessly executed in shared memory via `std::bind`, `std::packaged_task`, and `std::shared_ptr`. You can also add a task that takes a *thread_id* as an argument.

**methods**

- `std::future<> push(F&& f, Args&&… args)`
  `std::bind` and package the given function and arguments at the point of adding the work, and pass it to the worker who made it beforehand. Once the operation is done through the returned `std::future`, you can get the result.

### Logger

Logger class for logging

Write function writes a string to the stream over time, It may not be output sequentially by other threads. So it supports safe_write function. safe_write function puts another stream buffer and writes it to the file when the output is done.

Also, support ostream operator<< override.

### transaction::transaction

A special class for solving a given problem. Have *n* ***Logger***, *r* ***Record** through ***Container***. Process given task.

1. Select three `size_t` randomly.
2. Make transaction with selected id.
3. Commit if transaction successfully builded.
4. Write log to ***Logger***.

### util::Random

Beautiful random generater with uniform distribution.



## Waiting contributes!
you can comments and make merge requests to fix my codes. :smile: 
