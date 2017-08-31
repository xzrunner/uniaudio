#ifndef UA_NO_MPG123

#ifndef _UNIAUDIO_MPG123_DECODER_H_
#define _UNIAUDIO_MPG123_DECODER_H_

#include "uniaudio/Decoder.h"

#include <mpg123.h>

#include <string>

struct fs_file;

namespace ua
{

class Mpg123Decoder : public Decoder
{
public:
	Mpg123Decoder(const std::string& filepath, int buf_sz);
	virtual ~Mpg123Decoder();

	virtual int Decode();

	virtual bool Seek(float s);
	virtual bool Rewind();

	virtual int GetChannels() const;
	virtual int GetBitDepth() const;

	static bool Accepts(const std::string& ext);

	static void Quit();

private:
	void InitHandle();
	void InitMpg123();

private:
	fs_file* m_file;

	mpg123_handle* m_handle;

	int m_channels;

	static bool m_inited;

}; // Mpg123Decoder

}

#endif // _UNIAUDIO_MPG123_DECODER_H_

#endif // UA_NO_MPG123