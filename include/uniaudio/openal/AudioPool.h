#ifndef _UNIAUDIO_AUDIO_POOL_H_
#define _UNIAUDIO_AUDIO_POOL_H_

#include <CU_Uncopyable.h>
#include <OpenAL/al.h>

#include <queue>
#include <map>

namespace ua
{
namespace thread { class Mutex; }
namespace openal
{

class Source;
class AudioPool : private cu::Uncopyable
{
public:
	AudioPool();
	~AudioPool();

	void Update();

	bool Play(Source* source, ALuint& out);
	void Stop();
	void Stop(Source* source);
	void Pause();
	void Pause(Source* source);
	void Resume();
	void Resume(Source* source);
	void Rewind();
	void Rewind(Source* source);

private:
	thread::Mutex* m_mutex;

	static const int NUM_SOURCES = 64;
	ALuint m_sources[NUM_SOURCES];

	std::queue<ALuint> m_available;

	std::map<Source*, ALuint> m_playing;

}; // AudioPool

}
}

#endif // _UNIAUDIO_AUDIO_POOL_H_