#ifndef _UNIAUDIO_OPENSL_AUDIO_POOL_H_
#define _UNIAUDIO_OPENSL_AUDIO_POOL_H_

#include <uniaudio/AudioMixer.h>
#include <uniaudio/opensl/AudioPlayer.h>

#include <cu/uncopyable.h>
#include <cu/cu_stl.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <queue>
#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#include <mutex>
#include <mingw.mutex.h>
#else
#include <mutex>
#endif
#include <memory>

namespace ua
{
namespace opensl
{
class Source;
class AudioContext;
class AudioPool : private cu::Uncopyable
{
public:
	AudioPool(AudioContext* ctx);
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

	void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);

private:
	void CreateAssetsAudioPlayer();
	void CreateBufferQueueAudioPlayer();

	void EnqueueAllBuffers();

private:
	static const int NUM_OPENSL_BUFFERS = 2;

private:
	bool InitAssetsAudioPlayer(AssetPlayer* player, const std::shared_ptr<const Source>& source);

private:
	std::mutex m_mutex;

	AudioContext* m_ctx;

	CU_SET<std::shared_ptr<Source>> m_playing;

	// asset
	static const int NUM_ASSET_PLAYERS = 16;
	std::queue<AssetPlayer*> m_asset_player_freelist;

	// queue
	QueuePlayer m_queue_player;
	AudioMixer  m_queue_mixer;

}; // AudioPool

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_POOL_H_