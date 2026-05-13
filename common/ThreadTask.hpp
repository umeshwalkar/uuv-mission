#ifndef THREADTASK_HPP
#define THREADTASK_HPP

#include <pthread.h>
#include <atomic>
#include <unistd.h>

class ThreadTask
{
public:
    ThreadTask()
        : running(false)
    {
    }

    virtual ~ThreadTask()
    {
        stop();
    }

    bool start()
    {
        running = true;

        return pthread_create(
                   &threadId,
                   nullptr,
                   threadEntry,
                   this) == 0;
    }

    void stop()
    {
        if (running)
        {
            running = false;
            pthread_join(threadId, nullptr);
        }
    }

    bool isRunning() const
    {
        return running;
    }

protected:
    virtual void task() = 0;

    std::atomic<bool> running;

private:
    pthread_t threadId;

    static void* threadEntry(void* arg)
    {
        ThreadTask* self =
            static_cast<ThreadTask*>(arg);

        self->task();

        return nullptr;
    }
};

#endif