#ifndef _UNIAUDIO_OPENSL_AUDIO_CONTEXT_H_
#define _UNIAUDIO_OPENSL_AUDIO_CONTEXT_H_

#include "uniaudio/AudioContext.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#ifdef __ANDROID__
// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#endif // __ANDROID__

namespace ua
{

class Source;

namespace opensl
{

class AudioPool;
class AudioContext : public ua::AudioContext
{
public:
	AudioContext();
	virtual ~AudioContext();

	virtual ua::Source* CreateSource(const AudioData* data);
	virtual ua::Source* CreateSource(Decoder* decoder);
	virtual ua::Source* CreateSource(const std::string& filepath, bool stream);

#ifdef __ANDROID__
	void InitAAssetMgr(JNIEnv* env, jobject assetManager);
	bool LoadAssetFile(const std::string& filepath, SLDataLocator_AndroidFD* loc_fd);
#endif // __ANDROID__

	SLEngineItf GetEngine() { return m_engine_engine; }
	SLObjectItf GetOutputMix() { return m_output_mix_obj; }

public:
	// 10ms length.
	static const float BUFFER_TIME_LEN;

private:
	bool CreateEngine();

private:
	// engine interfaces
	SLObjectItf m_engine_obj;
	SLEngineItf m_engine_engine;

	// output mix interfaces
	SLObjectItf              m_output_mix_obj;
	SLEnvironmentalReverbItf m_output_mix_env_reverb;

	// buffer queue player interfaces
	AudioPool* m_pool;

	// aux effect on the output mix, used by the buffer queue player
	static const SLEnvironmentalReverbSettings m_reverb_settings;

#ifdef __ANDROID__
 	AAssetManager* m_aasset_mgr;
#endif // __ANDROID__

}; // AudioContext

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_CONTEXT_H_