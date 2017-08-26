#include "uniaudio/Callback.h"

namespace ua
{

static Callback::API CB;

void Callback::InitCallback(const Callback::API& api)
{
	CB = api;
}

void Callback::RegisterAsyncUpdate(void (*update)(void* arg), void* arg)
{
	CB.register_async_update(update, arg);
}

void Callback::UnregisterAsyncUpdate(void (*update)(void* arg))
{
	CB.unregister_async_update(update);
}

}