#include "cpp_style.hpp"

struct AllocateEntites {
    AllocateEntites() SKR_NOEXCEPT
    {
        storage = sugoiS_create();
    }
    ~AllocateEntites() SKR_NOEXCEPT
    {
        ::sugoiS_release(storage);
    }
    sugoi_storage_t* storage;
};

TEST_CASE_METHOD(AllocateEntites, "AllocateOne")
{

}