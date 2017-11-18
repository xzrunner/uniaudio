#ifdef UA_SUPPORT_COREAUDIO

#ifndef _UNIAUDIO_CORE_AUDIO_DECODER_H_
#define _UNIAUDIO_CORE_AUDIO_DECODER_H_

#include <cu/cu_stl.h>

#include "uniaudio/Decoder.h"

#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/ExtendedAudioFile.h>

#include <fs_file.h>

namespace ua
{

class CoreAudioDecoder : public Decoder
{
public:	
	CoreAudioDecoder(const std::string& filepath, int buf_sz);
	CoreAudioDecoder(const CoreAudioDecoder&);
	virtual ~CoreAudioDecoder();

	virtual Decoder* Clone();

	virtual int Decode() final;

	virtual bool Seek(float s) final;
	virtual bool Rewind() final;

	virtual int GetChannels() const final;
	virtual int GetBitDepth() const final;

	virtual float GetDuration() const override final;

	static bool Accepts(const CU_STR& ext);

private:
	void CloseAudioFile();

	void Init();

public:
	struct Data
	{
		Data() : file(nullptr), size(0) {}
		~Data() { if (file) fs_close(file); }

		fs_file* file;
		size_t size;
	};

private:
	std::string m_filepath;	
	Data        m_source;

	AudioFileID     m_audio_file;
	ExtAudioFileRef m_ext_audio_file;

	AudioStreamBasicDescription m_input_info;
	AudioStreamBasicDescription m_output_info;

	mutable float m_duration;

}; // CoreAudioDecoder

}

#endif // _UNIAUDIO_CORE_AUDIO_DECODER_H_

#endif // UA_SUPPORT_COREAUDIO