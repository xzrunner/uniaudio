#ifndef _UNIAUDIO_SOURCE_H_
#define _UNIAUDIO_SOURCE_H_

#include <cu/uncopyable.h>

namespace ua
{

class Source : private cu::Uncopyable
{
public:
	Source() {}
	virtual ~Source() {}

	virtual bool Update() = 0;

	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Rewind() = 0;

	virtual void Seek(float offset) = 0;
	virtual float Tell() = 0;

}; // Source

}

#endif // _UNIAUDIO_SOURCE_H_