#ifndef _UNIAUDIO_AUDIO_MIXER_H_
#define _UNIAUDIO_AUDIO_MIXER_H_

#include <CU_Uncopyable.h>

#include <stdint.h>
#include <string.h>

namespace ua
{

class AudioMixer : private cu::Uncopyable
{
public:
	AudioMixer(float buf_time_len);
	~AudioMixer();

	void Input(const uint8_t* buf, int buf_sz, int sample_rate, int bit_depth, int channel);
	int16_t* Output();

	int GetSamples() const { return m_samples; }
	int GetBufSize() const { return sizeof(int16_t) * DEFAULT_CHANNELS * m_samples; }

	void Reset();

public:
	// Indicates the quality of the sound.
	static const int DEFAULT_SAMPLE_RATE = 44100;

	// Default is stereo.
	static const int DEFAULT_CHANNELS = 2;

	static const int DEFAULT_BIT_DEPTH = 16;

private:
	void MixFast(const uint8_t* buf, int buf_sz, int sample_rate, int bit_depth, int channel);
	void MixSlow(const uint8_t* buf, int buf_sz, int sample_rate, int bit_depth, int channel);

private:
	int32_t* m_mix_buffer;
	int16_t* m_out_buffer;
	
	int m_samples;

	bool m_dirty;

}; // AudioMixer

}

#endif // _UNIAUDIO_AUDIO_MIXER_H_