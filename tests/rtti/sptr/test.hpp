#pragma once
#include "gtest/gtest.h"
#include "containers/sptr.hpp"
#include "SkrRT/misc/log.hpp"
#include "SkrRT/platform/guid.hpp"

class SPTRBase : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};