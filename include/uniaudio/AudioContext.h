#ifndef _UNIAUDIO_AUDIO_CONTEXT_H_
#define _UNIAUDIO_AUDIO_CONTEXT_H_

#include <cu/cu_stl.h>

#include <memory>

namespace ua
{

class Source;
class AudioData;
class Decoder;

class AudioContext
{
public:
	AudioContext() {}
	virtual ~AudioContext() {}

	virtual std::shared_ptr<ua::Source> CreateSource(const AudioData* data) = 0;
	virtual std::shared_ptr<ua::Source> CreateSource(std::unique_ptr<Decoder>& decoder) = 0;
	virtual std::shared_ptr<ua::Source> CreateSource(const CU_STR& filepath, bool stream) = 0;

	virtual void Stop() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Rewind() = 0;

	virtual void SetVolume(float volume) = 0;

}; // AudioContext

}

#endif // _UNIAUDIO_AUDIO_CONTEXT_H_