#ifdef UA_SUPPORT_COREAUDIO

#include <cu/cu_stl.h>

#include "uniaudio/CoreAudioDecoder.h"
#include "uniaudio/Exception.h"

namespace ua
{

// callbacks
namespace
{
OSStatus read_func(void* in_client_data, SInt64 in_position, UInt32 request_count, void* buffer, UInt32* actual_count)
{
	CoreAudioDecoder::Data* data = (CoreAudioDecoder::Data*)in_client_data;
	SInt64 bytes_left = data->size - in_position;

	if (bytes_left > 0)
	{
		UInt32 actual_size = bytes_left >= request_count ? request_count : (UInt32) bytes_left;
		fs_seek_from_head(data->file, in_position);
		fs_read(data->file, buffer, actual_size);
		*actual_count = actual_size;
	}
	else
	{
		*actual_count = 0;
		return kAudioFilePositionError;
	}

	return noErr;
}

SInt64 get_size_func(void* in_client_data)
{
	CoreAudioDecoder::Data* data = (CoreAudioDecoder::Data*)in_client_data;
	return data->size;
}
} // callbacks

CoreAudioDecoder::CoreAudioDecoder(const std::string& filepath, int buf_sz)
	: Decoder(buf_sz)
	, m_filepath(filepath)
	, m_audio_file(nullptr)
	, m_ext_audio_file(nullptr)
	, m_duration(-2)
{
	Init();
}

CoreAudioDecoder::CoreAudioDecoder(const CoreAudioDecoder& src)
	: Decoder(src)
	, m_filepath(src.m_filepath)
	, m_audio_file(nullptr)
	, m_ext_audio_file(nullptr)	
	, m_duration(-2)	
{
	Init();
}

CoreAudioDecoder::~CoreAudioDecoder()
{
	CloseAudioFile();
}

Decoder* CoreAudioDecoder::Clone()
{
	return new CoreAudioDecoder(*this);
}

int CoreAudioDecoder::Decode()
{
	int size = 0;

	while (size < m_buf_size)
	{
		AudioBufferList data_buffer;
		data_buffer.mNumberBuffers = 1;
		data_buffer.mBuffers[0].mDataByteSize = m_buf_size - size;
		data_buffer.mBuffers[0].mData = (char *) m_buf + size;
		data_buffer.mBuffers[0].mNumberChannels = m_output_info.mChannelsPerFrame;

		UInt32 frames = (m_buf_size - size) / m_output_info.mBytesPerFrame;

		if (ExtAudioFileRead(m_ext_audio_file, &frames, &data_buffer) != noErr) {
			return size;
		}

		if (frames == 0)
		{
			m_eof = true;
			break;
		}

		size += frames * m_output_info.mBytesPerFrame;
	}

	return size;
}

bool CoreAudioDecoder::Seek(float s)
{
	OSStatus err = ExtAudioFileSeek(m_ext_audio_file, (SInt64) (s * m_input_info.mSampleRate));

	if (err == noErr)
	{
		m_eof = false;
		return true;
	}

	return false;
}

bool CoreAudioDecoder::Rewind()
{
	OSStatus err = ExtAudioFileSeek(m_ext_audio_file, 0);

	if (err == noErr)
	{
		m_eof = false;
		return true;
	}

	return false;
}

int CoreAudioDecoder::GetChannels() const
{
	return m_output_info.mChannelsPerFrame;
}

int CoreAudioDecoder::GetBitDepth() const
{
	return m_output_info.mBitsPerChannel;
}

float CoreAudioDecoder::GetDuration() const
{
	// Only calculate the duration if we haven't done so already.
	if (m_duration == -2.0)
	{
		SInt64 samples = 0;
		UInt32 psize = (UInt32) sizeof(samples);

		OSStatus err = ExtAudioFileGetProperty(m_ext_audio_file, kExtAudioFileProperty_FileLengthFrames, &psize, &samples);

		if (err == noErr)
			m_duration = (double) samples / (double) m_sample_rate;
		else
			m_duration = -1.0;
	}

	return m_duration;	
}

bool CoreAudioDecoder::Accepts(const CU_STR& ext)
{
	UInt32 size = 0;
	CU_VEC<UInt32> types;

	// Get the size in bytes of the type array we're about to get.
	OSStatus err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, sizeof(UInt32), nullptr, &size);
	if (err != noErr) {
		return false;
	}

