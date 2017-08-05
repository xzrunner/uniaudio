#ifndef _UNIAUDIO_OPENAL_SOURCE_H_
#define _UNIAUDIO_OPENAL_SOURCE_H_

#include "uniaudio/Source.h"

#include <OpenAL/al.h>

namespace ua
{

class AudioData;

namespace openal
{

class AudioPool;
class Source : public ua::Source
{
public:
	Source(AudioPool* pool, const AudioData* data);
	virtual ~Source();

	virtual void Play();
	virtual void Stop();
	virtual void Pause();
	virtual void Resume();
	virtual void Rewind();

	void PlayImpl();
	void StopImpl();
	void PauseImpl();
	void ResumeImpl();
	void RewindImpl();

private:
	static ALenum GetFormat(int channels, int bit_depth);

private:
	AudioPool* m_pool;

	bool m_stream;

	// static data buffer
	ALuint m_buffer;
	ALsizei m_size;

	ALuint m_source;

	bool m_active;
	bool m_paused;

}; // Source

}
}

#endif // _UNIAUDIO_OPENAL_SOURCE_H_