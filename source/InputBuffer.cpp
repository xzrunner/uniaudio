#include "uniaudio/InputBuffer.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/OutputBuffer.h"

#include <assert.h>

namespace ua
{

InputBuffer::InputBuffer(Decoder* decoder)
	: m_decoder(decoder)
	, m_size(0)
	, m_used(0)
{
	if (m_decoder) {
		m_decoder->AddReference();
	}
}

InputBuffer::~InputBuffer()
{
	if (m_decoder) {
		m_decoder->RemoveReference();
	}
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