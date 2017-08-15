#include "uniaudio/opensl/AudioPool.h"
#include "uniaudio/opensl/AudioContext.h"
#include "uniaudio/opensl/Source.h"
#include "uniaudio/Thread.h"
#include "uniaudio/Decoder.h"

#include <assert.h>
#include <stddef.h>

namespace ua
{
namespace opensl
{

AudioPool::AudioPool(AudioContext* ctx)
	: m_ctx(ctx)
	, m_bq_player_obj(NULL)
	, m_bq_player_sample_rate(0)
{
	m_mutex = new thread::Mutex();

	CreateBufferQueueAudioPlayer();

	EnqueueAllBuffers();
}

AudioPool::~AudioPool()
{
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->RemoveReference();
	}

	delete m_mutex;
}

void AudioPool::Update()
{
	thread::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); )
	{
		Source* source = *itr;
		if (source->IsStopped() || source->IsPaused()) {
			++itr;
			continue;
		}
		if (source->Update()) {
			++itr;
			continue;
		}

		source->StopImpl();
		source->RewindImpl();
		source->RemoveReference();

		m_playing.erase(itr++);
	}
}

bool AudioPool::Play(Source* source)
{
	thread::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		return true;
	}

	m_playing.insert(source);
	source->AddReference();

	source->PlayImpl();

	return true;
}

void AudioPool::Stop()
{
	thread::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr)
	{
		(*itr)->StopImpl();
		(*itr)->RemoveReference();
	}
	m_playing.clear();
}

void AudioPool::Stop(Source* source)
{
	thread::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end())
	{
		source->StopImpl();
		m_playing.erase(itr);
		source->RemoveReference();
	}
}

void AudioPool::Pause()
{
	thread::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->PauseImpl();
	}
}

void AudioPool::Pause(Source* source)
{
	thread::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		(*itr)->PauseImpl();
	}
}

void AudioPool::Resume()
{
	thread::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->ResumeImpl();
	}
}

void AudioPool::Resume(Source* source)
{
	thread::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		(*itr)->ResumeImpl();
	}
}

void AudioPool::Rewind()
{
	thread::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->RewindImpl();
	}
}

void AudioPool::Rewind(Source* source)
{
	thread::Lock lock(m_mutex);
	source->RewindImpl();
}

void AudioPool::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq)
{
	assert(bq ==  m_bq_player_buffer_queue);
	if (m_playing.empty()) {
		return;
	}

	bool no_data = true;
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr)
	{
		Source* source = *itr;
		if (source->IsStopped() || source->IsPaused()) {
			continue;
		}

		const Decoder* decoder = source->GetDecoder();

		AudioQueue* bufs = source->GetBuffers();
		int buf_sz;
		const unsigned char* buf = bufs->Top(buf_sz);
		if (buf != NULL) {
			no_data = false;
			m_mixer.Input(buf, buf_sz, decoder->GetSampleRate(), decoder->GetBitDepth(), decoder->GetChannels());
		}
		bufs->Pop();
	}

	if (no_data) {
		return;
	}

	int16_t* buf = m_mixer.Output();
	int buf_sz = m_mixer.GetBufSize();
	(*m_bq_player_buffer_queue)->Enqueue(m_bq_player_buffer_queue, buf, buf_sz);
}

// this callback handler is called every time a buffer finishes playing
static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	(static_cast<AudioPool*>(context))->ProcessSLCallback(bq);
}

bool AudioPool::CreateBufferQueueAudioPlayer()
{
	SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, NUM_OPENSL_BUFFERS};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if(m_bq_player_sample_rate) {
        format_pcm.samplesPerSec = m_bq_player_sample_rate;       //sample rate in mili second
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, m_ctx->GetOutputMix()};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
                                    /*SL_IID_MUTESOLO,*/};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                                   /*SL_BOOLEAN_TRUE,*/ };

	result = (*m_ctx->GetEngine())->CreateAudioPlayer(m_ctx->GetEngine(), &m_bq_player_obj, &audioSrc, &audioSnk,
		m_bq_player_sample_rate? 2 : 3, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// realize the player
	result = (*m_bq_player_obj)->Realize(m_bq_player_obj, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the play interface
	result = (*m_bq_player_obj)->GetInterface(m_bq_player_obj, SL_IID_PLAY, &m_bq_player_play);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the buffer queue interface
	result = (*m_bq_player_obj)->GetInterface(m_bq_player_obj, SL_IID_BUFFERQUEUE,
		&m_bq_player_buffer_queue);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// register callback on the buffer queue
	result = (*m_bq_player_buffer_queue)->RegisterCallback(m_bq_player_buffer_queue, bqPlayerCallback, this);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the effect send interface
	m_bq_player_effect_send = NULL;
	if( 0 == m_bq_player_sample_rate) {
		result = (*m_bq_player_obj)->GetInterface(m_bq_player_obj, SL_IID_EFFECTSEND,
			&m_bq_player_effect_send);
		assert(SL_RESULT_SUCCESS == result);
		(void)result;
	}

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
	// get the mute/solo interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;
#endif

	// get the volume interface
	result = (*m_bq_player_obj)->GetInterface(m_bq_player_obj, SL_IID_VOLUME, &m_bq_player_volume);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// set the player's state to playing
	result = (*m_bq_player_play)->SetPlayState(m_bq_player_play, SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	return true;
}

void AudioPool::EnqueueAllBuffers()
{
	void* buf = reinterpret_cast<void*>(m_mixer.Output());
	int buf_sz = m_mixer.GetBufSize();
	memset(buf, 0, buf_sz);
	for (int i = 0; i < NUM_OPENSL_BUFFERS; ++i)
	{
 		SLresult result = (*m_bq_player_buffer_queue)->Enqueue(m_bq_player_buffer_queue, buf, buf_sz);
 		if (SL_RESULT_SUCCESS != result) {
			return;
 		}
	}
}

}
}