	types.resize(size / sizeof(UInt32));

	// Get an array of supported types.
	err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ReadableTypes, 0, nullptr, &size, &types[0]);
	if (err != noErr) {
		return false;
	}

	// Turn the extension string into a CFStringRef.
	CFStringRef extstr = CFStringCreateWithCString(nullptr, ext.c_str(), kCFStringEncodingUTF8);

	CFArrayRef exts = nullptr;
	size = sizeof(CFArrayRef);

	for (int i = 0, n = types.size(); i < n; ++i)
	{
		UInt32 type = types[i];

		// Get the extension strings for the type.
		err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof(UInt32), &type, &size, &exts);
		if (err != noErr)
			continue;

		// A type can have more than one extension string.
		for (CFIndex i = 0; i < CFArrayGetCount(exts); i++)
		{
			CFStringRef value = (CFStringRef) CFArrayGetValueAtIndex(exts, i);

			if (CFStringCompare(extstr, value, 0) == kCFCompareEqualTo)
			{
				CFRelease(extstr);
				CFRelease(exts);
				return true;
			}
		}

		CFRelease(exts);
	}

	CFRelease(extstr);
	return false;
}

void CoreAudioDecoder::CloseAudioFile()
{
	if (m_ext_audio_file) {
		ExtAudioFileDispose(m_ext_audio_file);
		m_ext_audio_file = nullptr;
	}
	if (m_audio_file) {
		AudioFileClose(m_audio_file);
		m_audio_file = nullptr;
	}
}

void CoreAudioDecoder::Init()
{
	try 
	{
		m_source.file = fs_open(m_filepath.c_str(), "rb");
		if (!m_source.file) {
			throw Exception("Could not open file: %s", m_filepath.c_str());
		}
		m_source.size = fs_size(m_source.file);
		
		OSStatus err = noErr;

		// Open the file represented by the Data.
		err = AudioFileOpenWithCallbacks(&m_source, read_func, nullptr, get_size_func, nullptr, kAudioFileMP3Type, &m_audio_file);
		if (err != noErr) {
			throw Exception("Could not open audio file for decoding.");
		}

		// We want to use the Extended AudioFile API.
		err = ExtAudioFileWrapAudioFileID(m_audio_file, false, &m_ext_audio_file);
		if (err != noErr) {
			throw Exception("Could not get extended api for decoding.");
		}

		// Get the format of the audio data.
		UInt32 property_size = sizeof(m_input_info);
		err = ExtAudioFileGetProperty(m_ext_audio_file, kExtAudioFileProperty_FileDataFormat, &property_size, &m_input_info);
		if (err != noErr) {
			throw Exception("Could not determine file format.");
		}

		// Set the output format to 16 bit signed integer (native-endian) data.
		// Keep the channel count and sample rate of the source format.
		m_output_info.mSampleRate = m_input_info.mSampleRate;
		m_output_info.mChannelsPerFrame = m_input_info.mChannelsPerFrame;

		int bytes = (m_input_info.mBitsPerChannel == 8) ? 1 : 2;

		m_output_info.mFormatID = kAudioFormatLinearPCM;
		m_output_info.mBitsPerChannel = bytes * 8;
		m_output_info.mBytesPerFrame = bytes * m_output_info.mChannelsPerFrame;
		m_output_info.mFramesPerPacket = 1;
		m_output_info.mBytesPerPacket = bytes * m_output_info.mChannelsPerFrame;
		m_output_info.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;

		// unsigned 8-bit or signed 16-bit integer PCM data.
		if (m_output_info.mBitsPerChannel == 16) {
			m_output_info.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
		}

		// Set the desired output format.
		property_size = sizeof(m_output_info);
		err = ExtAudioFileSetProperty(m_ext_audio_file, kExtAudioFileProperty_ClientDataFormat, property_size, &m_output_info);
		if (err != noErr) {
			throw Exception("Could not set decoder properties.");
		}
	}
	catch (Exception& )
	{
		CloseAudioFile();
		throw;
	}

	m_sample_rate = static_cast<int>(m_output_info.mSampleRate);
}

}

#endif // UA_SUPPORT_COREAUDIO