#ifndef UA_NO_MPG123

#include "uniaudio/Mpg123Decoder.h"
#include "uniaudio/Exception.h"

#include <fs_file.h>

#include <cstdio>

namespace ua
{

bool Mpg123Decoder::m_inited = false;

Mpg123Decoder::Mpg123Decoder(const std::string& filepath, int buf_sz)
	: Decoder(buf_sz)
	, m_file(NULL)
	, m_handle(NULL)
	, m_channels(MPG123_STEREO)
{
	m_file = fs_open(filepath.c_str(), "rb");
	if (m_file) {
		InitMpg123();
		InitHandle();
	}
}

Mpg123Decoder::~Mpg123Decoder()
{
	if (m_file) {
		fs_close(m_file);
	}
	if (m_handle) {
		mpg123_delete(m_handle);
	}
}

int Mpg123Decoder::Decode()
{
	int size = 0;

	while (size < m_buf_size && !m_eof)
	{
		size_t numbytes = 0;
		int ret = mpg123_read(m_handle, m_buf + size, m_buf_size - size, &numbytes);
		switch (ret)
		{
		case MPG123_NEED_MORE:
		case MPG123_NEW_FORMAT:
		case MPG123_OK:
			size += numbytes;
			continue;
		case MPG123_DONE:
			size += numbytes;
			m_eof = true;
		default:
			return size;
		}
	}

	return size; 
}

bool Mpg123Decoder::Seek(float s)
{
	off_t offset = (off_t) (s * static_cast<double>(m_sample_rate));

	if (offset < 0) {
		return false;
	}

	if (mpg123_seek(m_handle, offset, SEEK_SET) >= 0) {
		m_eof = false;
		return true;
	} else {
		return false;
	}
}

bool Mpg123Decoder::Rewind()
{
	m_eof = false;
	if (mpg123_seek(m_handle, 0, SEEK_SET) >= 0) {
		return true;
	} else {
		return false;
	}
}

int Mpg123Decoder::GetChannels() const
{
	return m_channels;
}

int Mpg123Decoder::GetBitDepth() const
{
	return 16;
}

bool Mpg123Decoder::Accepts(const std::string& ext)
{
	return ext == "mp3";
}

void Mpg123Decoder::Quit()
{
	if (m_inited) {
		mpg123_exit();
		m_inited = false;
	}
}

// callbacks
namespace
{
static ssize_t read_callback(void* udata, void* buffer, size_t count)
{
	if (!udata) {
		return 0;
	}

	fs_file* file = static_cast<fs_file*>(udata);

	// Calculates how much data is still left and takes that value or
	// the buffer size, whichever is lower, as the number of bytes to write.
	size_t count_left = fs_size(file) - fs_ftell(file);
	size_t count_write = count_left < count ? count_left : count;
	if (count_write > 0) {
		fs_read(file, buffer, count_write);
	}

	// Returns the number of written bytes. 0 means EOF.
	return count_write;
}
} // callbacks

static off_t seek_callback(void* udata, off_t offset, int whence)
{
	if (!udata) {
		return -1;
	}

	fs_file* file = static_cast<fs_file*>(udata);

	switch (whence)
	{
	case SEEK_SET:
		// Negative values are invalid at this point.
		if (offset < 0)
			return -1;

		// Prevents the offset from going over EOF.
		if (fs_size(file) > (size_t) offset) {
			fs_seek_from_head(file, offset);
		} else {
			fs_seek_from_end(file, 0);
		}
		break;
	case SEEK_END:
		// Offset is set to EOF. Offset calculation is just like SEEK_CUR.
		fs_seek_from_end(file, 0);
	case SEEK_CUR:
		// Prevents the offset from going over EOF or below 0.
		if (offset > 0)
		{
			if (fs_size(file) > fs_ftell(file) + (size_t) offset) {
				fs_seek_from_cur(file, offset);
			} else {
				fs_seek_from_end(file, 0);
			}
		}
		else if (offset < 0)
		{
			if (fs_ftell(file) >= (size_t) (-offset)) {
				fs_seek_from_cur(file, offset);
			} else {
				fs_seek_from_head(file, 0);
			}
		}
		break;
	default:
		return -1;
	};

	return fs_ftell(file);
}

static void cleanup_callback(void *)
{
	// Cleanup is done by the Decoder class.
}

void Mpg123Decoder::InitHandle()
{
	m_handle = mpg123_new(NULL, NULL);
	if (!m_handle) {
		return;
	}

	// Suppressing all mpg123 messages.
	mpg123_param(m_handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0);	

	int ret = mpg123_replace_reader_handle(m_handle, &read_callback, &seek_callback, &cleanup_callback);
	if (ret != MPG123_OK) {
		mpg123_delete(m_handle);
		m_handle = NULL;
		return;
	}

	ret = mpg123_open_handle(m_handle, m_file);
	if (ret != MPG123_OK) {
		mpg123_delete(m_handle);
		m_handle = NULL;
		return;
	}

	long rate = 0;
	ret = mpg123_getformat(m_handle, &rate, &m_channels, NULL);
	if (ret == MPG123_ERR) {
		mpg123_delete(m_handle);
		m_handle = NULL;
		return;
	}

	if (m_channels == 0) {
		m_channels = 2;
	}

	// Force signed 16-bit output.
	mpg123_param(m_handle, MPG123_FLAGS, (m_channels == 2 ? MPG123_FORCE_STEREO : MPG123_MONO_MIX), 0);
	mpg123_format_none(m_handle);
	mpg123_format(m_handle, rate, m_channels, MPG123_ENC_SIGNED_16);

	m_sample_rate = rate;
}

void Mpg123Decoder::InitMpg123()
{
	if (m_inited) {
		return;
	}

	int ret = mpg123_init();
	if (ret != MPG123_OK) {
		throw Exception("Could not init mpg123.");
	}
	m_inited = true;
}

}

#endif // UA_NO_MPG123