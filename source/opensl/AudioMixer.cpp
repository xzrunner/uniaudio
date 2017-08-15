#include "uniaudio/opensl/AudioMixer.h"
#include "uniaudio/opensl/AudioContext.h"

#include <algorithm>

#include <assert.h>

namespace ua
{
namespace opensl
{

AudioMixer::AudioMixer()
{
	m_samples = static_cast<int>(DEFAULT_SAMPLE_RATE * AudioContext::BUFFER_TIME_LEN);
	m_mix_buffer = new int32_t[m_samples * DEFAULT_CHANNELS];
	if (m_mix_buffer) {
		memset(m_mix_buffer, 0, sizeof(int32_t) * m_samples * DEFAULT_CHANNELS);
	}
	m_out_buffer = new int16_t[m_samples * DEFAULT_CHANNELS];
	if (m_out_buffer) {
		memset(m_out_buffer, 0, sizeof(int16_t) * m_samples * DEFAULT_CHANNELS);
	}
}

AudioMixer::~AudioMixer()
{
	if (m_mix_buffer) {
		delete[] m_mix_buffer;
	}
	if (m_out_buffer) {
		delete[] m_out_buffer;
	}
}

void AudioMixer::Input(const uint8_t* buf, int buf_sz, int sample_rate, int bit_depth, int channel)
{
	assert((bit_depth == 8 || bit_depth == 16)
		&& (channel == 1 || channel == 2));

 	const int src_samples = buf_sz / bit_depth / channel;
	const int dst_samples = src_samples * DEFAULT_SAMPLE_RATE / sample_rate;
	int32_t* ptr = m_mix_buffer;
	if (channel == 1)
	{
		if (bit_depth == 8)
		{
			for (int i = 0; i < dst_samples; ++i)
			{
				int src_idx = i * sample_rate / DEFAULT_SAMPLE_RATE;
				int src_val = static_cast<int8_t>(buf[src_idx]);
				*ptr++ = src_val;
				*ptr++ = src_val;
			}
		}
		else // bit_depth == 16
		{
			for (int i = 0; i < dst_samples; ++i)
			{
				int src_idx = i * sample_rate / DEFAULT_SAMPLE_RATE;
				int src_val = ((const int16_t*)buf)[src_idx];
				*ptr++ = src_val;
				*ptr++ = src_val;
			}
		}
	}
	else // channel == 2
	{
		if (bit_depth == 8)
		{
			for (int i = 0; i < dst_samples; ++i)
			{
				int src_idx = i * sample_rate / DEFAULT_SAMPLE_RATE * 2;
				int src_val = static_cast<int8_t>(buf[src_idx]);
				*ptr++ = src_val;
				src_val = static_cast<int8_t>(buf[src_idx + 1]);
				*ptr++ = src_val;
			}
		}
		else // bit_depth == 16
		{
			for (int i = 0; i < dst_samples; ++i)
			{
				int src_idx = i * sample_rate / DEFAULT_SAMPLE_RATE * 2;
				int src_val = ((const int16_t*)buf)[src_idx];
				*ptr++ = src_val;
				src_val = ((const int16_t*)buf)[src_idx + 1];
				*ptr++ = src_val;
			}
		}
	}
}

int16_t* AudioMixer::Output()
{
	for (int i = 0; i < m_samples; ++i) {
		m_out_buffer[i] = std::min(std::max(-32768, m_mix_buffer[i]), 32767);
	}
	return m_out_buffer;
}

}
}