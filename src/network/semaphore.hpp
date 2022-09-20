#ifndef REVELATION_SEMAPHORE_HPP
#define REVELATION_SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

struct Semaphore { // real semaphores are only available in C++20
private:
    std::mutex m; //Protects count
    std::condition_variable cv;
    int count;
public:
    explicit Semaphore(int count = 0): count(count) {  };
    //Assumes that nobody is waiting for the semaphore. Do NOT move semaphores that might be in use.
    Semaphore(Semaphore&& move): count(move.count){};
    void acquire(unsigned short int i = 1){
        std::unique_lock<std::mutex> lock(m);
        count -= i;
        if(count < 0) {
            cv.wait(lock, [&]{ return count >= 0; });
        }
        //else just release the lock and continue
    }
    void release(unsigned short int i = 1){
        {
            std::lock_guard<std::mutex> lock(m);
            count += i;
        } //release the lock
        cv.notify_one();
    }
};

#endif //REVELATION_SEMAPHORE_HPP
