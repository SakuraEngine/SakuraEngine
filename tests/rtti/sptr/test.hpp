#pragma once
#include "gtest/gtest.h"
#include "containers/sptr.hpp"
#include "utils/log.hpp"
#include "platform/guid.hpp"

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