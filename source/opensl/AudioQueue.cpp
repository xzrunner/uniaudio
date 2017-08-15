#include "uniaudio/opensl/AudioQueue.h"

namespace ua
{
namespace opensl
{

AudioQueue::AudioQueue(int count, int size)
{
	Init(count, size);
}

AudioQueue::~AudioQueue()
{
	std::list<Buffer*>::iterator itr = m_bufs.begin();
	for ( ; itr != m_bufs.end(); ++itr) {
		delete *itr;
	}
}

int AudioQueue::Filling(const unsigned char* buf, int buf_sz)
{
	int fill_sz = 0;

	const unsigned char* ptr = buf;
	std::list<Buffer*>::iterator itr = m_bufs.begin();
	for ( ; itr != m_bufs.end(); ++itr)
	{
		Buffer* dst = *itr;
		if (dst->size >= dst->cap) {
			continue;
		}

		int sz = std::min(static_cast<int>(dst->cap - dst->size), buf_sz - fill_sz);
		memcpy(dst, ptr, sz);
		ptr += sz;
		fill_sz += sz;
		if (fill_sz >= buf_sz) {
			break;
		}
	}

	return fill_sz;
}

const unsigned char* AudioQueue::Top(int& sz) const
{
	Buffer* buf = m_bufs.front();
	if (buf->size == 0) {
		sz = 0;
		return NULL;
	} else {
		sz = buf->size;
		return buf->buf;
	}
}

void AudioQueue::Pop()
{
	Buffer* buf = m_bufs.front();
	m_bufs.pop_front();
	buf->size = 0;
	m_bufs.push_back(buf);
}

void AudioQueue::Init(int count, int size)
{
	size_t sz = sizeof(Buffer) - sizeof(uint8_t) + size;
	sz = (sz + 3) & ~3;   // padding to 4 bytes aligned
	for (int i = 0; i < count; ++i)
	{
		void* ptr = malloc(sz);
		if (!ptr) {
			break;
		}
		Buffer* buf = new (ptr) Buffer;
		memset(buf, 0, sz);
		buf->cap = size;
		buf->size = 0;
		m_bufs.push_back(buf);
	}
}

}
}