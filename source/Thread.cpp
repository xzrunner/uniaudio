#include "uniaudio/Thread.h"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

namespace ua
{
namespace thread
{

/************************************************************************/
/* class Mutex                                                          */
/************************************************************************/

Mutex::Mutex() 
{
	pthread_mutex_init(&m_lock, NULL);
}

Mutex::~Mutex() 
{
	pthread_mutex_destroy(&m_lock);
}

/************************************************************************/
/* class Lock                                                           */
/************************************************************************/

Lock::Lock(Mutex* mutex) 
	: m_mutex(mutex) 
{
	pthread_mutex_lock(&m_mutex->m_lock);
}

Lock::~Lock() 
{
	pthread_mutex_unlock(&m_mutex->m_lock);
}

/************************************************************************/
/* class Thread                                                         */
/************************************************************************/

Thread::Thread(void* (*main)(void*), void* arg)
{
	pthread_create(&m_thread, NULL, main, arg);
}

Thread::~Thread()
{
	pthread_join(m_thread, NULL);
}

void Thread::Delay(unsigned int ms)
{
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(1000 * ms);
#endif
}

}
}