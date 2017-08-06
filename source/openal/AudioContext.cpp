#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/Source.h"

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
	delete m_pool;
	delete m_pool_thread;

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
	return new Source(m_pool, decoder);
}

static void* 
pool_thread_main(void* arg)
{
	while (true)
	{
		AudioPool* pool = static_cast<AudioPool*>(arg);
		pool->Update();
		thread::Thread::Delay(5);
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
	m_pool_thread = new thread::Thread(pool_thread_main, m_pool);

	return true;
}

}
}