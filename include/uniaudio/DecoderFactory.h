#ifndef _UNIAUDIO_DECODER_FACTORY_H_
#define _UNIAUDIO_DECODER_FACTORY_H_

#include <string>

namespace ua
{

class Decoder;
class DecoderFactory
{
public:
	static Decoder* Create(const std::string& filepath);

}; // DecoderFactory

}

#endif // _UNIAUDIO_DECODER_FACTORY_H_