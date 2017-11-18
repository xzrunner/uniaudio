#include "uniaudio/Source.h"
#include "uniaudio/Decoder.h"

namespace ua
{

Source::Source() 
	: m_offset(0)
	, m_duration(0)
	, m_fade_in(0)
	, m_fade_out(0)
	, m_ori_volume(1)
	, m_curr_volume(1)
{
}

Source::Source(const Decoder& decoder)
	: m_offset(0)
	, m_duration(decoder.GetDuration())
	, m_fade_in(0)
	, m_fade_out(0)
	, m_ori_volume(1)
	, m_curr_volume(1)
{
}

Source::Source(const Source& source)
	: m_offset(source.m_offset)
	, m_duration(source.m_duration)
	, m_fade_in(source.m_fade_in)
	, m_fade_out(source.m_fade_out)
	, m_ori_volume(source.m_ori_volume)
	, m_curr_volume(source.m_curr_volume)
{
}

}