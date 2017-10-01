# project1: Signal

## Process:
- [x] Implement aho-corasick
    - [x] failure link
    - [x] prefix map
    - [x] O(n) delete
- [x] Build project structure
    - [x] add reference solution
    - [x] add marker (to check)
    - [x] add test set (tiny, small)
- [ ] Multi thread
    - [ ] optimize critical section
    - [ ] change hyperparameter (thread num, chunk size, etc...)
- [ ] Benchmark
    - [x] IO bench
    - [x] STL bench (e.g. vector vs array)
    - [x] pass parameter

## Issues:

#### IO Issue
`std::iostream` is not much faster than **regular IO***(e.g. `scanf`, `getchar`, `getchar_unlocked` especially)*.
but marker doesn't work well using **regular IO** like below code.
```diff
-   std::cin >> n;
+   scanf("%d", &n);
```
