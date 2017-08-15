#ifndef _UNIAUDIO_OPENSL_SOURCE_H_
#define _UNIAUDIO_OPENSL_SOURCE_H_

#include "uniaudio/Source.h"
#include "uniaudio/opensl/AudioQueue.h"

namespace ua
{

class AudioData;
class Decoder;

namespace opensl
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

	const Decoder* GetDecoder() const { return m_decoder; }
	AudioQueue* GetBuffers() { return m_bufs; }

	bool IsStopped() const;
	bool IsPaused() const;
	bool IsFinished() const;

private:
	void Stream();
	void ReloadBuffer();

private:
	static const int QUEUE_BUF_COUNT = 16;

private:
	AudioPool* m_pool;

	Decoder* m_decoder;

	const bool m_stream;

	bool m_looping;
	bool m_active;
	bool m_paused;

	AudioQueue* m_bufs;

	// decoder's
	unsigned char* m_buf;
	int m_buf_sz, m_buf_used;

}; // Source

}
}

#endif // _UNIAUDIO_OPENSL_SOURCE_H_