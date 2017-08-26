#ifndef _UNIAUDIO_OUTPUT_BUFFER_H_
#define _UNIAUDIO_OUTPUT_BUFFER_H_

#include <CU_Uncopyable.h>
#include <multitask/Thread.h>

#include <list>

#include <stdint.h>

namespace ua
{

class OutputBuffer
{
public:
	OutputBuffer(int count, int size);
	~OutputBuffer();

	int Input(const unsigned char* buf, int buf_sz);
	const unsigned char* Output(int& sz);

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
	mt::Mutex m_mutex;

 	std::list<Buffer*> m_bufs;

	bool m_full;

}; // OutputBuffer

}

#endif // _UNIAUDIO_OUTPUT_BUFFER_H_
