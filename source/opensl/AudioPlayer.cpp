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
	: object(NULL) 
	, play(NULL)
	, seek(NULL)
	, mute_solo(NULL)
	, volume(NULL)
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
	play = NULL;
	seek = NULL;
	mute_solo = NULL;
	volume = NULL;
}

/************************************************************************/
/* class QueuePlayer                                                    */
/************************************************************************/

QueuePlayer::QueuePlayer()
	: object(NULL)
	, play(NULL)
	, queue(NULL)
	, effect_send(NULL)
	, mute_solo(NULL)
	, volume(0)
	, sample_rate(0)
{
}

QueuePlayer::~QueuePlayer() {
	if (object) {
		(*object)->Destroy(object);
	}
}

}
}