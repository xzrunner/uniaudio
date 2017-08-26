#include "uniaudio/opensl/AudioPool.h"
#include "uniaudio/opensl/AudioContext.h"
#include "uniaudio/opensl/Source.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/InputBuffer.h"
#include "uniaudio/OutputBuffer.h"

#include <assert.h>
#include <stddef.h>

namespace ua
{
namespace opensl
{

AudioPool::AudioPool(AudioContext* ctx)
	: m_ctx(ctx)
	, m_queue_mixer(AudioContext::BUFFER_TIME_LEN)
{
	CreateAssetsAudioPlayer();

	CreateBufferQueueAudioPlayer();
	EnqueueAllBuffers();
}

AudioPool::~AudioPool()
{
	Stop();
	assert(m_playing.empty());

	while (!m_asset_player_freelist.empty()) {
		AssetPlayer* player = m_asset_player_freelist.front();
		delete player;
		m_asset_player_freelist.pop();
	}
}

void AudioPool::Update()
{
	mt::Lock lock(m_mutex);

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

		if (!source->IsStream()) {
			AssetPlayer* player = source->GetPlayer();
			assert(player);
			m_asset_player_freelist.push(player);
		}

		source->StopImpl();
		source->RewindImpl();
		source->RemoveReference();

		m_playing.erase(itr++);
	}
}

bool AudioPool::Play(Source* source)
{
	mt::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		return true;
	}

	if (!source->IsStream()) 
	{
		if (m_asset_player_freelist.empty()) {
			return false;
		}

		AssetPlayer* player = m_asset_player_freelist.front();
		m_asset_player_freelist.pop();
		InitAssetsAudioPlayer(player, source);

		source->SetPlayer(player);	
	}

	m_playing.insert(source);
	source->AddReference();

	source->PlayImpl();

	return true;
}

void AudioPool::Stop()
{
	mt::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr)
	{
		Source* s = *itr;
		if (!s->IsStream()) 
		{
			AssetPlayer* player = s->GetPlayer();
			assert(player);
			player->Release();
			m_asset_player_freelist.push(player);
		}
		s->StopImpl();
		s->RemoveReference();
	}
	m_playing.clear();
}

void AudioPool::Stop(Source* source)
{
	mt::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr == m_playing.end()) {
		return;
	}

	if (!source->IsStream())
	{
		AssetPlayer* player = source->GetPlayer();
		assert(player);
		player->Release();
		m_asset_player_freelist.push(player);
	}

	source->StopImpl();
	m_playing.erase(itr);
	source->RemoveReference();
}

void AudioPool::Pause()
{
	mt::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->PauseImpl();
	}
}

void AudioPool::Pause(Source* source)
{
	mt::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		(*itr)->PauseImpl();
	}
}

void AudioPool::Resume()
{
	mt::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->ResumeImpl();
	}
}

void AudioPool::Resume(Source* source)
{
	mt::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		(*itr)->ResumeImpl();
	}
}

void AudioPool::Rewind()
{
	mt::Lock lock(m_mutex);
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		(*itr)->RewindImpl();
	}
}

void AudioPool::Rewind(Source* source)
{
	mt::Lock lock(m_mutex);
	source->RewindImpl();
}

void AudioPool::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq)
{
	assert(bq == m_queue_player.queue);

	m_queue_mixer.Reset();

	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr)
	{
		Source* source = *itr;
		if (!source->IsStream() || source->IsStopped() || source->IsPaused()) {
			continue;
		}

		OutputBuffer* obuf = source->GetOutputBuffer();
		assert(obuf);
		int buf_sz;
		const unsigned char* buf = obuf->Output(buf_sz);
		if (!buf) {
			continue;
		}

		const InputBuffer* ibuf = source->GetInputBuffer();
		assert(ibuf);
		const Decoder* decoder = ibuf->GetDecoder();
		int hz = decoder->GetSampleRate();
		int depth = decoder->GetBitDepth();
		int channels = decoder->GetChannels();

		m_queue_mixer.Input(buf, buf_sz, hz, depth, channels);
	}

	int16_t* buf = m_queue_mixer.Output();
	int buf_sz = m_queue_mixer.GetBufSize();
	(*m_queue_player.queue)->Enqueue(m_queue_player.queue, buf, buf_sz);
}

