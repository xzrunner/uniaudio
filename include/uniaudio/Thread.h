#ifndef _UNIAUDIO_AUDIO_THREAD_H_
#define _UNIAUDIO_AUDIO_THREAD_H_

#include <CU_Uncopyable.h>

#include <pthread.h>

namespace ua
{
namespace thread
{

class Mutex : private cu::Uncopyable
{
public:
	Mutex();
	~Mutex();
	
private:
	pthread_mutex_t m_lock;

	friend class Lock;

}; // Mutex

class Lock : private cu::Uncopyable
{
public:
	Lock(Mutex* mutex);
	~Lock();

private:
	Mutex* m_mutex;

}; // Lock

class Thread : private cu::Uncopyable
{
public:
	Thread(void* (*main)(void*), void* arg);
	~Thread();

	static void Delay(unsigned int ms);

private:
	pthread_t m_thread;

}; // Thread

}
}

#endif // _UNIAUDIO_AUDIO_THREAD_H_