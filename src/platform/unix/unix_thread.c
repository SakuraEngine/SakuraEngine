#include "platform/thread.h"
#include "platform/atomic.h"
#include <pthread.h>

static SCallOnceGuard gKeyInitGuard;
static pthread_key_t gThreadIDKey;

static void destroyThreadIDKey() 
{
	pthread_key_delete(gThreadIDKey);
}

static void initThreadIDKey() 
{
	int result = pthread_key_create(&gThreadIDKey, NULL);
	result = atexit(destroyThreadIDKey);
}

SThreadID skrGetCurrentPthreadID() 
{
	static SAtomic32 counter = 1;
	if (counter == 1)
	{
		skr_init_call_once_guard(&gKeyInitGuard);
	}
	skr_call_once(&gKeyInitGuard, initThreadIDKey);

	void* ptr = pthread_getspecific(gThreadIDKey);
	uintptr_t ptr_id = (uintptr_t)ptr;
	SThreadID id = (SThreadID)ptr_id;

	// thread id wasn't set
	if (id == 0) 
	{
		id = (SThreadID)tfrg_atomic32_add_relaxed(&counter, 1);
		ASSERT(id != 0 && "integer overflow");
		// we store plain integers instead of pointers to data
		ptr_id = (uintptr_t)id;
		ptr = (void*)ptr_id;
		int result = pthread_setspecific(gThreadIDKey, ptr);
	}
	return id;
}
