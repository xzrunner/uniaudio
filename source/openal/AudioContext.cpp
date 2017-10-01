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
	, m_device(nullptr)
	, m_context(nullptr)
	, m_pool(nullptr)
{
	Initialize();
}

AudioContext::AudioContext(ALCdevice* device, ALCcontext* context)
	: m_own_ctx(false)
	, m_device(device)
	, m_context(context)
	, m_pool(nullptr)
{
	Initialize();
}

AudioContext::~AudioContext()
{
	Terminate();
}

std::shared_ptr<ua::Source> AudioContext::CreateSource(const AudioData* data)
{
	if (!m_pool) {
		return nullptr;
	} else {
		return std::make_shared<Source>(m_pool, data);
	}
}

std::shared_ptr<ua::Source> AudioContext::CreateSource(std::unique_ptr<Decoder>& decoder)
{
	if (!m_pool) {
		return nullptr;
	} else {
		return std::make_shared<Source>(m_pool, decoder, false);
	}
}

std::shared_ptr<ua::Source> AudioContext::CreateSource(const std::string& filepath, bool stream)
{
	if (!m_pool) {
		return nullptr;
	}
	if (stream) {
		return std::make_shared<Source>(m_pool, DecoderFactory::Create(filepath));
	} else {
		return std::make_shared<Source>(m_pool, new AudioData(filepath));
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
			m_device = alcOpenDevice(nullptr);
			if (!m_device) {
				throw Exception("Could not open openal device.");
			}

			m_context = alcCreateContext(m_device, nullptr);
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

	alcMakeContextCurrent(nullptr);
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
