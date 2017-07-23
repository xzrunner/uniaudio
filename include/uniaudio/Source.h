#ifndef _UNIAUDIO_SOURCE_H_
#define _UNIAUDIO_SOURCE_H_

#include <CU_Uncopyable.h>
#include <CU_RefCountObj.h>

namespace ua
{

class Source : private cu::Uncopyable, public cu::RefCountObj
{
public:
	Source() {}
	virtual ~Source() {}

	virtual bool Play() = 0;
	virtual void Stop() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	
}; // Source

}

#endif // _UNIAUDIO_SOURCE_H_