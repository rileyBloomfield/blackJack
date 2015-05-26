#include "thread.h"
#include "unistd.h"

Thread::Thread(bool startOnCreation/*==false*/)
    : statusMutex(1), pauseMutex(0), pauseRequest(false),paused(true),stopRequest(false), stopped(false)
{
    //clock_gettime(CLOCK_REALTIME,&creation);
    gettimeofday(&creation,NULL);
    if (startOnCreation)
        Start();
}

long Thread::NowInUSec(void) const
{
    //timespec now;
    //clock_gettime(CLOCK_REALTIME,&now);
    timeval now;
    gettimeofday(&now,NULL);
    return 100000*(now.tv_sec - creation.tv_sec)+(now.tv_usec-creation.tv_usec);
}

Thread::~Thread()
{
    Stop();
    Wait(1000);
}

void * ThreadFunction(void * me)
{
    // The parameter was the "this" pointer, passed in Thread::Start.  It can
    // be safely cast back to Thread*
    Thread * theThread = (Thread*)me;
    return (void*) theThread->ThreadMain(); // Call a derived ThreadMain.
}

bool Thread::Start(void)
{
    // Create the thread through a system call.  Give the OS a pointer to this object
    bool error = pthread_create(&threadId,NULL,ThreadFunction,(void*)this);
    if (error == 0)
    {
        statusMutex.Wait();
        paused = false;
        statusMutex.Signal();
    }
    return error == 0;
}

void Thread::Stop(void)
{
    statusMutex.Wait();
    stopRequest = true;

    if (paused)
        pauseMutex.Signal();
    else
        statusMutex.Signal();
}

void Thread::Pause(void)
{
    statusMutex.Wait();
    pauseRequest = true;
    statusMutex.Signal();
}

void Thread::Resume(void)
{
    statusMutex.Wait();
    if (paused)
        pauseMutex.Signal();
    else
        statusMutex.Signal();
}

bool Thread::IsRunning(void)
{
    bool retVal;
    statusMutex.Wait();
    retVal = !paused;
    statusMutex.Signal();
    return retVal;
}

bool Thread::Wait(int millisecondTimeout)
{
    for (int i=0;i<millisecondTimeout;i++)
    {
        bool sample;
        statusMutex.Wait();
        sample = stopped;
        statusMutex.Signal();
        if (sample)
            return true;
        usleep(1000);
    }
    return false;
}

bool Thread::BePolite(void)
{
    bool changed = false;
    statusMutex.Wait();
    if (pauseRequest)
    {
        changed = true;
        paused = true;
        statusMutex.Signal();
        pauseMutex.Wait();
        // If we get here, we have the baton
        paused = false;
        pauseRequest = false;
    }
    if (stopRequest)
    {
        stopped = true;
        statusMutex.Signal();
        pthread_exit(NULL);
    }
    statusMutex.Signal();
    return changed;
}

void Thread::Sleep(int milliseconds)
{
    int centiseconds = milliseconds/10;
    for (int i=0;i<centiseconds;i++)
    {
        // After each ms, check to see if we've been asked to change status
        usleep(10000);
        long microsecondsBefore = NowInUSec();
        // The return value of BePolite lets us verify how long we were asleep.
        if (BePolite())
        {
            long elapsed = NowInUSec() - microsecondsBefore;
            i += elapsed/10000;
        }
    }
}
