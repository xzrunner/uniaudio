#ifndef _UNIAUDIO_AUDIO_DATA_H_
#define _UNIAUDIO_AUDIO_DATA_H_

#include <cu/uncopyable.h>
#include <cu/cu_stl.h>

#include <stdint.h>

namespace ua
{

class Decoder;

class AudioData : private cu::Uncopyable
{
public:
	AudioData(const CU_STR& filepath);
	AudioData(const CU_VEC<ua::AudioData*>& list);
	~AudioData();

	const uint8_t* GetData() const { return m_data; }
	int GetSize() const { return m_size; }

	int GetSampleRate() const { return m_sample_rate; }
	int GetChannels() const { return m_channels; }
	int GetBitDepth() const { return m_bit_depth; }

private:
	void LoadFromFile(const CU_STR& filepath);

	void LoadFromList(const CU_VEC<ua::AudioData*>& list);

private:
	uint8_t* m_data;
	int m_size;

	int m_sample_rate;
	int m_channels;
	int m_bit_depth;

}; // AudioData

}

#endif // _UNIAUDIO_AUDIO_DATA_H_