#ifndef _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_
#define _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_

#include "uniaudio/AudioContext.h"

#include <OpenAL/alc.h>

namespace ua
{
namespace openal
{

class AudioPool;
class AudioContext : public ua::AudioContext
{
public:
	AudioContext();
	virtual ~AudioContext();

	virtual ua::Source* CreateSource(const AudioData* data);
	virtual ua::Source* CreateSource(Decoder* decoder);
	virtual ua::Source* CreateSource(const std::string& filepath, bool stream);

	virtual void Stop();
	virtual void Pause();
	virtual void Resume();
	virtual void Rewind();

public:
	// 10ms length.
	static const float BUFFER_TIME_LEN;

private:
	void Initialize();
	void Terminate();

private:	
	ALCdevice* m_device;

	ALCcontext* m_context;

	AudioPool* m_pool;

}; // AudioContext

}
}

#endif // _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_