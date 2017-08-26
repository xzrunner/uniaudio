#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/Source.h"
#include "uniaudio/AudioData.h"
#include "uniaudio/DecoderFactory.h"
#include "uniaudio/Callback.h"

#include <stddef.h>
#include <assert.h>

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
	: m_device(NULL)
	, m_context(NULL)
{
	Init();
}

AudioContext::~AudioContext()
{
	Callback::UnregisterAsyncUpdate(update_cb);

	delete m_pool;

	alcMakeContextCurrent(NULL);
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

ua::Source* AudioContext::CreateSource(const AudioData* data)
{
	return new Source(m_pool, data);
}

ua::Source* AudioContext::CreateSource(Decoder* decoder)
{
	return new Source(m_pool, decoder, true);
}

ua::Source* AudioContext::CreateSource(const std::string& filepath, bool stream)
{
	if (stream) {
		return new Source(m_pool, DecoderFactory::Create(filepath));
	} else {
		return new Source(m_pool, new AudioData(filepath));
	}
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

	m_pool = new AudioPool();

	Callback::RegisterAsyncUpdate(update_cb, m_pool);

	return true;
}

}
}