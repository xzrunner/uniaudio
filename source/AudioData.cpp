#include "uniaudio/AudioData.h"
#include "uniaudio/Mpg123Decoder.h"
#include "uniaudio/Exception.h"

#include <algorithm>
#include <limits>

#include <string.h>

namespace ua
{

AudioData::AudioData(const std::string& filepath)
	: m_data(NULL)
	, m_size(0)
	, m_sample_rate(Decoder::DEFAULT_SAMPLE_RATE)
	, m_channels(0)
	, m_bit_depth(0)
{
	LoadFromFile(filepath);
}

AudioData::~AudioData()
{
	if (m_data) {
		delete[] m_data;
	}	
}

void AudioData::LoadFromFile(const std::string& filepath)
{
	Decoder* decoder = CreateDecoder(filepath);
	if (!decoder) {
		return;
	}
	
	size_t buf_size = 524288; // 0x80000
	int decoded = decoder->Decode();
	while (decoded > 0)
	{
		// Expand or allocate buffer. Note that realloc may move
		// memory to other locations.
		if (!m_data || buf_size < m_size + decoded)
		{
			while (buf_size < m_size + decoded) {
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
		if (m_size > std::numeric_limits<size_t>::max() - decoded)
		{
			free(m_data);
			throw Exception("Not enough memory.");
		}

		// Keep this up to date.
		m_size += decoded;

		decoded = decoder->Decode();
	}

	if (m_data && buf_size > m_size) {
		m_data = static_cast<uint8_t*>(realloc(m_data, m_size));
	}

	m_sample_rate = decoder->GetSampleRate();
	m_channels = decoder->GetChannels();
	m_bit_depth = decoder->GetBitDepth();

	decoder->RemoveReference();
}

Decoder* AudioData::CreateDecoder(const std::string& filepath)
{
	Decoder* decoder = NULL;

	std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
	if (ext == "mp3") {
		decoder = new Mpg123Decoder(filepath);
	}

	return decoder;
}

}