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

Decoder::Decoder(const Decoder& src)
	: m_buf(nullptr)
	, m_buf_size(src.m_buf_size)
	, m_offset(src.m_offset)
	, m_eof(src.m_eof)
	, m_sample_rate(src.m_sample_rate)
{
	m_buf = new unsigned char[m_buf_size];
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