#ifndef _UNIAUDIO_DECODER_FACTORY_H_
#define _UNIAUDIO_DECODER_FACTORY_H_

#include "uniaudio/Decoder.h"

#include <cu/cu_stl.h>

#include <memory>

namespace ua
{

class DecoderFactory
{
public:
	static std::unique_ptr<Decoder> Create(const CU_STR& filepath,
		int buf_sz = Decoder::DEFAULT_BUFFER_SIZE);

}; // DecoderFactory

}

#endif // _UNIAUDIO_DECODER_FACTORY_H_