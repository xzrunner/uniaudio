#include "uniaudio/Decoder.h"

namespace ua
{

Decoder::Decoder(int buf_sz) 
	: m_buf(NULL)
	, m_buf_size(buf_sz)
	, m_offset(0)
	, m_eof(false)
	, m_sample_rate(DEFAULT_SAMPLE_RATE)
{
	m_buf = new unsigned char[buf_sz];
}

Decoder::~Decoder() 
{
	if (m_buf) {
		delete[] m_buf;
	}
}

}