#ifndef THREADSEM_H
#define THREADSEM_H
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <string>
#include <sstream>

class ThreadSem
{
private:
    sem_t * pSem;
    std::string fullName;
public:
    ThreadSem(int initialState = 0)
        : pSem(SEM_FAILED)
    {
        // Find an unused name
        for (int i=0;;i++)
        {
            std::ostringstream os;
            os << "THREADSEM" << i;
            pSem = sem_open(os.str().c_str(),0);
            if (pSem == SEM_FAILED)
            {
                fullName = os.str();
                break;
            }
        }
        // Now create it for real
        pSem = sem_open(fullName.c_str(), O_CREAT, 0777,initialState);
        if (pSem == SEM_FAILED)
            throw std::string("Couldn't create semaphore");
    }
    virtual ~ThreadSem(void)
    {
        sem_close(pSem);
        sem_unlink(fullName.c_str());
    }

    void Wait()
    {
        sem_wait(pSem);
    }
    void Signal()
    {
        sem_post(pSem);
    }
    bool Signaled(void)
    {
        int value;
        sem_getvalue(pSem,&value);
        return (value > 0);
    }
};

#endif // SEMAPHORE_H