void AudioPool::CreateAssetsAudioPlayer()
{
	for (int i = 0; i < NUM_ASSET_PLAYERS; ++i) {
		m_asset_player_freelist.push(new AssetPlayer);
	}
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
	// configure a typical audio source of 44.1 kHz stereo 16-bit little endian
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if(m_queue_player.sample_rate) {
        format_pcm.samplesPerSec = m_queue_player.sample_rate;       //sample rate in mili second
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

	result = (*m_ctx->GetEngine())->CreateAudioPlayer(m_ctx->GetEngine(), &m_queue_player.object, &audioSrc, &audioSnk, 2, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// realize the player
	result = (*m_queue_player.object)->Realize(m_queue_player.object, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the m_queue_player.play interface
	result = (*m_queue_player.object)->GetInterface(m_queue_player.object, SL_IID_PLAY, &m_queue_player.play);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the buffer m_queue_player.queue interface
	result = (*m_queue_player.object)->GetInterface(m_queue_player.object, SL_IID_BUFFERQUEUE,
		&m_queue_player.queue);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// register callback on the buffer m_queue_player.queue
	result = (*m_queue_player.queue)->RegisterCallback(m_queue_player.queue, bqPlayerCallback, this);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

// 	// get the effect send interface
// 	m_queue_player.effect_send = NULL;
// 	if( 0 == m_queue_player.sample_rate) {
// 		result = (*m_queue_player.object)->GetInterface(m_queue_player.object, SL_IID_EFFECTSEND,
// 			&m_queue_player.effect_send);
// 		assert(SL_RESULT_SUCCESS == result);
// 		(void)result;
// 	}

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
	// get the mute/solo interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;
#endif

	// get the m_queue_player.volume interface
	result = (*m_queue_player.object)->GetInterface(m_queue_player.object, SL_IID_VOLUME, &m_queue_player.volume);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// set the player's state to playing
	result = (*m_queue_player.play)->SetPlayState(m_queue_player.play, SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	return true;
}

void AudioPool::EnqueueAllBuffers()
{
	int buf_sz = m_queue_mixer.GetBufSize();
	void* buf = malloc(buf_sz);
	memset(buf, 0, buf_sz);
	for (int i = 0; i < NUM_OPENSL_BUFFERS; ++i) {
 		SLresult result = (*m_queue_player.queue)->Enqueue(m_queue_player.queue, buf, buf_sz);
 		if (SL_RESULT_SUCCESS != result) {
			free(buf);
			return;
 		}
	}
	free(buf);
}

static void asset_player_cb(SLPlayItf caller, void* context, SLuint32 play_event)
{
	Source* source = static_cast<Source*>(context);
	AudioPool* pool = source->GetPool();
	pool->Stop(source);
}

bool AudioPool::InitAssetsAudioPlayer(AssetPlayer* player, const Source* source)
{
	SLresult result;

	SLDataLocator_AndroidFD loc_fd;
#ifdef __ANDROID__
	bool loaded = m_ctx->LoadAssetFile(source->GetFilepath(), &loc_fd);
	if (!loaded) {
		return false;
	}
#endif // __ANDROID__

	SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
	SLDataSource audioSrc = {&loc_fd, &format_mime};

	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, m_ctx->GetOutputMix()};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	// create audio player
	const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
	const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
	result = (*m_ctx->GetEngine())->CreateAudioPlayer(m_ctx->GetEngine(), &player->object, &audioSrc, &audioSnk,
		3, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// realize the player
	result = (*player->object)->Realize(player->object, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the play interface
	result = (*player->object)->GetInterface(player->object, SL_IID_PLAY, &player->play);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the seek interface
	result = (*player->object)->GetInterface(player->object, SL_IID_SEEK, &player->seek);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the mute/solo interface
	result = (*player->object)->GetInterface(player->object, SL_IID_MUTESOLO, &player->mute_solo);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the volume interface
	result = (*player->object)->GetInterface(player->object, SL_IID_VOLUME, &player->volume);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// enable whole file looping
	result = (*player->seek)->SetLoop(player->seek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	(*player->play)->RegisterCallback(player->play, asset_player_cb, const_cast<Source*>(source));

	return result == SL_RESULT_SUCCESS;
}

}
}