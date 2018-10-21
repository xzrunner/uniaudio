#include "uniaudio/opensl/AudioPlayer.h"

#include <stddef.h>

namespace ua
{
namespace opensl
{

/************************************************************************/
/* class AssetPlayer                                                    */
/************************************************************************/

AssetPlayer::AssetPlayer()
	: object(nullptr)
	, play(nullptr)
	, seek(nullptr)
	, mute_solo(nullptr)
	, volume(nullptr)
{
}

AssetPlayer::~AssetPlayer()
{
	if (object) {
		(*object)->Destroy(object);
	}
}

void AssetPlayer::Release()
{
	if (object) {
		(*object)->Destroy(object);
	}
	play = nullptr;
	seek = nullptr;
	mute_solo = nullptr;
	volume = nullptr;
}

/************************************************************************/
/* class QueuePlayer                                                    */
/************************************************************************/

QueuePlayer::QueuePlayer()
	: object(nullptr)
	, play(nullptr)
	, queue(nullptr)
//	, effect_send(nullptr)
	, mute_solo(nullptr)
	, volume(0)
	, sample_rate(SL_SAMPLINGRATE_44_1)
{
}

QueuePlayer::~QueuePlayer() {
	if (object) {
		(*object)->Destroy(object);
	}
}

}
}