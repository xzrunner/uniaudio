#ifndef _UNIAUDIO_OPENSL_AUDIO_PLAYER_H_
#define _UNIAUDIO_OPENSL_AUDIO_PLAYER_H_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace ua
{
namespace opensl
{

class AssetPlayer
{
public:
	SLObjectItf   object;
	SLPlayItf     play;
	SLSeekItf     seek;
	SLMuteSoloItf mute_solo;
	SLVolumeItf   volume;

public:
	AssetPlayer();
	~AssetPlayer();

	void Release();

}; // AssetPlayer

class QueuePlayer
{
public:
	SLObjectItf                   object;
	SLPlayItf		              play;
	SLAndroidSimpleBufferQueueItf queue;
//	SLEffectSendItf               effect_send;
	SLMuteSoloItf                 mute_solo;
	SLVolumeItf                   volume;
	SLmilliHertz                  sample_rate;

public:
	QueuePlayer();
	~QueuePlayer();

}; // QueuePlayer

}
}

#endif // _UNIAUDIO_OPENSL_AUDIO_PLAYER_H_