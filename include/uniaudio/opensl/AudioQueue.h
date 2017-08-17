#ifndef _UNIAUDIO_OPENSL_AUDIO_QUEUE_H_
#define _UNIAUDIO_OPENSL_AUDIO_QUEUE_H_

#include <CU_Uncopyable.h>

#include <list>

#include <stdint.h>

namespace ua
{
namespace thread { class Mutex; }
namespace opensl
{

class AudioQueue
{
public:
	AudioQueue(int count, int size);
	~AudioQueue();

	int Push(const unsigned char* buf, int buf_sz);

	const unsigned char* Pop(int& sz); 

private:
	void Init(int count, int size);

private:
	struct Buffer
	{
		uint32_t cap;
		uint32_t size;
		uint8_t  buf[1];
	};

private:
	thread::Mutex* m_mutex;

 	std::list<Buffer*> m_bufs;

	bool m_full;

}; // AudioQueue

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_QUEUE_H_