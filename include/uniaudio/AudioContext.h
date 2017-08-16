#ifndef _UNIAUDIO_AUDIO_CONTEXT_H_
#define _UNIAUDIO_AUDIO_CONTEXT_H_

#include <string>

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

	virtual Source* CreateSource(const AudioData* data) = 0;
	virtual Source* CreateSource(Decoder* decoder) = 0;
	virtual Source* CreateSource(const std::string& filepath, bool stream) = 0;

}; // AudioContext

}

#endif // _UNIAUDIO_AUDIO_CONTEXT_H_