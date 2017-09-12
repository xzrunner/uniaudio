#ifndef _UNIAUDIO_OPENSL_AUDIO_POOL_H_
#define _UNIAUDIO_OPENSL_AUDIO_POOL_H_

#include <uniaudio/AudioMixer.h>
#include <uniaudio/opensl/AudioPlayer.h>

#include <CU_Uncopyable.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <set>
#include <queue>
#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)
#include <mutex>
#include <mingw.mutex.h>
#else
#include <mutex>
#endif

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

	void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);

private:
	void CreateAssetsAudioPlayer();
	void CreateBufferQueueAudioPlayer();

	void EnqueueAllBuffers();

private:
	static const int NUM_OPENSL_BUFFERS = 2;

private:
	bool InitAssetsAudioPlayer(AssetPlayer* player, const Source* source);

private:
	std::mutex m_mutex;

	AudioContext* m_ctx;

	std::set<Source*> m_playing;

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