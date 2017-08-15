#ifndef _UNIAUDIO_OPENSL_AUDIO_QUEUE_H_
#define _UNIAUDIO_OPENSL_AUDIO_QUEUE_H_

#include <CU_Uncopyable.h>

#include <list>

#include <stdint.h>

namespace ua
{
namespace opensl
{

class AudioQueue
{
public:
	AudioQueue(int count, int size);
	~AudioQueue();

	int Filling(const unsigned char* buf, int buf_sz);

	const unsigned char* Top(int& sz) const; 
	void Pop();

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
 	std::list<Buffer*> m_bufs;

}; // AudioQueue

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_QUEUE_H_