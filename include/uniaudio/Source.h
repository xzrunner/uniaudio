#ifndef _UNIAUDIO_SOURCE_H_
#define _UNIAUDIO_SOURCE_H_

#include <cu/uncopyable.h>

#include <memory>

namespace ua
{

class Decoder;

class Source : private cu::Uncopyable
{
public:
	Source();
	Source(const Source&);
	Source(const Decoder& decoder);
	virtual ~Source() {}

	virtual std::shared_ptr<Source> Clone() = 0;

	virtual bool Update() = 0;

	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Rewind() = 0;

	virtual void Seek(float offset) = 0;
	virtual float Tell() = 0;

	void  SetOffset(float offset) { m_offset = offset; }
	float GetOffset() const { return m_offset; }
	void  SetDuration(float duration) { m_duration = duration; }
	float GetDuration() const { return m_duration; }

	void  SetFadeIn(float time) { m_fade_in = time; }
	float GetFadeIn() const { return m_fade_in; }
	void  SetFadeOut(float time) { m_fade_out = time; }
	float GetFadeOut() const { return m_fade_out; }

	void  SetOriVolume(float volume) { m_ori_volume = volume; }
	float GetOriVolume() const { return m_ori_volume; }
	float GetCurrVolume() const { return m_curr_volume; }

protected:
	float m_offset, m_duration;
	float m_fade_in, m_fade_out;

	float m_ori_volume, m_curr_volume;

}; // Source

}

#endif // _UNIAUDIO_SOURCE_H_