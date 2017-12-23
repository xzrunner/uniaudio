#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/Source.h"
#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/InputBuffer.h"
#include "uniaudio/OutputBuffer.h"
#include "uniaudio/Exception.h"

#include <assert.h>

namespace ua
{
namespace openal
{

class CheckOpenal
{
public:
    ~CheckOpenal() {
        EatError();
    }
        
private:
    void EatError() {
        while (alGetError() != AL_NO_ERROR) {
            ;
        }
    }
        
}; // CheckOpenal

AudioPool::AudioPool()
	: m_active(true)
{
	ALuint sources[NUM_ASSET_PLAYERS];
	alGenSources(NUM_ASSET_PLAYERS, sources);	
	if (alGetError() != AL_NO_ERROR) {
		throw Exception("Could not openal sources.");
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
}

void AudioPool::Update()
{
	CheckOpenal check;

	if (!m_active) {
		return;
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	
	auto itr = m_playing.begin();
	for ( ; itr != m_playing.end(); )
	{
		const std::shared_ptr<Source>& source = *itr;
		if (source->Update()) {
			++itr;
			continue;
		}

		source->StopImpl();
		source->RewindImpl();

		if (!source->IsMix()) {
			ALuint player = source->GetPlayer();
			m_asset_player_freelist.push(player);
		}

		m_playing.erase(itr++);
	}

	m_queue_player.Update(m_playing);
}

bool AudioPool::Play(const std::shared_ptr<Source>& source)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);

	auto itr = m_playing.find(source);
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

	source->PlayImpl();

	return true;
}

void AudioPool::Stop()
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	auto itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr)
	{
		const std::shared_ptr<Source>& s = *itr;
		if (!s->IsMix())
		{
			ALuint player = s->GetPlayer();
			m_asset_player_freelist.push(player);
		}
		s->StopImpl();
	}
	m_playing.clear();
}

void AudioPool::Stop(const std::shared_ptr<Source>& source)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	
	auto itr = m_playing.find(source);
	if (itr != m_playing.end())
	{
		if (!source->IsMix())
		{
			ALuint player = source->GetPlayer();
			m_asset_player_freelist.push(player);
		}
		source->StopImpl();
		m_playing.erase(itr);
	}
}

void AudioPool::Pause()
{
	CheckOpenal check;

	m_active = false;

	std::lock_guard<std::mutex> lock(m_mutex);
	for (auto& source : m_playing) {
		source->PauseImpl();
	}
}

void AudioPool::Pause(const std::shared_ptr<Source>& source)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	auto itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		(*itr)->PauseImpl();
	}
}

void AudioPool::Resume()
{
	CheckOpenal check;

	m_active = true;

	std::lock_guard<std::mutex> lock(m_mutex);
	for (auto& source : m_playing) {
		source->ResumeImpl();
	}
}

void AudioPool::Resume(const std::shared_ptr<Source>& source)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	auto itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		(*itr)->ResumeImpl();
	}
}

void AudioPool::Rewind()
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	for (auto& source : m_playing) {
		source->RewindImpl();
	}
}

void AudioPool::Rewind(const std::shared_ptr<Source>& source)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	source->RewindImpl();
}

void AudioPool::Seek(const std::shared_ptr<Source>& source, float offset)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	source->SeekImpl(offset);
}

float AudioPool::Tell(const std::shared_ptr<Source>& source)
{
	CheckOpenal check;

	std::lock_guard<std::mutex> lock(m_mutex);
	return source->TellImpl();
}

/************************************************************************/
/* class AudioPool::QueuePlayer                                         */
/************************************************************************/

AudioPool::QueuePlayer::
QueuePlayer()
	: m_source(0)
	, m_mixer(AudioContext::BUFFER_TIME_LEN)
{
	bool inited_buffers = false;
	memset(m_buffers, 0, sizeof(m_buffers));

	alGetError();

	try {
		ALenum err;

		alGenSources(1, &m_source);
		if ((err = alGetError()) != AL_NO_ERROR)  {
			throw Exception("AudioPool::QueuePlayer alGenSources error: %x\n", err);
		}

		alGenBuffers(MAX_BUFFERS, m_buffers);
		if ((err = alGetError()) != AL_NO_ERROR)  {
			throw Exception("AudioPool::QueuePlayer alGenBuffers error: %x\n", err);
		}
		inited_buffers = true;

		ALenum fmt = AL_FORMAT_STEREO16;
		assert(AudioMixer::DEFAULT_BIT_DEPTH == 16 && AudioMixer::DEFAULT_CHANNELS == 2);
		int16_t* buf = m_mixer.Output();
		int buf_sz = m_mixer.GetBufSize();
		for (int i = 0; i < MAX_BUFFERS; ++i) {
			alBufferData(m_buffers[i], fmt, buf, buf_sz, AudioMixer::DEFAULT_SAMPLE_RATE);
			if ((err = alGetError()) != AL_NO_ERROR)  {
				throw Exception("AudioPool::QueuePlayer alBufferData error: %x\n", err);
			}
		}
		alSourceQueueBuffers(m_source, MAX_BUFFERS, m_buffers);
		if ((err = alGetError()) != AL_NO_ERROR)  {
			throw Exception("AudioPool::QueuePlayer alSourceQueueBuffers error: %x\n", err);
		}

		alSourcePlay(m_source);
		if (alGetError() != AL_NO_ERROR)  {
			throw Exception("AudioPool::QueuePlayer alSourcePlay error: %x\n", err);
		}
	} catch (Exception&) {
		if (inited_buffers) {
			alDeleteBuffers(MAX_BUFFERS, m_buffers);
		}
		throw;
	}
}

AudioPool::QueuePlayer::
~QueuePlayer()
{
	alDeleteBuffers(MAX_BUFFERS, m_buffers);
}

void AudioPool::QueuePlayer::
Update(const std::set<std::shared_ptr<Source>>& playing)
{
	CheckOpenal check;

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
Stream(ALuint buffer, const std::set<std::shared_ptr<Source>>& playing)
{
	CheckOpenal check;

	m_mixer.Reset();

	ALenum fmt = AL_FORMAT_STEREO16;
	auto itr = playing.begin();
	for ( ; itr != playing.end(); ++itr)
	{
		auto& source = *itr;
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
		const std::unique_ptr<Decoder>& decoder = ibuf->GetDecoder();
		int hz = decoder->GetSampleRate();
		int depth = decoder->GetBitDepth();
		int channels = decoder->GetChannels();

		m_mixer.Input(buf, buf_sz, hz, depth, channels, source->GetCurrVolume());
	}

	int16_t* buf = m_mixer.Output();
	int buf_sz = m_mixer.GetBufSize();
	alBufferData(buffer, fmt, buf, buf_sz, AudioMixer::DEFAULT_SAMPLE_RATE);
}

}
}
