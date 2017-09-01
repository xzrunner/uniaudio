#include "uniaudio/openal/Source.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/AudioData.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/OutputBuffer.h"
#include "uniaudio/InputBuffer.h"

#include <logger.h>

#include <assert.h>

#define FORCE_REPLAY

namespace ua
{
namespace openal
{

Source::Source(AudioPool* pool, const AudioData* data)
	: m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_freq(data->GetSampleRate())
	, m_offset(0)
	, m_stream(false)
	, m_mix(false)
	, m_ibuf(NULL)
	, m_obuf(NULL)
	, m_player(0)
{
	alGetError();

	ALenum err;

	alGenBuffers(1, m_buffers);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("alGenBuffers error: %p", data);
		return;
	}

	ALenum fmt = GetFormat(data->GetChannels(), data->GetBitDepth());
	if (fmt == 0) {
		LOGW("err fmt: %p", data);
		return;
	}

	alBufferData(m_buffers[0], fmt, data->GetData(), data->GetSize(), data->GetSampleRate());
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("alBufferData error: %p", data);
		return;
	}
}

Source::Source(AudioPool* pool, Decoder* decoder, bool mix)
	: m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_freq(decoder->GetSampleRate())
	, m_offset(0)
	, m_stream(true)
	, m_mix(mix)
	, m_ibuf(NULL)
	, m_obuf(NULL)
	, m_player(0)
{
	m_ibuf = new InputBuffer(decoder);

	if (m_mix)
	{
		const int HZ = decoder->GetSampleRate();
		const int depth = decoder->GetBitDepth();
		const int channels = decoder->GetChannels();
		const int samples = static_cast<int>(HZ * AudioContext::BUFFER_TIME_LEN);
		int buf_sz = depth * channels * samples / 8;
		m_obuf = new OutputBuffer(OUTPUT_BUF_COUNT, buf_sz);
	}
	else
	{
		alGetError();
		alGenBuffers(MAX_BUFFERS, m_buffers);
		if (alGetError() != AL_NO_ERROR)  {
			LOGW("alGenBuffers error: %p", decoder);
			return;
		}
	}
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(this);
	}
	if (m_stream) {
		if (!m_mix) {
			alDeleteBuffers(MAX_BUFFERS, m_buffers);
		}
	} else {
		assert(!m_mix);
		alDeleteBuffers(1, m_buffers);
	}
	if (m_ibuf) {
		delete m_ibuf;
	}
	if (m_obuf) {
		delete m_obuf;
	}
}

bool Source::Update()
{
	if (!m_active) {
		return false;
	}

	if (!m_stream) {
		assert(!m_mix);
		alSourcei(m_player, AL_LOOPING, IsLooping() ? AL_TRUE : AL_FALSE);
		return !IsStopped();
	} else if (!IsLooping() && IsFinished()) {
		return false;
	}

	if (m_mix)
	{
		assert(m_ibuf && m_obuf);
		m_ibuf->Output(m_obuf, IsLooping());
	}
	else
	{
		ALint processed = 0;
		alGetSourcei(m_player, AL_BUFFERS_PROCESSED, &processed);
		while (processed--)
		{
			float old_offset_seconds = GetCurrOffset(m_freq);

			ALuint buffer;
			alSourceUnqueueBuffers(m_player, 1, &buffer);

			float new_offset_seconds = GetCurrOffset(m_freq);
			m_offset += old_offset_seconds - new_offset_seconds;

			if (Stream(buffer) > 0) {
				alSourceQueueBuffers(m_player, 1, &buffer);
			}
		}
	}

 	return true;
}

void Source::Play()
{
	if (m_active) 
	{
		if (m_paused) {
			m_pool->Resume(this);
		}
#ifdef FORCE_REPLAY
		else {
			m_pool->Rewind(this);
		}
#endif // FORCE_REPLAY
		return;
	}

	m_active = m_pool->Play(this);
}

void Source::Stop()
{
	if (!IsStopped()) {
		m_pool->Stop(this);
	}
}

void Source::Pause()
{
	m_pool->Pause(this);
}

void Source::Resume()
{
	m_pool->Resume(this);
}

void Source::Rewind()
{
	m_pool->Rewind(this);
}

void Source::Seek(float offset)
{
	m_pool->Seek(this, offset);
}

float Source::Tell()
{
	return m_pool->Tell(this);
}

void Source::PlayImpl()
{
	if (m_stream)
	{
		if (!m_mix)
		{
			int used = 0;
			for (int i = 0; i < MAX_BUFFERS; ++i)
			{
				if (Stream(m_buffers[i]) == 0) {
					break;
				}
				++used;
				assert(m_ibuf);
				if (m_ibuf->GetDecoder()->IsFinished()) {
					break;
				}
			}
			if (used > 0) {
				alSourceQueueBuffers(m_player, used, m_buffers);
			}
		}
	}
	else
	{
		assert(!m_mix);
		alSourcei(m_player, AL_BUFFER, m_buffers[0]);
		if (alGetError() != AL_NO_ERROR)  {
			LOGW("%s", "Source::PlayImpl bind buffer error");
			return;
		}
	}

	if (!m_mix)
	{
		alSourcePlay(m_player);
		if (alGetError() != AL_NO_ERROR)  {
			LOGW("%s", "Source::PlayImpl alSourcePlay error");
			return;
		}
	}

	m_active = true;
}

