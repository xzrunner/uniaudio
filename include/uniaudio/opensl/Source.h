#ifndef _UNIAUDIO_OPENSL_SOURCE_H_
#define _UNIAUDIO_OPENSL_SOURCE_H_

#include "uniaudio/Source.h"
#include "uniaudio/opensl/AudioQueue.h"
#include "uniaudio/opensl/AudioPlayer.h"

#include <string>

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
	Source(AudioPool* pool, const std::string& filepath);
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

	const Decoder* GetDecoder() const { return m_in_buf.GetDecoder(); }
	AudioQueue* GetBuffers() { return m_out_buf; }

	const std::string& GetFilepath() const { return m_filepath; }

	void SetPlayer(AssetPlayer* player) { m_player = player; }
	AssetPlayer* GetPlayer() { return m_player; }
	
	AudioPool* GetPool() { return m_pool; }

	bool IsStopped() const;
	bool IsPaused() const;
	bool IsFinished() const;

	bool IsStream() const { return m_stream; }

private:
	void Stream();

private:
	static const int QUEUE_BUF_COUNT = 16;

private:
	class Buffer
	{
	public:
		Buffer(Decoder* decoder);
		~Buffer();

		void Output(AudioQueue* out, bool looping);

		bool IsDecoderFinished() const;
		void DecoderRewind();

		const Decoder* GetDecoder() const { return m_decoder; }

	private:
		void Reload(bool looping);

	private:
		Decoder* m_decoder;
		int m_size, m_used;

	}; // Buffer

private:
	AudioPool* m_pool;

	bool m_looping;
	bool m_active;
	bool m_paused;

	const bool m_stream;

	// stream
	Buffer      m_in_buf;
	AudioQueue* m_out_buf;

	// asset
	std::string  m_filepath;
	AssetPlayer* m_player;

}; // Source

}
}

#endif // _UNIAUDIO_OPENSL_SOURCE_H_