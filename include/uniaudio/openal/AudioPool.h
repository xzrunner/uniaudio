#ifndef _UNIAUDIO_OPENAL_AUDIO_POOL_H_
#define _UNIAUDIO_OPENAL_AUDIO_POOL_H_

#include <uniaudio/AudioMixer.h>

#include <cu/uncopyable.h>

#include <OpenAL/al.h>

#include <queue>
#include <set>
#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#include <mutex>
#include <mingw.mutex.h>
#else
#include <mutex>
#endif
#include <atomic>

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

	bool Play(const std::shared_ptr<Source>& source);
	void Stop();
	void Stop(const std::shared_ptr<Source>& source);
	void Pause();
	void Pause(const std::shared_ptr<Source>& source);
	void Resume();
	void Resume(const std::shared_ptr<Source>& source);
	void Rewind();
	void Rewind(const std::shared_ptr<Source>& source);

	void Seek(const std::shared_ptr<Source>& source, float offset);
	float Tell(const std::shared_ptr<Source>& source);

private:
	class QueuePlayer
	{
	public:
		QueuePlayer();
		~QueuePlayer();

		void Update(const std::set<std::shared_ptr<Source>>& playing);

	private:
		void Stream(ALuint buffer, const std::set<std::shared_ptr<Source>>& playing);

	private:
		ALuint     m_source;
		AudioMixer m_mixer;

		static const unsigned int MAX_BUFFERS = 16;
		ALuint m_buffers[MAX_BUFFERS];

	}; // QueuePlayer

private:
	std::mutex m_mutex;

	std::set<std::shared_ptr<Source>> m_playing;

	// asset
	static const int NUM_ASSET_PLAYERS = 16;
	std::queue<ALuint> m_asset_player_freelist;

	// queue
	QueuePlayer m_queue_player;

	std::atomic<bool> m_active;

}; // AudioPool

}
}

#endif // _UNIAUDIO_OPENAL_AUDIO_POOL_H_