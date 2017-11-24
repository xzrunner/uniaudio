#include "uniaudio/openal/Source.h"
#include "uniaudio/openal/AudioPool.h"
#include "uniaudio/openal/AudioContext.h"
#include "uniaudio/AudioData.h"
#include "uniaudio/Decoder.h"
#include "uniaudio/OutputBuffer.h"
#include "uniaudio/InputBuffer.h"
#include "uniaudio/Exception.h"

#include <assert.h>

#define FORCE_REPLAY

namespace ua
{
namespace openal
{

Source::Source(AudioPool* pool, const AudioData* data)
	: m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_freq(data->GetSampleRate())
	, m_curr_offset(0)
	, m_stream(false)
	, m_mix(false)
	, m_ibuf(nullptr)
	, m_obuf(nullptr)
	, m_player(0)
{
	memset(m_buffers, 0, sizeof(m_buffers));

	alGetError();

	try {
		ALenum err;

		alGenBuffers(1, m_buffers);
		if ((err = alGetError()) != AL_NO_ERROR)  {
			throw Exception("Gen openal buffers error: %x\n", err);
		}

		ALenum fmt = GetFormat(data->GetChannels(), data->GetBitDepth());
		if (fmt == 0) {
			throw Exception("Source error fmt: %d\n", fmt);
		}

		alBufferData(m_buffers[0], fmt, data->GetData(), data->GetSize(), data->GetSampleRate());
		if ((err = alGetError()) != AL_NO_ERROR)  {
			throw Exception("Commit buffer data error: %x\n", err);
		}
	} catch (Exception&) {
		alDeleteBuffers(1, m_buffers);
		throw;		
	}
}

Source::Source(AudioPool* pool, std::unique_ptr<Decoder>& decoder, bool mix)
	: ua::Source(*decoder)
	, m_pool(pool)
	, m_looping(false)
	, m_active(false)
	, m_paused(false)
	, m_freq(decoder->GetSampleRate())
	, m_curr_offset(0)
	, m_stream(true)
	, m_mix(mix)
	, m_ibuf(nullptr)
	, m_obuf(nullptr)
	, m_player(0)
{
	m_ibuf = new InputBuffer(decoder);
	if (!m_ibuf) {
		throw Exception("Could not create InputBuffer.");
	}

	if (m_mix)
	{
		auto& dc = m_ibuf->GetDecoder();
		const int HZ = dc->GetSampleRate();
		const int depth = dc->GetBitDepth();
		const int channels = dc->GetChannels();
		const int samples = static_cast<int>(HZ * AudioContext::BUFFER_TIME_LEN);
		int buf_sz = depth * channels * samples / 8;
		m_obuf = new OutputBuffer(OUTPUT_BUF_COUNT, buf_sz);
		if (!m_obuf) {
			throw Exception("Could not create OutputBuffer.");
		}
	}
	else
	{
		alGetError();
		alGenBuffers(MAX_BUFFERS, m_buffers);
		if (ALenum err = alGetError() != AL_NO_ERROR)  {
			throw Exception("Gen openal buffers error: %x\n", err);
		}
	}
}

Source::Source(const Source& src)
	: ua::Source(src)
	, m_pool(src.m_pool)
	, m_looping(src.m_looping)
	, m_active(src.m_active)
	, m_paused(src.m_paused)
	, m_freq(src.m_freq)
	, m_curr_offset(src.m_curr_offset)
	, m_stream(src.m_stream)
	, m_mix(src.m_mix)
	, m_ibuf(nullptr)
	, m_obuf(nullptr)
	, m_player(src.m_player)
{
	memset(m_buffers, 0, sizeof(m_buffers));

	if (src.m_ibuf)
	{
		auto decoder = std::unique_ptr<Decoder>(src.m_ibuf->GetDecoder()->Clone());
		m_ibuf = new InputBuffer(decoder);
		if (!m_ibuf) {
			throw Exception("Could not create InputBuffer.");
		}

		if (m_mix)
		{
			auto& dc = m_ibuf->GetDecoder();
			const int HZ = dc->GetSampleRate();
			const int depth = dc->GetBitDepth();
			const int channels = dc->GetChannels();
			const int samples = static_cast<int>(HZ * AudioContext::BUFFER_TIME_LEN);
			int buf_sz = depth * channels * samples / 8;
			m_obuf = new OutputBuffer(OUTPUT_BUF_COUNT, buf_sz);
			if (!m_obuf) {
				throw Exception("Could not create OutputBuffer.");
			}
		}
		else
		{
			alGetError();
			alGenBuffers(MAX_BUFFERS, m_buffers);
			if (ALenum err = alGetError() != AL_NO_ERROR) {
				throw Exception("Gen openal buffers error: %x\n", err);
			}
		}
	}
	else
	{

	}
}

Source::~Source()
{
	if (m_active) {
		m_pool->Stop(shared_from_this());
	}
	if (m_stream) {
		if (!m_mix) {
			alDeleteBuffers(MAX_BUFFERS, m_buffers);
		}
	}
	else {
		assert(!m_mix);
		alDeleteBuffers(1, m_buffers);
	}
	if (m_ibuf) {
		delete m_ibuf;
	}
	if (m_obuf) {
		delete m_obuf;
	}
}

std::shared_ptr<ua::Source> Source::Clone()
{
	return std::make_shared<Source>(*this);
}

bool Source::Update()
{
	if (!m_active) {
		return false;
	}

	if (!m_stream) {
		assert(!m_mix);
		alSourcei(m_player, AL_LOOPING, IsLooping() ? AL_TRUE : AL_FALSE);
		return !IsStopped();
	} else if (!IsLooping() && IsFinished()) {
		return false;
	} else if (m_duration != 0 && GetCurrOffset() > m_offset + m_duration) {
		StopImpl();
		return false;
	}

	// fade
	UpdateCurrVolume();

	if (m_mix)
	{
		assert(m_ibuf && m_obuf);
		m_ibuf->Output(m_obuf, IsLooping());
	}
	else
	{
		// set volume
		alSourcef(m_player, AL_GAIN, m_curr_volume);

		ALint processed = 0;
		alGetSourcei(m_player, AL_BUFFERS_PROCESSED, &processed);
		while (processed--)
		{
			float old_offset_seconds = GetCurrOffset(m_freq);

			ALuint buffer;
			alSourceUnqueueBuffers(m_player, 1, &buffer);

			float new_offset_seconds = GetCurrOffset(m_freq);
			m_curr_offset += old_offset_seconds - new_offset_seconds;

			if (Stream(buffer) > 0) {
				alSourceQueueBuffers(m_player, 1, &buffer);
			}
		}
	}

 	return true;
}

void Source::Play()
{
	if (m_active) 
	{
		if (m_paused) {
			m_pool->Resume(shared_from_this());
		}
#ifdef FORCE_REPLAY
		else {
			m_pool->Rewind(shared_from_this());
		}
#endif // FORCE_REPLAY
		return;
	}

	m_active = m_pool->Play(shared_from_this());
}

void Source::Stop()
{
	if (!IsStopped()) {
		m_pool->Stop(shared_from_this());
	}
}

void Source::Pause()
{
	m_pool->Pause(shared_from_this());
}

void Source::Resume()
{
	m_pool->Resume(shared_from_this());
}

void Source::Rewind()
{
	m_pool->Rewind(shared_from_this());
}

void Source::Seek(float offset)
{
	m_pool->Seek(shared_from_this(), offset);
}

float Source::Tell()
{
	return m_pool->Tell(shared_from_this());
}

void Source::PlayImpl()
{
	// init offset
	m_curr_offset = m_offset;
	if (m_offset != 0)
	{
		if (m_stream) {
			bool looping = IsLooping();
			m_ibuf->Seek(m_offset, looping);
			if (m_mix) {
				m_ibuf->Output(m_obuf, looping);
			}
		} else {
			alSourcef(m_player, AL_SEC_OFFSET, m_offset);
		}
	}

	if (m_stream)
	{
		if (!m_mix)
		{
			int used = 0;
			for (int i = 0; i < MAX_BUFFERS; ++i)
			{
				if (Stream(m_buffers[i]) == 0) {
					break;
				}
				++used;
				assert(m_ibuf);
				if (m_ibuf->GetDecoder()->IsFinished()) {
					break;
				}
			}
			if (used > 0) {
				alSourceQueueBuffers(m_player, used, m_buffers);
			}
		}
	}
	else
	{
		assert(!m_mix);
		alSourcei(m_player, AL_BUFFER, m_buffers[0]);
		if (alGetError() != AL_NO_ERROR)  {
			throw Exception("Source::PlayImpl bind buffer error.");
		}
	}

	if (!m_mix)
	{
		alSourcePlay(m_player);
		if (alGetError() != AL_NO_ERROR)  {
			throw Exception("Source::PlayImpl alSourcePlay error.");
		}
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
		if (!m_mix)
		{
			alSourceStop(m_player);
			int queued = 0;
			alGetSourcei(m_player, AL_BUFFERS_QUEUED, &queued);
			while (queued--)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(m_player, 1, &buffer);
			}
		}
	}
	else
	{
		assert(!m_mix);
		alSourceStop(m_player);
	}
	if (!m_mix) {
		alSourcei(m_player, AL_BUFFER, AL_NONE);
	}

