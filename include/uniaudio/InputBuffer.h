#ifndef _UNIAUDIO_INPUT_BUFFER_H_
#define _UNIAUDIO_INPUT_BUFFER_H_

#include <memory>

namespace ua
{

class Decoder;
class OutputBuffer;

class InputBuffer
{
public:
	InputBuffer(std::unique_ptr<Decoder>& decoder);

	void Output(OutputBuffer* out, bool looping);

	bool IsDecoderFinished() const;
	void DecoderRewind();

	const std::unique_ptr<Decoder>& GetDecoder() const { return m_decoder; }
	std::unique_ptr<Decoder>& GetDecoder() { return m_decoder; }

	void Seek(float offset, bool looping);

	// return second
	float GetOffset() const;

	void Rewind();
	
private:
	void Reload(bool looping);

private:
	std::unique_ptr<Decoder> m_decoder;

	int m_size, m_used;

	float m_offset;

}; // InputBuffer

}

#endif // _UNIAUDIO_INPUT_BUFFER_H_