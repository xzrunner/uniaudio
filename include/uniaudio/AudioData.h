#ifndef _UNIAUDIO_AUDIO_DATA_H_
#define _UNIAUDIO_AUDIO_DATA_H_

#include <CU_Uncopyable.h>

#include <string>

#include <stdint.h>

namespace ua
{

class Decoder;

class AudioData : private cu::Uncopyable
{
public:
	AudioData(const std::string& filepath);
	~AudioData();

	const uint8_t* GetData() const { return m_data; }
	int GetSize() const { return m_size; }

	int GetSampleRate() const { return m_sample_rate; }
	int GetChannels() const { return m_channels; }
	int GetBitDepth() const { return m_bit_depth; }

private:
	void LoadFromFile(const std::string& filepath);

	static Decoder* CreateDecoder(const std::string& filepath);

private:
	uint8_t* m_data;
	int m_size;

	int m_sample_rate;
	int m_channels;
	int m_bit_depth;

}; // AudioData

}

#endif // _UNIAUDIO_AUDIO_DATA_H_