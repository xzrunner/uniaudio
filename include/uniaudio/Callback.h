#ifndef _UNIAUDIO_CALLBACK_H_
#define _UNIAUDIO_CALLBACK_H_

namespace ua
{

class Callback
{
public:
	struct API
	{
		void (*register_async_update)(void (*update)(void* arg), void* arg);
		void (*unregister_async_update)(void (*update)(void* arg));
	};

	static void InitCallback(const API& api);

	//////////////////////////////////////////////////////////////////////////

	static void RegisterAsyncUpdate(void (*update)(void* arg), void* arg);
	static void UnregisterAsyncUpdate(void (*update)(void* arg));

}; // Callback

}

#endif // _UNIAUDIO_CALLBACK_H_