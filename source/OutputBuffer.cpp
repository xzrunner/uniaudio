#include "uniaudio/OutputBuffer.h"
#include "uniaudio/Exception.h"

#include <algorithm>

#include <string.h>
#include <stdlib.h>

namespace ua
{

OutputBuffer::OutputBuffer(int count, int size)
	: m_full(false)
{
	Init(count, size);
}

OutputBuffer::~OutputBuffer()
{
	std::list<Buffer*>::iterator itr = m_bufs.begin();
	for ( ; itr != m_bufs.end(); ++itr) {
		delete *itr;
	}
}

int OutputBuffer::Input(const unsigned char* buf, int buf_sz)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_full) {
		return 0;
	}

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
		memcpy(&dst->buf[dst->size], ptr, sz);
		dst->size += sz;
		ptr += sz;
		fill_sz += sz;
		if (fill_sz >= buf_sz) {
			break;
		}
	}

	if (fill_sz == 0) {
		m_full = true;
	}

	return fill_sz;
}

const unsigned char* OutputBuffer::Output(int& sz)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	Buffer* buf = m_bufs.front();

	sz = buf->size;
	const unsigned char* ret = sz == 0 ? NULL : buf->buf;

	// move to end
	m_bufs.pop_front();
	buf->size = 0;
	m_bufs.push_back(buf);
	m_full = false;

	return ret;
}

void OutputBuffer::Init(int count, int size)
{
	size_t sz = sizeof(Buffer) - sizeof(uint8_t) + size;
	sz = (sz + 3) & ~3;   // padding to 4 bytes aligned
	for (int i = 0; i < count; ++i)
	{
		void* ptr = malloc(sz);
		if (!ptr) {
			throw Exception("malloc fail.");
		}
		Buffer* buf = new (ptr) Buffer;
		memset(buf, 0, sz);
		buf->cap = size;
		buf->size = 0;
		m_bufs.push_back(buf);
	}
}

}