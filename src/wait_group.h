#pragma once

#include <mutex>
#include <condition_variable>

class WaitGroup
{
 public:
    void Add(int v = 1);
    void Done();
    bool Wait(int64_t timeout_us = -1);

 private:
    int m_counter = 0;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};
