#include "uniaudio/DecoderFactory.h"
#include "uniaudio/Mpg123Decoder.h"

#include <algorithm>

namespace ua
{

Decoder* DecoderFactory::Create(const std::string& filepath)
{
	Decoder* decoder = NULL;

	std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
	if (ext == "mp3") {
		decoder = new Mpg123Decoder(filepath);
	}

	return decoder;
}

}