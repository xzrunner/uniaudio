#ifndef _UNIAUDIO_AUDIO_CONTEXT_H_
#define _UNIAUDIO_AUDIO_CONTEXT_H_

namespace ua
{

class Source;

class AudioContext
{
public:
	AudioContext() {}
	virtual ~AudioContext() {}

	virtual bool Play(Source* source) = 0;
	virtual void Stop(Source* source) = 0;
	virtual void Pause(Source* source) = 0;
	virtual void Resume(Source* source) = 0;

}; // AudioContext

}

#endif // _UNIAUDIO_AUDIO_CONTEXT_H_