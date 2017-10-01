#include "uniaudio/Decoder.h"
#include "uniaudio/Exception.h"

namespace ua
{

Decoder::Decoder(int buf_sz) 
	: m_buf(nullptr)
	, m_buf_size(buf_sz)
	, m_offset(0)
	, m_eof(false)
	, m_sample_rate(DEFAULT_SAMPLE_RATE)
{
	m_buf = new unsigned char[buf_sz];
	if (!m_buf) {
		throw Exception("Could not create decode buf.");
	}
}

Decoder::~Decoder() 
{
	if (m_buf) {
		delete[] m_buf;
	}
}

}