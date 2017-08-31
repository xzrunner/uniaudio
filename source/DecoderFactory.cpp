#include "uniaudio/DecoderFactory.h"
#ifndef UA_NO_MPG123
#include "uniaudio/Mpg123Decoder.h"
#endif // UA_NO_MPG123
#ifdef UA_SUPPORT_COREAUDIO
#include "uniaudio/CoreAudioDecoder.h"
#endif // UA_SUPPORT_COREAUDIO

#include <algorithm>

namespace ua
{

Decoder* DecoderFactory::Create(const std::string& filepath, int buf_sz)
{
	Decoder* decoder = NULL;

	std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

	if (false)
		;
#ifndef UA_NO_MPG123
	else if (Mpg123Decoder::Accepts(ext))
		decoder = new Mpg123Decoder(filepath, buf_sz);
#endif // UA_NO_MPG123
#ifdef UA_SUPPORT_COREAUDIO
	else if (CoreAudioDecoder::Accepts(ext))
		decoder = new CoreAudioDecoder(filepath, buf_sz);
#endif // UA_SUPPORT_COREAUDIO

	return decoder;
}

}