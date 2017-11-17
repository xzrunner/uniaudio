#include "uniaudio/DecoderFactory.h"
#ifndef UA_NO_MPG123
#include "uniaudio/Mpg123Decoder.h"
#endif // UA_NO_MPG123
#ifdef UA_SUPPORT_COREAUDIO
#include "uniaudio/CoreAudioDecoder.h"
#endif // UA_SUPPORT_COREAUDIO

#include <algorithm>
#include <memory>

namespace ua
{

std::unique_ptr<Decoder> DecoderFactory::Create(const CU_STR& filepath, int buf_sz)
{
	std::unique_ptr<Decoder> decoder;

	CU_STR ext = filepath.substr(filepath.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

	if (false)
		;
#ifndef UA_NO_MPG123
	else if (Mpg123Decoder::Accepts(ext))
		decoder = std::make_unique<Mpg123Decoder>(filepath.c_str(), buf_sz);
#endif // UA_NO_MPG123
#ifdef UA_SUPPORT_COREAUDIO
	else if (CoreAudioDecoder::Accepts(ext))
		decoder = std::make_unique<CoreAudioDecoder>(filepath.c_str(), buf_sz);
#endif // UA_SUPPORT_COREAUDIO

	return decoder;
}

}