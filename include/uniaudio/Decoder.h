#ifndef _UNIAUDIO_DECODER_H_
#define _UNIAUDIO_DECODER_H_

#include <cu/uncopyable.h>

#include <stddef.h>

namespace ua
{

class Decoder : private cu::Uncopyable
{
public:
	/**
	 * Indicates how many bytes of raw data should be generated at each
	 * call to Decode.
	 **/
	static const int DEFAULT_BUFFER_SIZE = 16384;

	/**
	 * Indicates the quality of the sound.
	 **/
	static const int DEFAULT_SAMPLE_RATE = 44100;

	/**
	 * Default is stereo.
	 **/
	static const int DEFAULT_CHANNELS = 2;

	/**
	 * 16 bit audio is the default.
	 **/
	static const int DEFAULT_BIT_DEPTH = 16;

	Decoder(int buf_sz = DEFAULT_BUFFER_SIZE);
	Decoder(const Decoder&);
	virtual ~Decoder();

	virtual Decoder* Clone() = 0;

	virtual int Decode() = 0;

	virtual bool Seek(float s) = 0;
	virtual bool Rewind() = 0;

	virtual int GetChannels() const = 0;
	virtual int GetBitDepth() const = 0;

	// in second
	virtual float GetDuration() const  = 0;

	const unsigned char* GetBuffer() const { return m_buf; }
	int GetBufferSize() const { return m_buf_size; }

	int GetSampleRate() const { return m_sample_rate; }

	bool IsFinished() const { return m_eof; }

protected:
	unsigned char* m_buf;
	int m_buf_size;

	int m_offset;
	
	bool m_eof;

	int m_sample_rate;

	int m_length;

}; // Decoder

}

#endif // _UNIAUDIO_DECODER_H_