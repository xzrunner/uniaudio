#include "uniaudio/opensl/AudioContext.h"
#include "uniaudio/opensl/AudioPool.h"
#include "uniaudio/opensl/Source.h"
#include "uniaudio/DecoderFactory.h"

#include <multitask/Thread.h>
#include <multitask/Task.h>
#include <multitask/ThreadPool.h>

#include <assert.h>
#include <stddef.h>

namespace ua
{
namespace opensl
{

const float AudioContext::BUFFER_TIME_LEN = 0.01f;

const SLEnvironmentalReverbSettings AudioContext::m_reverb_settings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

static void
thread_update_cb(void* arg)
{
// 	AudioPool* pool = static_cast<AudioPool*>(arg);
// 	pool->Update();	
}

static void
task_update_cb(void* arg)
{
// 	mt::Task* t = mt::TaskPool::Instance()->Fetch();
// 	assert(t);
// 	static_cast<mt::CommonTask*>(t)->SetUpdateCB(thread_update_cb, arg);
// 	mt::ThreadPool::Instance()->AddTask(t);
}

AudioContext::AudioContext()
	: m_engine_obj(NULL)
	, m_engine_engine(NULL)
	, m_output_mix_obj(NULL)
	, m_output_mix_env_reverb(NULL)
{
	CreateEngine();

	m_pool = new AudioPool(this);

//	mt::ThreadPool::Instance()->RegisterUpdateCB(task_update_cb, m_pool);
}

AudioContext::~AudioContext()
{
	if (m_engine_obj) {
		(*m_engine_obj)->Destroy(m_engine_obj);
	}
	if (m_output_mix_obj) {
		(*m_output_mix_obj)->Destroy(m_output_mix_obj);
	}

//	mt::ThreadPool::Instance()->UnregisterUpdateCB(task_update_cb);

	delete m_pool;
}

ua::Source* AudioContext::CreateSource(const AudioData* data)
{
	return NULL;
}

ua::Source* AudioContext::CreateSource(Decoder* decoder)
{
	return new Source(m_pool, decoder);
}

ua::Source* AudioContext::CreateSource(const std::string& filepath, bool stream)
{
	if (stream) {
		return new Source(m_pool, DecoderFactory::Create(filepath));
	} else {
		return new Source(m_pool, filepath);
	}
}

#ifdef __ANDROID__

void AudioContext::InitAAssetMgr(JNIEnv* env, jobject assetManager)
{
	m_aasset_mgr = AAssetManager_fromJava(env, assetManager);
}

bool AudioContext::LoadAssetFile(const std::string& filepath, SLDataLocator_AndroidFD* loc_fd)
{
	AAsset* asset = AAssetManager_open(m_aasset_mgr, filepath.c_str(), AASSET_MODE_UNKNOWN);
	if (!asset) {
		return false;
	}

	off_t start, length;
	int fd = AAsset_openFileDescriptor(asset, &start, &length);
	AAsset_close(asset);
	if (fd < 0) {
		return false;
	}
	SLDataLocator_AndroidFD _loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
	*loc_fd = _loc_fd;
	
	return true;
}

#endif // __ANDROID__

bool AudioContext::CreateEngine()
{
	SLresult result;

	// create engine
	result = slCreateEngine(&m_engine_obj, 0, NULL, 0, NULL, NULL);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// realize the engine
	result = (*m_engine_obj)->Realize(m_engine_obj, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the engine interface, which is needed in order to create other objects
	result = (*m_engine_obj)->GetInterface(m_engine_obj, SL_IID_ENGINE, &m_engine_engine);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// create output mix, with environmental reverb specified as a non-required interface
	const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = (*m_engine_engine)->CreateOutputMix(m_engine_engine, &m_output_mix_obj, 1, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// realize the output mix
	result = (*m_output_mix_obj)->Realize(m_output_mix_obj, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void)result;

	// get the environmental reverb interface
	// this could fail if the environmental reverb effect is not available,
	// either because the feature is not present, excessive CPU load, or
	// the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
	result = (*m_output_mix_obj)->GetInterface(m_output_mix_obj, SL_IID_ENVIRONMENTALREVERB,
		&m_output_mix_env_reverb);
	if (SL_RESULT_SUCCESS == result) {
		result = (*m_output_mix_env_reverb)->SetEnvironmentalReverbProperties(
			m_output_mix_env_reverb, &m_reverb_settings);
		(void)result;
	}

	return true;
}

}
}