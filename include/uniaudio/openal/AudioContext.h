#ifndef _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_
#define _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_

#include "uniaudio/AudioContext.h"

#include <OpenAL/alc.h>

namespace ua
{
namespace openal
{

class AudioContext : public ua::AudioContext
{
public:
	AudioContext();
	virtual ~AudioContext();

	virtual bool Play(Source* source);
	virtual void Stop(Source* source);
	virtual void Pause(Source* source);
	virtual void Resume(Source* source);

private:
	bool Init();

private:	
	ALCdevice* m_device;

	ALCcontext* m_context;

}; // AudioContext

}
}

#endif // _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_