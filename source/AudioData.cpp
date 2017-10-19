#include "uniaudio/AudioData.h"
#include "uniaudio/Exception.h"
#include "uniaudio/DecoderFactory.h"
#include "uniaudio/Decoder.h"

#include <limits>
#include <algorithm>

#include <string.h>
#include <stdlib.h>

namespace ua
{

AudioData::AudioData(const CU_STR& filepath)
	: m_data(nullptr)
	, m_size(0)
	, m_sample_rate(Decoder::DEFAULT_SAMPLE_RATE)
	, m_channels(0)
	, m_bit_depth(0)
{
	LoadFromFile(filepath);
}

AudioData::AudioData(const CU_VEC<ua::AudioData*>& list)
	: m_data(nullptr)
	, m_size(0)
	, m_sample_rate(Decoder::DEFAULT_SAMPLE_RATE)
	, m_channels(0)
	, m_bit_depth(0)
{	
	LoadFromList(list);
}

AudioData::~AudioData()
{
	if (m_data) {
		delete[] m_data;
	}	
}

void AudioData::LoadFromFile(const CU_STR& filepath)
{
	std::unique_ptr<Decoder> decoder = DecoderFactory::Create(filepath);
	if (!decoder) {
		return;
	}
	
	size_t buf_size = 524288; // 0x80000
	int decoded = decoder->Decode();
	while (decoded > 0)
	{
		// Expand or allocate buffer. Note that realloc may move
		// memory to other locations.
		if (!m_data || buf_size < static_cast<size_t>(m_size + decoded))
		{
			while (buf_size < static_cast<size_t>(m_size + decoded)) {
				buf_size <<= 1;
			}
			m_data = static_cast<uint8_t*>(realloc(m_data, buf_size));
		}

		if (!m_data) {
			throw Exception("Not enough memory.");
		}

		// Copy memory into new part of memory.
		memcpy(m_data + m_size, decoder->GetBuffer(), decoded);

		// Overflow check.
		if (static_cast<unsigned int>(m_size) > std::numeric_limits<size_t>::max() - decoded)
		{
			free(m_data);
			throw Exception("Not enough memory.");
		}

		// Keep this up to date.
		m_size += decoded;

		decoded = decoder->Decode();
	}

	if (m_data && buf_size > static_cast<size_t>(m_size)) {
		m_data = static_cast<uint8_t*>(realloc(m_data, m_size));
	}

	m_sample_rate = decoder->GetSampleRate();
	m_channels = decoder->GetChannels();
	m_bit_depth = decoder->GetBitDepth();
}

void AudioData::LoadFromList(const CU_VEC<ua::AudioData*>& list)
{
	int sz = list.size();
	if (sz == 0) {
		return;
	} else {
		const ua::AudioData* src = list[0];
		m_size = src->m_size;
		m_data = static_cast<uint8_t*>(malloc(src->m_size));
		if (!m_data) {
			throw Exception("Could not malloc buf.");
		}
		memcpy(m_data, src->m_data, m_size);
		m_sample_rate = src->m_sample_rate;
		m_channels = src->m_channels;
		m_bit_depth = src->m_bit_depth;
		if (sz == 1) {
			return;
		}
	}

	for (int i = 1; i < sz; ++i) 
	{
		const ua::AudioData* ad = list[i];
		if (ad->m_sample_rate != m_sample_rate ||
			ad->m_channels != m_channels ||
			ad->m_bit_depth != m_bit_depth) {
			continue;
		}

		if (ad->m_size > m_size) {
			m_data = static_cast<uint8_t*>(realloc(m_data, ad->m_size));
			memset(m_data + m_size, 0, ad->m_size - m_size);
		}

		int ptr = 0;
		if (m_bit_depth == 16) 
		{
			short* s = (short*)(ad->m_data);
			short* d = (short*)(m_data);
			int idx = ptr >> 1;
			while (ptr < m_size && ptr < ad->m_size) {
				int add = s[idx] + d[idx];
				d[idx] = std::min(std::max(-32768, add), 32767);
				ptr += 2;
				idx += 1;
			}
		} 
		else if (m_bit_depth == 8)
		{
			while (ptr < m_size && ptr < ad->m_size) {
				int add = ad->m_data[ptr] + m_data[ptr];
				m_data[ptr] = std::min(std::max(-128, add), 127);
				ptr += 1;
			}
		}

		if (ptr == m_size) {
			memcpy(&m_data[ptr], &ad->m_data[ptr], ad->m_size - ptr);
		}
	}
}

}