void Source::StopImpl()
{
	if (!m_active) {
		return;
	}

	if (m_stream)
	{
		if (!m_mix)
		{
			alSourceStop(m_player);
			int queued = 0;
			alGetSourcei(m_player, AL_BUFFERS_QUEUED, &queued);
			while (queued--)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(m_player, 1, &buffer);
			}
		}
	}
	else
	{
		assert(!m_mix);
		alSourceStop(m_player);
	}
	if (!m_mix) {
		alSourcei(m_player, AL_BUFFER, AL_NONE);
	}

	m_active = false;
}

void Source::PauseImpl()
{
 	if (m_active) {
		if (!m_mix) {
			alSourcePause(m_player);
		}
 		m_paused = true;
 	}
}

void Source::ResumeImpl()
{
 	if (m_active && m_paused) {
		if (!m_mix) {
 			alSourcePlay(m_player);
		}
 		m_paused = false;
 	}
}

void Source::RewindImpl()
{
	if (m_active)
	{
		if (m_stream)
		{
			bool paused = m_paused;
			assert(m_ibuf);
			m_ibuf->GetDecoder()->Rewind();
			StopImpl();
			PlayImpl();
			if (paused) {
				PauseImpl();
			}
		}
		else
		{
			assert(!m_mix);
		 	alSourceRewind(m_player);
		 	if (!m_paused) {
		 		alSourcePlay(m_player);
		 	}
		}
	}
	else
	{
		if (m_stream)
		{
			assert(m_ibuf);
			m_ibuf->GetDecoder()->Rewind();
		}
	}
}

void Source::SeekImpl(float offset)
{
	if (!m_active) {
		return;
	}

	if (m_stream) 
	{
		m_offset = offset;

		bool looping = IsLooping();
		m_ibuf->Seek(offset, looping);
		if (m_mix) {
			m_ibuf->Output(m_obuf, looping);
		}

		bool paused = m_paused;
		StopImpl();
		PlayImpl();
		if (paused) {
			PauseImpl();
		}
	} 
	else 
	{
		alSourcef(m_player, AL_SEC_OFFSET, offset);
	}
}

float Source::TellImpl()
{
	if (!m_active) {
		return 0;
	}

	float offset;
	alGetSourcef(m_player, AL_SAMPLE_OFFSET, &offset);
	offset /= m_freq;
	if (m_stream) {
		offset += m_offset;
	}
	return offset;
}

void Source::SetLooping(bool looping)
{
	if (m_active && !m_stream) {
		assert(!m_mix);
		alSourcei(m_player, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	}
	m_looping = looping;
}

void Source::SetPlayer(ALuint player) 
{ 
	assert(!m_mix); 
	m_player = player;
}

bool Source::IsStopped() const
{
	// todo
	if (m_mix) {
		return false;
	}

	if (m_active) {
		ALenum state;
		alGetSourcei(m_player, AL_SOURCE_STATE, &state);
		return (state == AL_STOPPED);
	} else {
		return true;
	}
}

bool Source::IsPaused() const
{
	// todo
	if (m_mix) {
		return false;
	}

	if (m_active) {
		ALenum state;
		alGetSourcei(m_player, AL_SOURCE_STATE, &state);
		return (state == AL_PAUSED);
	} else {
		return false;
	}
}

bool Source::IsFinished() const
{
	if (m_stream) {
		assert(m_ibuf);
		return IsStopped() && !IsLooping() && m_ibuf->GetDecoder()->IsFinished();
	} else {
		return IsStopped();
	}
}

ALenum Source::GetFormat(int channels, int bit_depth)
{
	if (channels == 1 && bit_depth == 8) {
		return AL_FORMAT_MONO8;
	} else if (channels == 1 && bit_depth == 16) {
		return AL_FORMAT_MONO16;
	} else if (channels == 2 && bit_depth == 8) {
		return AL_FORMAT_STEREO8;
	} else if (channels == 2 && bit_depth == 16) {
		return AL_FORMAT_STEREO16;
	}
	return 0;
}

int Source::Stream(ALuint buffer)
{
	assert(m_ibuf && !m_mix);
	Decoder* d = m_ibuf->GetDecoder();
	int decoded = std::max(d->Decode(), 0);
	if (decoded > 0)
	{
		int fmt = GetFormat(d->GetChannels(), d->GetBitDepth());
		if (fmt != 0) {
			alBufferData(buffer, fmt, d->GetBuffer(), decoded, d->GetSampleRate());
		} else {
			decoded = 0;			
		}
	}

	if (d->IsFinished() && IsLooping())
	{
		d->Rewind();
	}

	return decoded;
}

//int Source::GetFreq() const
//{
//	int freq;
//	ALint b;
//	alGetSourcei(m_player, AL_BUFFER, &b);
//	alGetBufferi(b, AL_FREQUENCY, &freq);
//
//	assert(freq == m_freq);
//	
//	return freq;
//}

float Source::GetCurrOffset(int freq)
{
	assert(freq != 0);
	float offset_samples;
	alGetSourcef(m_player, AL_SAMPLE_OFFSET, &offset_samples);
	float offset_seconds = offset_samples / freq;
	return offset_seconds;
}

}
}