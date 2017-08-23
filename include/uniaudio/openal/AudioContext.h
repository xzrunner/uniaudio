#ifndef _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_
#define _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_

#include "uniaudio/AudioContext.h"

#include <OpenAL/alc.h>

namespace mt { class Thread; }

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

public:
	// 10ms length.
	static const float BUFFER_TIME_LEN;

private:
	bool Init();

private:	
	ALCdevice* m_device;

	ALCcontext* m_context;

	AudioPool* m_pool;
	mt::Thread* m_pool_thread;

}; // AudioContext

}
}

#endif // _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_