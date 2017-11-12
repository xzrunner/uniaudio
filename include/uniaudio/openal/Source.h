#ifndef _UNIAUDIO_OPENAL_SOURCE_H_
#define _UNIAUDIO_OPENAL_SOURCE_H_

#include "uniaudio/Source.h"

#include <OpenAL/al.h>

#include <memory>

namespace ua
{

class AudioData;
class Decoder;
class InputBuffer;
class OutputBuffer;

namespace openal
{

class AudioPool;
class Source : public ua::Source, public std::enable_shared_from_this<Source>
{
public:
	Source(AudioPool* pool, const AudioData* data);
	Source(AudioPool* pool, std::unique_ptr<Decoder>& decoder, bool mix = false);
	virtual ~Source();

	virtual bool Update() override final;

	virtual void Play() override final;
	virtual void Stop() override final;
	virtual void Pause() override final;
	virtual void Resume() override final;
	virtual void Rewind() override final;

	virtual void Seek(float offset) override final;
	virtual float Tell() override final;

	void PlayImpl();
	void StopImpl();
	void PauseImpl();
	void ResumeImpl();
	void RewindImpl();

	void SeekImpl(float offset);
	float TellImpl();

	void SetLooping(bool looping);
	bool IsLooping() const { return m_looping; }

	const InputBuffer* GetInputBuffer() const { return m_ibuf; }
	OutputBuffer* GetOutputBuffer() { return m_obuf; }

	void SetPlayer(ALuint player);
	ALuint GetPlayer() { return m_player; }

	bool IsStopped() const;
	bool IsPaused() const;
	bool IsFinished() const;

	bool IsStream() const { return m_stream; }
	bool IsMix() const { return m_mix; }

private:
	static ALenum GetFormat(int channels, int bit_depth);

	int Stream(ALuint buffer);

//	int GetFreq() const;
	float GetCurrOffset(int freq);

private:
	static const int OUTPUT_BUF_COUNT = 16;

private:
	AudioPool* m_pool;

	bool m_looping;
	bool m_active;
	bool m_paused;

	int m_freq;
	float m_offset;	// seconds

	const bool m_stream;
	const bool m_mix;

	// queue
	InputBuffer*  m_ibuf;
	OutputBuffer* m_obuf;

	// no mix
	ALuint m_player;
	static const unsigned int MAX_BUFFERS = 32;
	ALuint m_buffers[MAX_BUFFERS];

}; // Source

}
}

#endif // _UNIAUDIO_OPENAL_SOURCE_H_