# Project1: Signal

- [Master](https://github.com/MaybeS/ITE4065/tree/master/project1)
- [Single Thread](https://github.com/MaybeS/ITE4065/tree/single/project1)
- [Multi Thread](https://github.com/MaybeS/ITE4065/tree/multi/project1)

## Master Version

### Features

- [x] Handle input query
    - [x] Q as query, find matched patterns in query (see also [ahocorasick::match]())
        - [x] print -1 if there are no matches
    - [x] A as Add, add pattern
    - [x] D as Delete, delete pattern

- [x] Ahocorasick
    - [x] make trie for patterns
    - [x] make `Failure Link` and `Output Link`
    - [x] find match
    - [x] efficent `add` algorithm; <= O(n)
    - [ ] efficent `delete` algorithm; <= O(n log n)

- [ ] Thread pool (not in this version)
    - [ ] minimize cond lock
    - [ ] find optimized thread size
    - [ ] support c++11(or above 14)
