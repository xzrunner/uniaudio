#ifndef _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_
#define _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_

#include "uniaudio/AudioContext.h"

#include <OpenAL/alc.h>

#include <memory>

namespace ua
{
namespace openal
{

class AudioPool;
class AudioContext : public ua::AudioContext
{
public:
	AudioContext();
	AudioContext(ALCdevice* device, ALCcontext* context);
	virtual ~AudioContext();

	virtual std::shared_ptr<ua::Source> CreateSource(const AudioData* data) override final;
	virtual std::shared_ptr<ua::Source> CreateSource(std::unique_ptr<Decoder>& decoder) override final;
	virtual std::shared_ptr<ua::Source> CreateSource(const CU_STR& filepath, bool stream) override final;

	virtual void Stop() override final;
	virtual void Pause() override final;
	virtual void Resume() override final;
	virtual void Rewind() override final;

	virtual void SetVolume(float volume) override final;

public:
	// 10ms length.
	static const float BUFFER_TIME_LEN;

private:
	void Initialize();
	void Initialize(ALCdevice* device, ALCcontext* context);
	void Terminate();

private:
	bool m_own_ctx;

	ALCdevice* m_device;

	ALCcontext* m_context;

	AudioPool* m_pool;

}; // AudioContext

}
}

#endif // _UNIAUDIO_OPENAL_AUDIO_CONTEXT_H_