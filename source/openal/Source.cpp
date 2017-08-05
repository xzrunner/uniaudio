#include "uniaudio/openal/Source.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/AudioData.h"

#include <logger.h>

#define FORCE_REPLAY

namespace ua
{
namespace openal
{

Source::Source(AudioPool* pool, const AudioData* data)
	: m_pool(pool)
	, m_stream(false)
	, m_buffer(0)
	, m_size(0)
	, m_source(0)
	, m_active(false)
	, m_paused(false)
{
	alGetError();

	ALenum err;

	alGenBuffers(1, &m_buffer);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("alGenBuffers error: %p", data);
		return;
	}

	ALenum fmt = GetFormat(data->GetChannels(), data->GetBitDepth());
	if (fmt == 0) {
		LOGW("err fmt: %p", data);
		return;
	}

	alBufferData(m_buffer, fmt, data->GetData(), data->GetSize(), data->GetSampleRate());
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("alBufferData error: %p", data);
		return;
	}

	alGenSources(1, &m_source);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("alGenSources error: %p", data);
		return;
	}
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(this);
	}
	alDeleteBuffers(1, &m_buffer);
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
	if (m_active) {
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

	}
	else
	{
		alSourcei(m_source, AL_BUFFER, m_buffer);
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
			// 
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
			// 
		}
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

}
}