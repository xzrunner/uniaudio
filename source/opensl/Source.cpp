#include "uniaudio/opensl/Source.h"
#include "uniaudio/opensl/AudioPool.h"
#include "uniaudio/opensl/AudioContext.h"
#include "uniaudio/Decoder.h"

#include <assert.h>

namespace ua
{
namespace opensl
{

Source::Source(AudioPool* pool, const AudioData* data)
	: m_pool(pool)
	, m_decoder(NULL)
	, m_stream(false)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_bufs(NULL)
	, m_buf(NULL)
	, m_buf_sz(0)
	, m_buf_used(0)
{
	// todo
}

Source::Source(AudioPool* pool, Decoder* decoder)
	: m_pool(pool)
	, m_decoder(decoder)
	, m_stream(true)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_buf(NULL)
	, m_buf_sz(0)
	, m_buf_used(0)
{
	m_decoder->AddReference();

	const int HZ = decoder->GetSampleRate();
	const int depth = decoder->GetBitDepth();
	const int channels = decoder->GetChannels();
	const int samples = static_cast<int>(HZ * AudioContext::BUFFER_TIME_LEN);
	int buf_sz = depth * channels * samples / 8;
	m_bufs = new AudioQueue(QUEUE_BUF_COUNT, buf_sz);

	m_buf = new unsigned char[m_decoder->GetBufferSize()];
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(this);
	}
	if (m_buf) {
		delete[] m_buf;
	}
	if (m_bufs) {
		delete m_bufs;
	}
	if (m_decoder) {
		m_decoder->RemoveReference();
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
		// 
	}
	else
	{
		// todo
	}

	m_active = true;
}

void Source::StopImpl()
{
	if (m_active)
	{
		if (m_stream)
		{
			//
		}
		else
		{
			// todo
		}
	}
	m_active = false;
}

void Source::PauseImpl()
{
	if (m_active) {
		m_paused = true;
	}
}

void Source::ResumeImpl()
{
	if (m_active && m_paused) {
		m_paused = false;
	}
}

void Source::RewindImpl()
{
	if (m_active)
	{
		if (m_stream)
		{
			bool paused = m_paused;
			m_decoder->Rewind();
			StopImpl();
			PlayImpl();
			if (paused) {
				PauseImpl();
			}
		}
		else
		{

		}
	}
	else
	{
		if (m_stream)
		{
			m_decoder->Rewind();
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
		return IsStopped() && !IsLooping() && m_decoder->IsFinished();
	} else {
		return IsStopped();
	}
}

void Source::ReloadBuffer()
{
	m_buf_sz = m_decoder->Decode();
	assert(m_buf_sz <= m_decoder->GetBufferSize());	
	m_buf_used = 0;
	memcpy(m_buf, m_decoder->GetBuffer(), m_buf_sz);
	if (m_decoder->IsFinished() && IsLooping()) {
		m_decoder->Rewind();
	}
}

void Source::Stream()
{
	if (m_buf_sz == 0 || m_buf_used == m_buf_sz) {
		ReloadBuffer();
	}
	if (m_buf_sz == 0) {
		return;
	}

	while (true)
	{
		int left = m_buf_sz - m_buf_used;
		int sz = m_bufs->Filling(&m_buf[m_buf_used], left);
		assert(sz <= left);
		if (sz < left) {
			m_buf_used += sz;
			break;
		} else if (sz == left) {
			ReloadBuffer();
			if (m_buf_sz == 0) {
				break;
			}
		}
	}
}

}
}