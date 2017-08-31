#ifdef UA_SUPPORT_COREAUDIO

#ifndef _UNIAUDIO_CORE_AUDIO_DECODER_H_
#define _UNIAUDIO_CORE_AUDIO_DECODER_H_

#include "uniaudio/Decoder.h"

namespace ua
{

class CoreAudioDecoder : public Decoder
{
public:	
	CoreAudioDecoder(const std::string& filepath, int buf_sz = DEFAULT_BUFFER_SIZE);
	virtual ~CoreAudioDecoder();

	virtual int Decode();

	virtual bool Seek(float s);
	virtual bool Rewind();

	virtual int GetChannels() const;
	virtual int GetBitDepth() const;

private:
	void CloseAudioFile();

private:
	struct Data
	{
		Data() : file(NULL), size(0) {}
		~Data() { if (file) fs_close(file); }

		fs_file* file;
		size_t size;
	};

private:
	Data m_source;

	AudioFileID m_audio_file;
	ExtAudioFileRef m_ext_audio_file;

	AudioStreamBasicDescription m_input_info;
	AudioStreamBasicDescription m_output_info;

}; // CoreAudioDecoder

}

#endif // _UNIAUDIO_CORE_AUDIO_DECODER_H_

#endif // UA_SUPPORT_COREAUDIO