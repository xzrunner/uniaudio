#ifndef _UNIAUDIO_OPENSL_SOURCE_H_
#define _UNIAUDIO_OPENSL_SOURCE_H_

#include "uniaudio/Source.h"
#include "uniaudio/opensl/AudioPlayer.h"

#include <string>

namespace ua
{

class AudioData;
class Decoder;
class InputBuffer;
class OutputBuffer;

namespace opensl
{

class AudioPool;
class Source : public ua::Source
{
public:
	Source(AudioPool* pool, const std::string& filepath);
	Source(AudioPool* pool, Decoder* decoder);
	virtual ~Source();

	virtual bool Update();

	virtual void Play();
	virtual void Stop();
	virtual void Pause();
	virtual void Resume();
	virtual void Rewind();

	virtual void Seek(float offset);
	virtual float Tell();

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

	const std::string& GetFilepath() const { return m_filepath; }

	void SetPlayer(AssetPlayer* player) { m_player = player; }
	AssetPlayer* GetPlayer() { return m_player; }
	
	AudioPool* GetPool() { return m_pool; }

	bool IsStopped() const;
	bool IsPaused() const;
	bool IsFinished() const;

	bool IsStream() const { return m_stream; }

	void UpdataOffset(float dt) { m_offset += dt; }

private:
	void Stream();

private:
	static const int OUTPUT_BUF_COUNT = 16;

private:
	AudioPool* m_pool;

	bool m_looping;
	bool m_active;
	bool m_paused;

	float m_offset;	// seconds

	const bool m_stream;

	// queue
	InputBuffer*  m_ibuf;
	OutputBuffer* m_obuf;

	// asset
	std::string  m_filepath;
	AssetPlayer* m_player;

}; // Source

}
}

#endif // _UNIAUDIO_OPENSL_SOURCE_H_