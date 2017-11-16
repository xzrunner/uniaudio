#ifndef _UNIAUDIO_OPENSL_SOURCE_H_
#define _UNIAUDIO_OPENSL_SOURCE_H_

#include <cu/cu_stl.h>

#include "uniaudio/Source.h"
#include "uniaudio/opensl/AudioPlayer.h"

#include <memory>

namespace ua
{

class AudioData;
class Decoder;
class InputBuffer;
class OutputBuffer;

namespace opensl
{

class AudioPool;
class Source : public ua::Source, public std::enable_shared_from_this<Source>
{
public:
	Source(AudioPool* pool, const std::string& filepath);
	Source(AudioPool* pool, std::unique_ptr<Decoder>& decoder);
	Source(const Source&);
	virtual ~Source();

	virtual std::shared_ptr<ua::Source> Clone() override final;

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

	const std::string& GetFilepath() const { return m_filepath; }

	void SetPlayer(AssetPlayer* player) { m_player = player; }
	AssetPlayer* GetPlayer() { return m_player; }
	
	AudioPool* GetPool() { return m_pool; }

	bool IsStopped() const;
	bool IsPaused() const;
	bool IsFinished() const;

	bool IsStream() const { return m_stream; }

	void UpdataOffset(float dt) { m_curr_offset += dt; }

private:
	void Stream();

	void UpdateCurrVolume();

private:
	static const int OUTPUT_BUF_COUNT = 16;

private:
	AudioPool* m_pool;

	bool m_looping;
	bool m_active;
	bool m_paused;

	float m_curr_offset;	// seconds

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