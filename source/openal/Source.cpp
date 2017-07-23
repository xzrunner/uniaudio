#include "uniaudio/openal/Source.h"
#include "uniaudio/AudioData.h"

#include <logger.h>

namespace ua
{
namespace openal
{

Source::Source(AudioData* data)
	: m_buffer(0)
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
	alDeleteBuffers(1, &m_buffer);
}

bool Source::Play()
{
	ALenum err;

	alSourcei(m_source, AL_BUFFER, m_buffer);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("%s", "Source::Play bind buffer error");
		return false;
	}

	alGetError();

	alSourcePlay(m_source);
	if ((err = alGetError()) != AL_NO_ERROR)  {
		LOGW("%s", "Source::Play alSourcePlay error");
		return false;
	}

	m_active = true;

	return true;
}

void Source::Stop()
{
	alSourceStop(m_source);
	alSourcei(m_source, AL_BUFFER, AL_NONE);

	m_active = false;
}

void Source::Pause()
{
	if (m_active) {
		alSourcePause(m_source);
		m_paused = true;
	}
}

void Source::Resume()
{
	if (m_active && m_paused) {
		alSourcePlay(m_source);
		m_paused = false;
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