#include "uniaudio/InputBuffer.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/OutputBuffer.h"

#include <assert.h>

namespace ua
{

InputBuffer::InputBuffer(std::unique_ptr<Decoder>& decoder)
	: m_decoder(std::move(decoder))
	, m_size(0)
	, m_used(0)
	, m_offset(0)
{
}

void InputBuffer::Output(OutputBuffer* out, bool looping)
{
	if (m_size == 0 || m_used == m_size) {
		Reload(looping);
	}
	if (m_size == 0) {
		return;
	}

	while (true)
	{
		int left = m_size - m_used;
		int sz = out->Input(&m_decoder->GetBuffer()[m_used], left);
		m_offset += sz;
		assert(sz <= left);
		if (sz < left) {
			m_used += sz;
			break;
		} else if (sz == left) {
			Reload(looping);
			if (m_size == 0) {
				break;
			}
		}
	}
}

bool InputBuffer::IsDecoderFinished() const
{
	return m_decoder ? m_decoder->IsFinished() : true;
}

void InputBuffer::DecoderRewind()
{
	if (m_decoder) {
		m_decoder->Rewind();
	}
}

void InputBuffer::Seek(float offset, bool looping)
{
	m_decoder->Seek(offset);
	Reload(looping);

	m_offset = m_decoder->GetBitDepth() * m_decoder->GetChannels() * offset * m_decoder->GetSampleRate() / 8;
}

float InputBuffer::GetOffset() const
{
	return m_offset * 8 / m_decoder->GetBitDepth() / m_decoder->GetChannels() / m_decoder->GetSampleRate();
}

void InputBuffer::Rewind()
{
	m_decoder->Rewind();
	m_offset = 0;
}

void InputBuffer::Reload(bool looping)
{
	m_size = m_decoder->Decode();
	assert(m_size <= m_decoder->GetBufferSize());	
	m_used = 0;
	if (m_decoder->IsFinished() && looping) {
		m_decoder->Rewind();
	}
}

}