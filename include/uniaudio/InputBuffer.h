#ifndef _UNIAUDIO_INPUT_BUFFER_H_
#define _UNIAUDIO_INPUT_BUFFER_H_

namespace ua
{

class Decoder;
class OutputBuffer;

class InputBuffer
{
public:
	InputBuffer(Decoder* decoder);
	~InputBuffer();

	void Output(OutputBuffer* out, bool looping);

	bool IsDecoderFinished() const;
	void DecoderRewind();

	const Decoder* GetDecoder() const { return m_decoder; }
	Decoder* GetDecoder() { return m_decoder; }

	void Seek(float offset, bool looping);

private:
	void Reload(bool looping);

private:
	Decoder* m_decoder;

	int m_size, m_used;

}; // InputBuffer

}

#endif // _UNIAUDIO_INPUT_BUFFER_H_