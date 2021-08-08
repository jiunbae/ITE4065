This page for documentation Project 3: Wait-Free Sanpshot

# Review
Implementation of Wait-Free Sanpshot

### The latest C++ standard features

- [If statement with initializer](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0305r0.html)
- [Generic lambda-capture initializers](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3610.html)
- [valarray](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3639.html)

## Classes

The following classes are implemented. Click on the link to see my implementation.

- [atomic::Snapshot](https://github.com/jiunbae/ITE4065/wiki/Namespace::Atomic#snapshot)
- [atomic::Snapshot::StampedSnap](https://github.com/jiunbae/ITE4065/wiki/Namespace::Atomic#snapshot)
- [atomic::Register](https://github.com/jiunbae/ITE4065/wiki/namespace::atomic#reigster)
- [arg::Parser](https://github.com/jiunbae/ITE4065/wiki/Project2#argparser)
- [thread::Pool](https://github.com/jiunbae/ITE4065/wiki/namespace::thread#pool)
- [util::Random](https://github.com/jiunbae/ITE4065/wiki/namespace::util#random)

See [atomic::Snapshot](https://github.com/jiunbae/ITE4065/wiki/Namespace::Atomic#snapshot), major implementation of wait-free atomic snapshot. This class implements the specifications required by the assignment. Main thread just push snapshot random update to `thread::Pool`. 

The following code is the main content of the main thread.

```C++
size_t n = parser.get<size_t>("N");
size_t t = parser.get<size_t>("T", 60);

// main thread scope
{
	atomic::Snapshot<int> snapshot(n);		// snapshot instance
	util::Random<int> random;				// util::Random generator
	thread::Pool pool(n);					// thread::Pool for multi thread
	std::queue<std::future<void>> tasks;		// thread::Pool tasks
		
	// time guard to run only the set time  
	std::thread time_guard([&pool, &t]() {
		std::this_thread::sleep_for(std::chrono::seconds(t));
		pool.terminate();
	});

	// push tasks to thread::Pool until set time
	while (!pool.is_stop()) {
		pool.push([&pool, &snapshot, &random](size_t tid) {
			snapshot.update(tid, random.next());
		});
	}
		
	time_guard.join();
}
```
`parser` instance parse input argument `thread_number` and `task time` as optional. `snapshot` is Wait-Free Snapshot implementation instance, `random` is distributed random generator and `pool` is thread pool to run multiple tasks as parallel. `time_guard` ensures that the thread::Pool will only work for a specified amount of time. Below while loop update snapshot randomly.

## Performance

Checked in many clients. Here is my clients specs.
- Macbook pro 13" 2016: i5-5257U 2.7GHz 2C4T, `g++ 5.4 based on LLVM version 9.0.0`
- Linux Server: i7-7700k 4.2GHz 4C8T, `g++ 5.4`
- ML Server: i7-5930K 3.5GHz 6C12T (and many GPUs, but no effect) `g++ 5.4`
- SCS Lab Server: E5-2697 2.7GHz 24C48T `g++ 5.4`

And each results is... (all result executed while 60 seconds)

![result.png](https://github.com/jiunbae/ITE4065/blob/master/project3/results/result.png?raw=true)

I do not understand why SCSLab Server's return low performance than others.

## Review
I expect more performance on many-core-server. But not, I can't understand why. (Maybe, Single core speed make difference... maybe...)

Lower cores is more faster because the number of threads increases, the chance of getting a clean cut is reduced.

## Waiting contributes!
you can comments and make merge requests to fix my codes. :smile: 
