using SB;
using SB.Core;

[TargetScript]
public static class RTM
{
    static RTM()
    {
        Engine.Target("rtm")
            .TargetType(TargetType.HeaderOnly)
            .IncludeDirs(Visibility.Public, "include")
            .Defines(Visibility.Public, "RTM_ON_ASSERT_ABORT");
    }
}