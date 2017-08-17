#include "uniaudio/opensl/Source.h"
#include "uniaudio/opensl/AudioPool.h"
#include "uniaudio/opensl/AudioContext.h"
#include "uniaudio/Decoder.h"

#include <assert.h>

namespace ua
{
namespace opensl
{

Source::Source(AudioPool* pool, const std::string& filepath)
	: m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_stream(false)
	, m_in_buf(NULL)
	, m_out_buf(NULL)
	, m_filepath(filepath)
	, m_player(NULL)
{
}

Source::Source(AudioPool* pool, Decoder* decoder)
	: m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_stream(true)
	, m_in_buf(decoder)
	, m_player(NULL)
{
	const int HZ = decoder->GetSampleRate();
	const int depth = decoder->GetBitDepth();
	const int channels = decoder->GetChannels();
	const int samples = static_cast<int>(HZ * AudioContext::BUFFER_TIME_LEN);
	int buf_sz = depth * channels * samples / 8;
	m_out_buf = new AudioQueue(QUEUE_BUF_COUNT, buf_sz);
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(this);
	}
	if (m_out_buf) {
		delete m_out_buf;
	}
}

bool Source::Update()
{
	if (!m_active) {
		return false;
	}

	if (!m_stream) {
		return !IsStopped();
	} else if (!IsLooping() && IsFinished()) {
		return false;
	}

	Stream();

	return true;
}

void Source::Play()
{
	if (m_active) 
	{
		if (m_paused) {
			m_pool->Resume(this);
		}
#ifdef FORCE_REPLAY
		else {
			m_pool->Rewind(this);
		}
#endif // FORCE_REPLAY
		return;
	}
	m_active = m_pool->Play(this);
}

void Source::Stop()
{
	if (!IsStopped()) {
		m_pool->Stop(this);
	}
}

void Source::Pause()
{
	m_pool->Pause(this);
}

void Source::Resume()
{
	m_pool->Resume(this);
}

void Source::Rewind()
{
	m_pool->Rewind(this);
}

void Source::PlayImpl()
{
	if (m_stream)
	{
	}
	else
	{
		assert(m_player && m_player->play);
		(*m_player->play)->SetPlayState(m_player->play, SL_PLAYSTATE_PLAYING);
	}

	m_active = true;
}

void Source::StopImpl()
{
	if (!m_active) {
		return;
	}

	if (m_stream)
	{
	}
	else
	{
		assert(m_player && m_player->play);
		(*m_player->play)->SetPlayState(m_player->play, SL_PLAYSTATE_STOPPED);
	}
	m_active = false;
}

void Source::PauseImpl()
{
	if (!m_active) {
		return;
	}

	if (m_stream) 
	{
	}
	else
	{
		assert(m_player && m_player->play);
		(*m_player->play)->SetPlayState(m_player->play, SL_PLAYSTATE_PAUSED);
	}
	m_paused = true;
}

void Source::ResumeImpl()
{
	if (!m_active || !m_paused) {
		return;
	}

	if (m_stream)
	{
	}
	else
	{
		assert(m_player && m_player->play);
		(*m_player->play)->SetPlayState(m_player->play, SL_PLAYSTATE_PLAYING);
	}
	m_paused = false;
}

void Source::RewindImpl()
{
	if (m_active)
	{
		if (m_stream)
		{
			bool paused = m_paused;
			m_in_buf.DecoderRewind();
			StopImpl();
			PlayImpl();
			if (paused) {
				PauseImpl();
			}
		}
		else
		{
			assert(m_player && m_player->play);
			(*m_player->play)->SetPlayState(m_player->play, SL_PLAYSTATE_STOPPED);
			(*m_player->play)->SetPlayState(m_player->play, SL_PLAYSTATE_PLAYING);
		}
	}
	else
	{
		if (m_stream)
		{
			m_in_buf.DecoderRewind();
		}
	}
}

bool Source::IsStopped() const
{
	return !m_active;
}

bool Source::IsPaused() const
{
	return m_paused;
}

bool Source::IsFinished() const
{
	if (m_stream) {
		return IsStopped() && !IsLooping() && m_in_buf.IsDecoderFinished();
	} else {
		return IsStopped();
	}
}

void Source::Stream()
{
	m_in_buf.Output(m_out_buf, IsLooping());
}

/************************************************************************/
/* class Source::Buffer                                                 */
/************************************************************************/

Source::Buffer::
Buffer(Decoder* decoder)
	: m_decoder(decoder)
	, m_used(0)
{
	if (m_decoder) {
		m_decoder->AddReference();
	}
}

Source::Buffer::
~Buffer()
{
	if (m_decoder) {
		m_decoder->RemoveReference();
	}
}

void Source::Buffer::
Output(AudioQueue* out, bool looping)
{
	if (m_size == 0 || m_used == m_size) {
		Reload(looping);
	}
	if (m_size == 0) {
		return;
	}

	while (true)
	{
		int left = m_size - m_used;
		int sz = out->Push(&m_decoder->GetBuffer()[m_used], left);
		assert(sz <= left);
		if (sz < left) {
			m_used += sz;
			break;
		} else if (sz == left) {
			Reload(looping);
			if (m_size == 0) {
				break;
			}
		}
	}
}

bool Source::Buffer::
IsDecoderFinished() const
{
	return m_decoder ? m_decoder->IsFinished() : true;
}

void Source::Buffer::
DecoderRewind()
{
	if (m_decoder) {
		m_decoder->Rewind();
	}
}

void Source::Buffer::
Reload(bool looping)
{
	m_size = m_decoder->Decode();
	assert(m_size <= m_decoder->GetBufferSize());	
	m_used = 0;
	if (m_decoder->IsFinished() && looping) {
		m_decoder->Rewind();
	}
}

}
}