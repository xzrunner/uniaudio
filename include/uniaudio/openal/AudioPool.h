#ifndef _UNIAUDIO_OPENAL_AUDIO_POOL_H_
#define _UNIAUDIO_OPENAL_AUDIO_POOL_H_

#include <uniaudio/AudioMixer.h>
#include <multitask/Thread.h>

#include <CU_Uncopyable.h>

#include <OpenAL/al.h>

#include <queue>
#include <set>

namespace ua
{
namespace openal
{

class Source;
class AudioPool : private cu::Uncopyable
{
public:
	AudioPool();
	~AudioPool();

	void Update();

	bool Play(Source* source);
	void Stop();
	void Stop(Source* source);
	void Pause();
	void Pause(Source* source);
	void Resume();
	void Resume(Source* source);
	void Rewind();
	void Rewind(Source* source);

	void Seek(Source* source, float offset);
	float Tell(Source* source);

private:
	class QueuePlayer
	{
	public:
		QueuePlayer();
		~QueuePlayer();

		void Update(const std::set<Source*>& playing);

	private:
		void Stream(ALuint buffer, const std::set<Source*>& playing);

	private:
		ALuint     m_source;
		AudioMixer m_mixer;

		static const unsigned int MAX_BUFFERS = 16;
		ALuint m_buffers[MAX_BUFFERS];

	}; // QueuePlayer

private:
	mt::Mutex m_mutex;

	std::set<Source*> m_playing;

	// asset
	static const int NUM_ASSET_PLAYERS = 64;
	std::queue<ALuint> m_asset_player_freelist;

	// queue
	QueuePlayer m_queue_player;

}; // AudioPool

}
}

#endif // _UNIAUDIO_OPENAL_AUDIO_POOL_H_