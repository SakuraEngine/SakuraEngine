#include "types.hpp"

namespace das
{
void ThrowCtxError(::das::Context& context, const char8_t* msg)
{
    context.throw_error((const char*)msg);
}

skr::das::reg4f EvalNode(::das::Context& context, ::das::SimNode* node)
{
    return node->eval(context);
}

int32_t EvalInt(::das::Context& context, ::das::SimNode* node)
{
    return node->evalInt(context);
}

uint32_t EvalUInt(::das::Context& context, ::das::SimNode* node)
{
    return node->evalUInt(context);
}

int64_t EvalInt64(::das::Context& context, ::das::SimNode* node)
{
    return node->evalInt64(context);
}

int64_t EvalUInt64(::das::Context& context, ::das::SimNode* node)
{
    return node->evalUInt64(context);
}

int64_t EvalFloat(::das::Context& context, ::das::SimNode* node)
{
    return node->evalFloat(context);
}

double EvalDouble(::das::Context& context, ::das::SimNode* node)
{
    return node->evalDouble(context);
}

float EvalBool(::das::Context& context, ::das::SimNode* node)
{
    return node->evalBool(context);
}

char* EvalPtr(::das::Context& context, ::das::SimNode* node)
{
    return node->evalPtr(context);
}

} // namespace das