	m_active = false;
}

void Source::PauseImpl()
{
 	if (m_active) {
		if (!m_mix) {
			alSourcePause(m_player);
		}
 		m_paused = true;
 	}
}

void Source::ResumeImpl()
{
 	if (m_active && m_paused) {
		if (!m_mix) {
 			alSourcePlay(m_player);
		}
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
			assert(m_ibuf);
			m_ibuf->Rewind();
			StopImpl();
			PlayImpl();
			if (paused) {
				PauseImpl();
			}
		}
		else
		{
			assert(!m_mix);
		 	alSourceRewind(m_player);
		 	if (!m_paused) {
		 		alSourcePlay(m_player);
		 	}
		}
	}
	else
	{
		if (m_stream)
		{
			assert(m_ibuf);
			m_ibuf->Rewind();
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
		bool looping = IsLooping();
		m_ibuf->Seek(offset, looping);
		if (m_mix) {
			m_ibuf->Output(m_obuf, looping);
		}

		bool paused = m_paused;
		StopImpl();
		PlayImpl();
		m_curr_offset = offset;
		if (paused) {
			PauseImpl();
		}
	} 
	else 
	{
		alSourcef(m_player, AL_SEC_OFFSET, offset);
	}
}

float Source::TellImpl()
{
	if (!m_active) {
		return 0;
	}

	float offset;
	alGetSourcef(m_player, AL_SAMPLE_OFFSET, &offset);
	offset /= m_freq;
	if (m_stream) {
		offset += m_curr_offset;
	}
	return offset;
}

void Source::SetLooping(bool looping)
{
	if (m_active && !m_stream) {
		assert(!m_mix);
		alSourcei(m_player, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	}
	m_looping = looping;
}

void Source::SetPlayer(ALuint player) 
{ 
	assert(!m_mix); 
	m_player = player;
}

bool Source::IsStopped() const
{
	// todo
	if (m_mix) {
		return false;
	}

	if (m_active) {
		ALenum state;
		alGetSourcei(m_player, AL_SOURCE_STATE, &state);
		return (state == AL_STOPPED);
	} else {
		return true;
	}
}

bool Source::IsPaused() const
{
	// todo
	if (m_mix) {
		return false;
	}

	if (m_active) {
		ALenum state;
		alGetSourcei(m_player, AL_SOURCE_STATE, &state);
		return (state == AL_PAUSED);
	} else {
		return false;
	}
}

bool Source::IsFinished() const
{
	if (m_stream) {
		assert(m_ibuf);
		return IsStopped() && !IsLooping() && m_ibuf->GetDecoder()->IsFinished();
	} else {
		return IsStopped();
	}
}

ALenum Source::GetFormat(int channels, int bit_depth)
{
	if (channels == 1 && bit_depth == 8) {
		return AL_FORMAT_MONO8;
	} else if (channels == 1 && bit_depth == 16) {
		return AL_FORMAT_MONO16;
	} else if (channels == 2 && bit_depth == 8) {
		return AL_FORMAT_STEREO8;
	} else if (channels == 2 && bit_depth == 16) {
		return AL_FORMAT_STEREO16;
	}
	return 0;
}

int Source::Stream(ALuint buffer)
{
	assert(m_ibuf && !m_mix);
	const std::unique_ptr<Decoder>& d = m_ibuf->GetDecoder();
	int decoded = std::max(d->Decode(), 0);
	if (decoded > 0)
	{
		int fmt = GetFormat(d->GetChannels(), d->GetBitDepth());
		if (fmt != 0) {
			alBufferData(buffer, fmt, d->GetBuffer(), decoded, d->GetSampleRate());
		} else {
			decoded = 0;			
		}
	}

	if (d->IsFinished() && IsLooping())
	{
		d->Rewind();
	}

	return decoded;
}

//int Source::GetFreq() const
//{
//	int freq;
//	ALint b;
//	alGetSourcei(m_player, AL_BUFFER, &b);
//	alGetBufferi(b, AL_FREQUENCY, &freq);
//
//	assert(freq == m_freq);
//	
//	return freq;
//}

float Source::GetCurrOffset(int freq)
{
	assert(freq != 0);
	float offset_samples;
	alGetSourcef(m_player, AL_SAMPLE_OFFSET, &offset_samples);
	float offset_seconds = offset_samples / freq;
	return offset_seconds;
}

void Source::UpdateCurrVolume()
{
	m_curr_volume = m_ori_volume;

	float offset = GetCurrOffset();
	if (m_fade_in > 0 && offset < m_fade_in) {
		m_curr_volume = m_ori_volume * offset / m_fade_in;
	} else if (m_fade_out > 0 && m_duration + m_offset - offset < m_fade_out) {
		m_curr_volume = m_ori_volume * (m_duration + m_offset - offset) / m_fade_out;
	}
}

float Source::GetCurrOffset() const
{
	return m_mix ? m_ibuf->GetOffset() : m_curr_offset;
}

}
}