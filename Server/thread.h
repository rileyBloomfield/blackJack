#ifndef THREAD_H
#define THREAD_H

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <list>
#include <iostream>
#include <string>
#include <pthread.h>
#include "ThreadSem.h"
class Thread
{
private:
    pthread_t threadId;
    timeval creation;
    ThreadSem statusMutex;
    ThreadSem pauseMutex;
    bool pauseRequest;
    bool paused;
    bool stopRequest;
    bool stopped;

private:
    void Finalize(void);
    long NowInUSec(void) const;
public:
    Thread(bool startOnCreation = false);
    virtual ~Thread();

    // Override this function in your threads
    virtual long ThreadMain(void) = 0;

    // Use these functions to control the thread from outside
    bool Start(void); // Start the thread
    void Stop(void);  // Kill the thread
    void Pause(void); // Make the thread stop temporarily
    void Resume(void); // Undo the effect of the previous Pause()
    bool IsRunning(void); // Is the thread currently active?
    bool Wait(int millisecondTimeout); // Wait for the thread to die.  Return false if it doesn't die before timeout

protected:
    // Your thread can use these to be polite with respect to Stop() and Pause()
    void Sleep(int milliseconds);   // Thread::Sleep can be stopped and paused nicely after worst case 10ms.  Otherwise, sleeps.
    bool BePolite(void);            // Thread::BePolite allows stopping or pausing if necessary.  Returns immediately otherwise.
};

#endif // THREAD_H
