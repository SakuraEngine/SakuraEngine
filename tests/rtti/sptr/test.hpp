#pragma once
#include "gtest/gtest.h"
#include "containers/sptr.hpp"
#include "misc/log.hpp"
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