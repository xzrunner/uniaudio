#ifndef _UNIAUDIO_OPENSL_AUDIO_MIXER_H_
#define _UNIAUDIO_OPENSL_AUDIO_MIXER_H_

#include <CU_Uncopyable.h>

#include <stdint.h>
#include <string.h>

namespace ua
{
namespace opensl
{

class AudioMixer : private cu::Uncopyable
{
public:
	AudioMixer();
	~AudioMixer();

	void Input(const uint8_t* buf, int buf_sz, int sample_rate, int bit_depth, int channel);
	int16_t* Output();

	int GetSamples() const { return m_samples; }
	int GetBufSize() const { return sizeof(int16_t) * DEFAULT_CHANNELS * m_samples; }

public:
	// Indicates the quality of the sound.
	static const int DEFAULT_SAMPLE_RATE = 44100;

	// Default is stereo.
	static const int DEFAULT_CHANNELS = 2;

	static const int DEFAULT_BIT_DEPTH = 16;

private:
	int32_t* m_mix_buffer;
	int16_t* m_out_buffer;
	
	int m_samples;

}; // AudioMixer

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_MIXER_H_