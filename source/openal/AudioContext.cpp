#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/Source.h"

#include <stddef.h>

namespace ua
{
namespace openal
{

AudioContext::AudioContext()
	: m_device(NULL)
	, m_context(NULL)
{
	Init();
}

AudioContext::~AudioContext()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

bool AudioContext::Play(Source* source)
{
	return source->Play();
}

void AudioContext::Stop(Source* source)
{
	source->Stop();
}

void AudioContext::Pause(Source* source)
{
	source->Pause();
}

void AudioContext::Resume(Source* source)
{
	source->Resume();
}

bool AudioContext::Init()
{
	m_device = alcOpenDevice(NULL);
	if (!m_device) {
		return false;
	}
	
	m_context = alcCreateContext(m_device, NULL);
	if (!m_context) {
		return false;
	}

	if (!alcMakeContextCurrent(m_context) || alcGetError(m_device) != ALC_NO_ERROR) {
		return false;
	}

	return true;
}

}
}