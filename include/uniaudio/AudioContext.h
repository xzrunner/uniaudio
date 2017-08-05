#ifndef _UNIAUDIO_AUDIO_CONTEXT_H_
#define _UNIAUDIO_AUDIO_CONTEXT_H_

namespace ua
{

class Source;
class AudioData;

class AudioContext
{
public:
	AudioContext() {}
	virtual ~AudioContext() {}

	virtual Source* CreateSource(const AudioData* data) = 0;

}; // AudioContext

}

#endif // _UNIAUDIO_AUDIO_CONTEXT_H_