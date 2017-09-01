#include "uniaudio/opensl/Source.h"
#include "uniaudio/opensl/AudioPool.h"
#include "uniaudio/opensl/AudioContext.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/OutputBuffer.h"
#include "uniaudio/InputBuffer.h"

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
	, m_offset(0)
	, m_stream(false)
	, m_ibuf(NULL)
	, m_obuf(NULL)
	, m_filepath(filepath)
	, m_player(NULL)
{
}

Source::Source(AudioPool* pool, Decoder* decoder)
	: m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_offset(0)
	, m_stream(true)
	, m_ibuf(NULL)
	, m_player(NULL)
{
	m_ibuf = new InputBuffer(decoder);

	const int HZ = decoder->GetSampleRate();
	const int depth = decoder->GetBitDepth();
	const int channels = decoder->GetChannels();
	const int samples = static_cast<int>(HZ * AudioContext::BUFFER_TIME_LEN);
	int buf_sz = depth * channels * samples / 8;
	m_obuf = new OutputBuffer(OUTPUT_BUF_COUNT, buf_sz);
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(this);
	}
	if (m_ibuf) {
		delete m_ibuf;
	}
	if (m_obuf) {
		delete m_obuf;
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
	if (!m_active) {
		m_offset = 0;
	}
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

void Source::Seek(float offset)
{
	m_pool->Seek(this, offset);
}

float Source::Tell()
{
	return m_pool->Tell(this);
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

	m_offset = 0;

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
			assert(m_ibuf);
			m_ibuf->DecoderRewind();
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
			assert(m_ibuf);
			m_ibuf->DecoderRewind();
		}
	}
}

void Source::SeekImpl(float offset)
{
	if (!m_active) {
		return;
	}

	if (m_stream)
	{
		m_offset = offset;

		bool looping = IsLooping();
		m_ibuf->Seek(offset, looping);
		m_ibuf->Output(m_obuf, looping);

		bool paused = m_paused;
		StopImpl();
		PlayImpl();
		if (paused) {
			PauseImpl();
		}
	}
	else
	{
		// todo
	}
}

float Source::TellImpl()
{
	return m_active ? m_offset : 0;
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
		assert(m_ibuf);
		return IsStopped() && !IsLooping() && m_ibuf->IsDecoderFinished();
	} else {
		return IsStopped();
	}
}

void Source::Stream()
{
	assert(m_ibuf && m_obuf);
	m_ibuf->Output(m_obuf, IsLooping());
}

}
}