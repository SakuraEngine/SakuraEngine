
#pragma once
#include "ecs/dual.h"

namespace dual
{
    struct serializer_t
    {
        void* t;
        const dual_serializer_v* v;
        void archive(void* data, uint32_t bytes);
        void peek(void* data, uint32_t bytes);
        bool is_serialize();
        template<class T>
        void peek(T& data)
        {
            peek((void*)&data, sizeof(T));
        }
        template<class T>
        void archive(T& data)
        {
            archive(&data, sizeof(T));
        }
        template<class T>
        void archive(const T& data)
        {
            archive(&data, sizeof(T));
        }
        template<class T>
        void archive(T* data, uint32_t size)
        {
            archive((void*)data, sizeof(T) * size);
        }
    };
}