#include "uniaudio/openal/Source.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/AudioData.h"
#include "uniaudio/Decoder.h"

#include <logger.h>

#define FORCE_REPLAY

namespace ua
{
namespace openal
{

Source::Source(AudioPool* pool, const AudioData* data)
	: m_pool(pool)
	, m_decoder(NULL)
	, m_stream(false)
	, m_source(0)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
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

Source::Source(AudioPool* pool, Decoder* decoder)
	: m_pool(pool)
	, m_decoder(decoder)
	, m_stream(true)
	, m_source(0)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
{
	m_decoder->AddReference();

	alGetError();
	alGenBuffers(MAX_BUFFERS, m_buffers);
	if (alGetError() != AL_NO_ERROR)  {
		LOGW("alGenBuffers error: %p", decoder);
		return;
	}
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(this);
	}
	if (m_stream) {
		alDeleteBuffers(MAX_BUFFERS, m_buffers);
	} else {
		alDeleteBuffers(1, m_buffers);
	}
	if (m_decoder) {
		m_decoder->RemoveReference();
	}
}

bool Source::Update()
{
	if (!m_active) {
		return false;
	}

	if (!m_stream) {
		alSourcei(m_source, AL_LOOPING, IsLooping() ? AL_TRUE : AL_FALSE);
		return !IsStopped();
	} else if (!IsLooping() && IsFinished()) {
		return false;
	}

	ALint processed = 0;
	alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
	while (processed--)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(m_source, 1, &buffer);
		Stream(buffer);
		alSourceQueueBuffers(m_source, 1, &buffer);
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

	m_active = m_pool->Play(this, m_source);
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

void Source::PlayImpl()
{
	if (m_stream)
	{
		int used = 0;
		for (int i = 0; i < MAX_BUFFERS; ++i)
		{
			Stream(m_buffers[i]);
			++used;
			if (m_decoder->IsFinished()) {
				break;
			}
		}
		if (used > 0) {
			alSourceQueueBuffers(m_source, used, m_buffers);
		}
	}
	else
	{
		alSourcei(m_source, AL_BUFFER, m_buffers[0]);
		if (alGetError() != AL_NO_ERROR)  {
			LOGW("%s", "Source::PlayImpl bind buffer error");
			return;
		}
	}

	alSourcePlay(m_source);
	if (alGetError() != AL_NO_ERROR)  {
		LOGW("%s", "Source::PlayImpl alSourcePlay error");
		return;
	}

	m_active = true;

}

void Source::StopImpl()
{
	if (m_active)
	{
		if (m_stream)
		{
			alSourceStop(m_source);
			int queued = 0;
			alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
			while (queued--)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(m_source, 1, &buffer);
			}
		}
		else
		{
			alSourceStop(m_source);
		}
		alSourcei(m_source, AL_BUFFER, AL_NONE);
	}
	m_active = false;
}

void Source::PauseImpl()
{
 	if (m_active) {
 		alSourcePause(m_source);
 		m_paused = true;
 	}
}

void Source::ResumeImpl()
{
 	if (m_active && m_paused) {
 		alSourcePlay(m_source);
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
			m_decoder->Rewind();
			StopImpl();
			PlayImpl();
			if (paused) {
				PauseImpl();
			}
		}
		else
		{
		 	alSourceRewind(m_source);
		 	if (!m_paused) {
		 		alSourcePlay(m_source);
		 	}
		}
	}
	else
	{
		if (m_stream)
		{
			m_decoder->Rewind();
		}
	}
}

void Source::SetLooping(bool looping)
{
	if (m_active && !m_stream) {
		alSourcei(m_source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	}
	m_looping = looping;
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

bool Source::IsStopped() const
{
	if (m_active) {
		ALenum state;
		alGetSourcei(m_source, AL_SOURCE_STATE, &state);
		return (state == AL_STOPPED);
	} else {
		return true;
	}
}

bool Source::IsPaused() const
{
	if (m_active) {
		ALenum state;
		alGetSourcei(m_source, AL_SOURCE_STATE, &state);
		return (state == AL_PAUSED);
	} else {
		return false;
	}
}

bool Source::IsFinished() const
{
	if (m_stream) {
		return IsStopped() && !IsLooping() && m_decoder->IsFinished();
	} else {
		return IsStopped();
	}
}

int Source::Stream(ALuint buffer)
{
	int decoded = m_decoder->Decode();

	int fmt = GetFormat(m_decoder->GetChannels(), m_decoder->GetBitDepth());
	if (fmt != 0) {
		alBufferData(buffer, fmt, m_decoder->GetBuffer(), decoded, m_decoder->GetSampleRate());
	}

	if (m_decoder->IsFinished() && IsLooping())
	{
		m_decoder->Rewind();
	}

	return decoded;
}

}
}