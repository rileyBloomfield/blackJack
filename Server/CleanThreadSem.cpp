#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <iostream>
#include <sstream>

int main(void)
{
    int found = 0;
    sem_t * pSem;

    for (int i=0;;i++)
    {
        std::ostringstream os;
        os << "THREADSEM" << i;
        pSem = sem_open(os.str().c_str(),0);
        if (pSem == SEM_FAILED)
        {
            // Done cleaning
            break;
        }
        else
        {
            // Clean one semaphore
            sem_close(pSem);
            sem_unlink(os.str().c_str());
            // Record the event
            found++;
        }
    }
    std::cout << "Found and deleted " << found << " stranded semaphores" << std::endl;
}
