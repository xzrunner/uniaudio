#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/Source.h"
#include "uniaudio/AudioData.h"
#include "uniaudio/DecoderFactory.h"
#include "uniaudio/Callback.h"
#include "uniaudio/Exception.h"

#include <stddef.h>

namespace ua
{
namespace openal
{

const float AudioContext::BUFFER_TIME_LEN = 0.01f;

static void
update_cb(void* arg)
{
	AudioPool* pool = static_cast<AudioPool*>(arg);
	pool->Update();	
}

AudioContext::AudioContext()
	: m_own_ctx(true)
	, m_device(NULL)
	, m_context(NULL)
	, m_pool(NULL)
{
	Initialize();
}

AudioContext::AudioContext(ALCdevice* device, ALCcontext* context)
	: m_own_ctx(false)
	, m_device(device)
	, m_context(context)
	, m_pool(NULL)
{
	Initialize();
}

AudioContext::~AudioContext()
{
	Terminate();
}

ua::Source* AudioContext::CreateSource(const AudioData* data)
{
	if (!m_pool) {
		return NULL;
	} else {
		return new Source(m_pool, data);
	}
}

ua::Source* AudioContext::CreateSource(Decoder* decoder)
{
	if (!m_pool) {
		return NULL;
	} else {
		return new Source(m_pool, decoder, false);
	}
}

ua::Source* AudioContext::CreateSource(const std::string& filepath, bool stream)
{
	if (!m_pool) {
		return NULL;
	}
	if (stream) {
		return new Source(m_pool, DecoderFactory::Create(filepath));
	} else {
		return new Source(m_pool, new AudioData(filepath));
	}
}

void AudioContext::Stop()
{
	if (m_pool) {
		m_pool->Stop();
	}
}

void AudioContext::Pause()
{
	if (m_pool) {
		m_pool->Pause();
	}
}

void AudioContext::Resume()
{
	if (m_pool) {
		m_pool->Resume();
	}
}

void AudioContext::Rewind()
{
	if (m_pool) {
		m_pool->Rewind();
	}
}

void AudioContext::Initialize()
{
	try {
		if (m_own_ctx)
		{
			m_device = alcOpenDevice(NULL);
			if (!m_device) {
				throw Exception("Could not open openal device.");
			}

			m_context = alcCreateContext(m_device, NULL);
			if (!m_context) {
				throw Exception("Could not create openal context.");
			}
		}

		if (!alcMakeContextCurrent(m_context) || alcGetError(m_device) != ALC_NO_ERROR) {
			throw Exception("Could not bind openal context.");
		}

		m_pool = new AudioPool();
		if (!m_pool) {
			throw Exception("Could not create pool.");
		}

		Callback::RegisterAsyncUpdate(update_cb, m_pool);
	} catch (Exception&) {
		Terminate();
		throw;
	}
}

void AudioContext::Terminate()
{
	Callback::UnregisterAsyncUpdate(update_cb);

	if (m_pool) {
		delete m_pool;
	}

	alcMakeContextCurrent(NULL);
	if (m_own_ctx)
	{
		if (m_context) {
			alcDestroyContext(m_context);
		}
		if (m_device) {
			alcCloseDevice(m_device);
		}
	}
}

}
}
