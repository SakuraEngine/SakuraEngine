using SB;
using SB.Core;

[TargetScript]
public static class SDL
{
    static SDL()
    {
        var SDL = Engine.Target("SDL3")
            .TargetType(TargetType.HeaderOnly)
            .IncludeDirs(Visibility.Public, "include")
            .Defines(Visibility.Public, "SDL_MAIN_HANDLED=1");
        
        if (BuildSystem.TargetOS == OSPlatform.Windows)
        {
            SDL.Link(Visibility.Public, "SDL3");
        }
        if (BuildSystem.TargetOS == OSPlatform.OSX)
        {
            SDL.Link(Visibility.Public, "SDL3.0");
        }
    }
}