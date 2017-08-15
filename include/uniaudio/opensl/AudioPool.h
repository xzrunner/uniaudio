#ifndef _UNIAUDIO_OPENSL_AUDIO_POOL_H_
#define _UNIAUDIO_OPENSL_AUDIO_POOL_H_

#include <uniaudio/opensl/AudioMixer.h>

#include <CU_Uncopyable.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <set>

namespace ua
{
namespace thread { class Mutex; }
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

	void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);

private:
	bool CreateBufferQueueAudioPlayer();

	void EnqueueAllBuffers();

private:
	static const int NUM_OPENSL_BUFFERS = 2;

private:
	thread::Mutex* m_mutex;

	AudioContext* m_ctx;

	// sles
	SLObjectItf		m_bq_player_obj;
	SLPlayItf		m_bq_player_play;
	SLAndroidSimpleBufferQueueItf m_bq_player_buffer_queue;
	SLEffectSendItf m_bq_player_effect_send;
	SLMuteSoloItf	m_bq_player_mute_solo;
	SLVolumeItf		m_bq_player_volume;
	SLmilliHertz	m_bq_player_sample_rate;

	AudioMixer m_mixer;

	std::set<Source*> m_playing;

}; // AudioPool

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_POOL_H_