#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/Source.h"
#include "uniaudio/Thread.h"

#include <fault.h>

namespace ua
{
namespace openal
{

AudioPool::AudioPool()
{
	m_mutex = new thread::Mutex();

	alGenSources(NUM_SOURCES, m_sources);
	if (alGetError() != AL_NO_ERROR) {
		fault("openal gen sources.");
	}

	for (int i = 0; i < NUM_SOURCES; ++i) {
		m_available.push(m_sources[i]);		
	}
}

AudioPool::~AudioPool()
{
	

	delete m_mutex;

	alDeleteSources(NUM_SOURCES, m_sources);
}

void AudioPool::Update()
{
	thread::Lock lock(m_mutex);
	
	std::map<Source*, ALuint>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); )
	{
		Source* source = itr->first;
		if (source->Update()) {
			++itr;
			continue;
		}

		source->StopImpl();
		source->RewindImpl();
		source->RemoveReference();
		m_available.push(itr->second);

		m_playing.erase(itr++);
	}
}

bool AudioPool::Play(Source* source, ALuint& out)
{
	thread::Lock lock(m_mutex);

	std::map<Source*, ALuint>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		return true;
	}

	if (m_available.empty()) {
		return false;
	}

	out = m_available.front(); 
	m_available.pop();

	m_playing.insert(std::make_pair(source, out));
	source->AddReference();

	source->PlayImpl();

	return true;
}

void AudioPool::Stop()
{
	thread::Lock lock(m_mutex);
	std::map<Source*, ALuint>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr)
	{
		itr->first->StopImpl();
		itr->first->RemoveReference();
		m_available.push(itr->second);
	}
	m_playing.clear();
}

void AudioPool::Stop(Source* source)
{
	thread::Lock lock(m_mutex);
	
	std::map<Source*, ALuint>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end())
	{
		source->StopImpl();
		m_available.push(itr->second);
		m_playing.erase(itr);
		source->RemoveReference();
	}
}

void AudioPool::Pause()
{
	thread::Lock lock(m_mutex);
	std::map<Source*, ALuint>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		itr->first->PauseImpl();
	}
}

void AudioPool::Pause(Source* source)
{
	thread::Lock lock(m_mutex);
	std::map<Source*, ALuint>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		itr->first->PauseImpl();
	}
}

void AudioPool::Resume()
{
	thread::Lock lock(m_mutex);
	std::map<Source*, ALuint>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		itr->first->ResumeImpl();
	}
}

void AudioPool::Resume(Source* source)
{
	thread::Lock lock(m_mutex);
	std::map<Source*, ALuint>::iterator itr = m_playing.find(source);
	if (itr != m_playing.end()) {
		itr->first->ResumeImpl();
	}
}

void AudioPool::Rewind()
{
	thread::Lock lock(m_mutex);
	std::map<Source*, ALuint>::iterator itr = m_playing.begin();
	for ( ; itr != m_playing.end(); ++itr) {
		itr->first->RewindImpl();
	}
}

void AudioPool::Rewind(Source* source)
{
	thread::Lock lock(m_mutex);
	source->RewindImpl();
}

}
}