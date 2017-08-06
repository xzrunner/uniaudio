#ifndef _UNIAUDIO_OPENAL_SOURCE_H_
#define _UNIAUDIO_OPENAL_SOURCE_H_

#include "uniaudio/Source.h"

#include <OpenAL/al.h>

namespace ua
{

class AudioData;
class Decoder;

namespace openal
{

class AudioPool;
class Source : public ua::Source
{
public:
	Source(AudioPool* pool, const AudioData* data);
	Source(AudioPool* pool, Decoder* decoder);
	virtual ~Source();

	virtual bool Update();

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

	void SetLooping(bool looping);
	bool IsLooping() const { return m_looping; }

private:
	static ALenum GetFormat(int channels, int bit_depth);

	bool IsStopped() const;
	bool IsPaused() const;
	bool IsFinished() const;

	int Stream(ALuint buffer);

private:
	AudioPool* m_pool;

	Decoder* m_decoder;

	const bool m_stream;

	static const unsigned int MAX_BUFFERS = 32;
	ALuint m_buffers[MAX_BUFFERS];

	ALuint m_source;

	bool m_looping;
	bool m_active;
	bool m_paused;

}; // Source

}
}

#endif // _UNIAUDIO_OPENAL_SOURCE_H_