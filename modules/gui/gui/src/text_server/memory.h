#pragma once
#include "text_server/safe_refcount.h"
#include <new>

namespace godot
{
class Memory {
#ifdef DEBUG_ENABLED
	static SafeNumeric<uint64_t> mem_usage;
	static SafeNumeric<uint64_t> max_usage;
#endif
	static SafeNumeric<uint64_t> alloc_count;

public:
	static void* alloc_static(size_t p_bytes, bool p_pad_align = false);
	static void* realloc_static(void *p_memory, size_t p_bytes, bool p_pad_align = false);
	static void free_static(void *p_ptr, bool p_pad_align = false);

	static uint64_t get_mem_available();
	static uint64_t get_mem_usage();
	static uint64_t get_mem_max_usage();
};

template <class T>
void memdelete(T *p_class)
{
	delete p_class;
}

void memfree(void* p_ptr);

void* memalloc(size_t size);

void* memrealloc(void* p_ptr, size_t size);

#define memnew_placement(ptr, T) new(ptr) T
#define memnew(T) new T

}