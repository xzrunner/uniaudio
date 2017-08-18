#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/Source.h"
#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/Thread.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/InputBuffer.h"
#include "uniaudio/OutputBuffer.h"

#include <OpenAL/al.h>

#include <fault.h>
#include <logger.h>

#include <assert.h>

namespace ua
{
namespace openal
{

AudioPool::AudioPool()
{
	m_mutex = new thread::Mutex();

	ALuint sources[NUM_ASSET_PLAYERS];
	alGenSources(NUM_ASSET_PLAYERS, sources);
	if (alGetError() != AL_NO_ERROR) {
		fault("openal gen sources.");
	}

	for (int i = 0; i < NUM_ASSET_PLAYERS; ++i) {
		m_asset_player_freelist.push(sources[i]);
	}
}

AudioPool::~AudioPool()
{
	Stop();
	assert(m_playing.empty());

	ALuint sources[NUM_ASSET_PLAYERS];
	assert(m_asset_player_freelist.size() == NUM_ASSET_PLAYERS);
	int ptr = 0;
	while (!m_asset_player_freelist.empty()) {
		ALuint player = m_asset_player_freelist.front();
		sources[ptr++] = player;
		m_asset_player_freelist.pop();
	}
	alDeleteSources(NUM_ASSET_PLAYERS, sources);

	delete m_mutex;
}

void AudioPool::Update()
{
	thread::Lock lock(m_mutex);
	
	std::set<Source*>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); )
	{
		Source* source = *itr;
		if (source->Update()) {
			++itr;
			continue;
		}

		source->StopImpl();
		source->RewindImpl();
		source->RemoveReference();

		if (!source->IsMix()) {
			ALuint player = source->GetPlayer();
			m_asset_player_freelist.push(player);
		}

		m_playing.erase(itr++);
	}

	m_queue_player.Update(m_playing);
}

bool AudioPool::Play(Source* source)
{
	thread::Lock lock(m_mutex);

	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		return true;
	}

	if (!source->IsMix())
	{
		if (m_asset_player_freelist.empty()) {
			return false;
		}
		ALuint player = m_asset_player_freelist.front(); 
		m_asset_player_freelist.pop();
		source->SetPlayer(player);
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
		Source* s = *itr;
		if (!s->IsMix())
		{
			ALuint player = s->GetPlayer();
			m_asset_player_freelist.push(player);
		}
		s->StopImpl();
		s->RemoveReference();
	}
	m_playing.clear();
}

void AudioPool::Stop(Source* source)
{
	thread::Lock lock(m_mutex);
	
	std::set<Source*>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end())
	{
		if (!source->IsMix())
		{
			ALuint player = source->GetPlayer();
			m_asset_player_freelist.push(player);
		}
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

/************************************************************************/
/* class AudioPool::QueuePlayer                                         */
/************************************************************************/

AudioPool::QueuePlayer::
QueuePlayer()
	: m_source(0)
	, m_mixer(AudioContext::BUFFER_TIME_LEN)
{
	alGetError();

	ALenum err;

	alGenSources(1, &m_source);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("AudioPool::QueuePlayer alGenSources error: %d\n", err);
		return;
	}

	alGenBuffers(MAX_BUFFERS, m_buffers);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("AudioPool::QueuePlayer alGenBuffers error: %d\n", err);
		return;
	}

 	ALenum fmt = AL_FORMAT_STEREO16;
 	assert(AudioMixer::DEFAULT_BIT_DEPTH == 16 && AudioMixer::DEFAULT_CHANNELS == 2);
	int16_t* buf = m_mixer.Output();
	int buf_sz = m_mixer.GetBufSize();
	for (int i = 0; i < MAX_BUFFERS; ++i) {
		alBufferData(m_buffers[i], fmt, buf, buf_sz, AudioMixer::DEFAULT_SAMPLE_RATE);
		if ((err = alGetError()) != AL_NO_ERROR)  {
			LOGW("AudioPool::QueuePlayer alBufferData error: %d\n", err);
			return;
		}
	}
	alSourceQueueBuffers(m_source, MAX_BUFFERS, m_buffers);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("AudioPool::QueuePlayer alSourceQueueBuffers error: %d\n", err);
		return;
	}

	alSourcePlay(m_source);
	if (alGetError() != AL_NO_ERROR)  {
		LOGW("AudioPool::QueuePlayer alSourcePlay error: %d\n", err);
		return;
	}
}

AudioPool::QueuePlayer::
~QueuePlayer()
{
	alDeleteBuffers(MAX_BUFFERS, m_buffers);
}

void AudioPool::QueuePlayer::
Update(const std::set<Source*>& playing)
{
	ALint processed = 0;
	alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
	while (processed--)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(m_source, 1, &buffer);
		Stream(buffer, playing);
		alSourceQueueBuffers(m_source, 1, &buffer);
	}
}

void AudioPool::QueuePlayer::
Stream(ALuint buffer, const std::set<Source*>& playing)
{
	ALenum fmt = AL_FORMAT_STEREO16;
	std::set<Source*>::const_iterator itr = playing.begin();
	for ( ; itr != playing.end(); ++itr)
	{
		Source* source = *itr;
		if (!source->IsMix() || source->IsStopped() || source->IsPaused()) {
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

		m_mixer.Input(buf, buf_sz, hz, depth, channels);
	}

	int16_t* buf = m_mixer.Output();
	int buf_sz = m_mixer.GetBufSize();
	alBufferData(buffer, fmt, buf, buf_sz, AudioMixer::DEFAULT_SAMPLE_RATE);
}

}
}