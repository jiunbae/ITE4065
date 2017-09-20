#ifndef TIME_H
#define TIME_H

#include <iostream>
#include <chrono>

using namespace std;

template<typename TimeT = chrono::nanoseconds>
struct measure
{
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F&& func, Args&&... args)
    {
        auto start = chrono::steady_clock::now();
        forward<decltype(func)>(func)(forward<Args>(args)...);
        auto duration = chrono::duration_cast< TimeT> 
                            (chrono::steady_clock::now() - start);
        return duration.count();
    }
};

#endif
