#ifndef _UNIAUDIO_DECODER_FACTORY_H_
#define _UNIAUDIO_DECODER_FACTORY_H_

#include "uniaudio/Decoder.h"

#include <string>
#include <memory>

namespace ua
{

class DecoderFactory
{
public:
	static std::unique_ptr<Decoder> Create(const std::string& filepath,
		int buf_sz = Decoder::DEFAULT_BUFFER_SIZE);

}; // DecoderFactory

}

#endif // _UNIAUDIO_DECODER_FACTORY_H_