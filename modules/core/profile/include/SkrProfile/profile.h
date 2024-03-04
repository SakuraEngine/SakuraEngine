#pragma once

#ifdef __OPTIMIZE__
    // Some platforms define NDEBUG for Release builds
    #ifndef NDEBUG
        #define NDEBUG
    #endif
#elif !defined(_MSC_VER)
    #define _DEBUG 1
#endif

#pragma region tracy

#if !defined(SKR_PROFILE_ENABLE) && !defined(SKR_PROFILE_OVERRIDE_DISABLE) && !defined(SKR_PROFILE_OVERRIDE_ENABLE)
    #ifdef _DEBUG
        #define SKR_PROFILE_ENABLE
    #else
    #endif
#endif

#if !defined(SKR_PROFILE_ENABLE) && defined(SKR_PROFILE_OVERRIDE_ENABLE)
    #define SKR_PROFILE_ENABLE
#endif

#if defined(SKR_PROFILE_ENABLE)
    #ifndef TRACY_ENABLE
    #define TRACY_ENABLE
    #endif

    #ifndef TRACY_IMPORTS
    #define TRACY_IMPORTS
    #endif
    
    #ifndef TRACY_ON_DEMAND
    #define TRACY_ON_DEMAND
    #endif

    #ifndef TRACY_FIBERS
    #define TRACY_FIBERS
    #endif

    #ifndef TRACY_TRACE_ALLOCATION
    #define TRACY_TRACE_ALLOCATION
    #endif
#endif

#include "../../internal/tracy/tracy/TracyC.h"

typedef TracyCZoneCtx SkrCZoneCtx;

#define SkrCZone( ctx, active ) TracyCZone( ctx, active )
#define SkrCZoneN( ctx, name, active ) TracyCZoneN( ctx, name, active ) 
#define SkrCZoneC( ctx, color, active ) TracyCZoneC( ctx, color, active ) 
#define SkrCZoneNC( ctx, name, color, active ) TracyCZoneNC( ctx, name, color, active )

#define SkrCZoneEnd( ctx ) TracyCZoneEnd( ctx )

#define SkrCZoneText( ctx, txt, size ) TracyCZoneText( ctx, txt, size )
#define SkrCZoneName( ctx, txt, size ) TracyCZoneName( ctx, txt, size )
#define SkrCZoneColor( ctx, color ) TracyCZoneColor( ctx, color )
#define SkrCZoneValue( ctx, value ) TracyCZoneValue( ctx, value )

#define SkrCAlloc( ptr, size ) TracyCAlloc( ptr, size ) 
#define SkrCFree( ptr ) TracyCFree( ptr )
#define SkrCSecureAlloc( ptr, size ) TracyCSecureAlloc( ptr, size ) 
#define SkrCSecureFree( ptr ) TracyCSecureFree( ptr ) 

#define SkrCAllocN( ptr, size, name ) TracyCAllocN( ptr, size, name ) 
#define SkrCFreeN( ptr, name ) TracyCFreeN( ptr, name ) 
#define SkrCSecureAllocN( ptr, size, name ) TracyCSecureAllocN( ptr, size, name ) 
#define SkrCSecureFreeN( ptr, name ) TracyCSecureFreeN( ptr, name ) 

#define SkrCMessage( txt, size ) TracyCMessage( txt, size ) 
#define SkrCMessageL( txt ) TracyCMessageL( txt ) 
#define SkrCMessageC( txt, size, color ) TracyCMessageC( txt, size, color ) 
#define SkrCMessageLC( txt, color ) TracyCMessageLC( txt, color ) 

#define SkrCFrameMark TracyCFrameMark
#define SkrCFrameMarkNamed( name ) TracyCFrameMarkNamed( name ) 
#define SkrCFrameMarkStart( name ) TracyCFrameMarkStart( name )
#define SkrCFrameMarkEnd( name ) TracyCFrameMarkEnd( name )
#define SkrCFrameImage( image, width, height, offset, flip ) TracyCFrameImage( image, width, height, offset, flip )

#define SkrCPlot( name, val ) TracyCPlot( name, val )
#define SkrCPlotF( name, val ) TracyCPlotF( name, val )
#define SkrCPlotI( name, val ) TracyCPlotI( name, val )
#define SkrCPlotConfig( name, type, step, fill, color ) TracyCPlotConfig( name, type, step, fill, color )
#define SkrCAppInfo( txt, size ) TracyCAppInfo( txt, size )

#define SkrCZoneS( ctx, depth, active ) TracyCZoneS( ctx, depth, active )
#define SkrCZoneNS( ctx, name, depth, active ) TracyCZoneNS( ctx, name, depth, active )
#define SkrCZoneCS( ctx, color, depth, active ) TracyCZoneCS( ctx, color, depth, active )
#define SkrCZoneNCS( ctx, name, color, depth, active ) TracyCZoneNCS( ctx, name, color, depth, active )

#define SkrCAllocS( ptr, size, depth ) TracyCAllocS( ptr, size, depth )
#define SkrCFreeS( ptr, depth ) TracyCFreeS( ptr, depth )
#define SkrCSecureAllocS( ptr, size, depth ) TracyCSecureAllocS( ptr, size, depth )
#define SkrCSecureFreeS( ptr, depth ) TracyCSecureFreeS( ptr, depth )

#define SkrCAllocNS( ptr, size, depth, name ) TracyCAllocNS( ptr, size, depth, name )
#define SkrCFreeNS( ptr, depth, name ) TracyCFreeNS( ptr, depth, name )
#define SkrCSecureAllocNS( ptr, size, depth, name ) TracyCSecureAllocNS( ptr, size, depth, name )
#define SkrCSecureFreeNS( ptr, depth, name ) TracyCSecureFreeNS( ptr, depth, name )

#define SkrCMessageS( txt, size, depth ) TracyCMessageS( txt, size, depth )
#define SkrCMessageLS( txt, depth ) TracyCMessageLS( txt, depth )
#define SkrCMessageCS( txt, size, color, depth ) TracyCMessageCS( txt, size, color, depth )
#define SkrCMessageLCS( txt, color, depth ) TracyCMessageLCS( txt, color, depth )

#define SkrCIsConnected TracyCIsConnected

#define SkrCFiberEnter( fiber ) TracyCFiberEnter( fiber )
#define SkrCFiberLeave TracyCFiberLeave

#ifdef __cplusplus
#include "../../internal/tracy/tracy/Tracy.hpp"

#define SkrZoneNamed(x,y) ZoneNamed(x,y)
#define SkrZoneNamedN(x,y,z) ZoneNamedN(x,y,z)
#define SkrZoneNamedC(x,y,z) ZoneNamedC(x,y,z)
#define SkrZoneNamedNC(x,y,z,w) ZoneNamedNC(x,y,z,w)

#define SkrZoneTransient(x,y) ZoneTransient(x,y)
#define SkrZoneTransientN(x,y,z) ZoneTransientN(x,y,z)

#define SkrZoneScoped ZoneScoped
#define SkrZoneScopedN(x) ZoneScopedN(x)
#define SkrZoneScopedC(x) ZoneScopedC(x)
#define SkrZoneScopedNC(x,y) ZoneScopedNC(x,y)

#define SkrZoneText(x,y) ZoneText(x,y)
#define SkrZoneTextV(x,y,z) ZoneTextV(x,y,z)
#define SkrZoneName(x,y) ZoneName(x,y)
#define SkrZoneNameV(x,y,z) ZoneNameV(x,y,z)
#define SkrZoneColor(x) ZoneColor(x)
#define SkrZoneColorV(x,y) ZoneColorV(x,y)
#define SkrZoneValue(x) ZoneValue(x)
#define SkrZoneValueV(x,y) ZoneValueV(x,y)
#define SkrZoneIsActive ZoneIsActive 
#define SkrZoneIsActiveV(x)  ZoneIsActiveV(x)

#define SkrFrameMark FrameMark
#define SkrFrameMarkNamed(x) FrameMarkNamed(x)
#define SkrFrameMarkStart(x) FrameMarkStart(x)
#define SkrFrameMarkEnd(x) FrameMarkEnd(x)

#define SkrFrameImage(x,y,z,w,a) FrameImage(x,y,z,w,a)

#define SkrLockable( type, varname ) TracyLockable( type, varname )
#define SkrLockableN( type, varname, desc ) TracyLockableN( type, varname, desc )
#define SkrSharedLockable( type, varname ) TracySharedLockable( type, varname )
#define SkrSharedLockableN( type, varname, desc ) TracySharedLockableN( type, varname, desc )
#define SkrLockableBase( type ) LockableBase( type )
#define SkrSharedLockableBase( type ) SharedLockableBase( type )
#define SkrLockMark(x) LockMark(x) 
#define SkrLockableName(x,y,z) LockableName(x,y,z)

#define SkrPlot(x,y) TracyPlot(x,y)
#define SkrPlotConfig(x,y,z,w,a) TracyPlotConfig(x,y,z,w,a)

#define SkrMessage(x,y) TracyMessage(x,y)
#define SkrMessageL(x) TracyMessageL(x)
#define SkrMessageC(x,y,z) TracyMessageC(x,y,z)
#define SkrMessageLC(x,y) TracyMessageLC(x,y)
#define SkrAppInfo(x,y) TracyAppInfo(x,y)

#define SkrAlloc(x,y) TracyAlloc(x,y)
#define SkrFree(x) TracyFree(x)
#define SkrSecureAlloc(x,y) TracySecureAlloc(x,y)
#define SkrSecureFree(x) TracySecureFree(x)

#define SkrAllocN(x,y,z) TracyAllocN(x,y,z)
#define SkrFreeN(x,y) TracyFreeN(x,y)
#define SkrSecureAllocN(x,y,z) TracySecureAllocN(x,y,z)
#define SkrSecureFreeN(x,y) TracySecureFreeN(x,y)

#define SkrZoneNamedS(x,y,z) ZoneNamedS(x,y,z)
#define SkrZoneNamedNS(x,y,z,w) ZoneNamedNS(x,y,z,w)
#define SkrZoneNamedCS(x,y,z,w) ZoneNamedCS(x,y,z,w)
#define SkrZoneNamedNCS(x,y,z,w,a) ZoneNamedNCS(x,y,z,w,a)

#define SkrZoneTransientS(x,y,z) ZoneTransientS(x,y,z)
#define SkrZoneTransientNS(x,y,z,w) ZoneTransientNS(x,y,z,w)

#define SkrZoneScopedS(x) ZoneScopedS(x)
#define SkrZoneScopedNS(x,y) ZoneScopedNS(x,y)
#define SkrZoneScopedCS(x,y) ZoneScopedCS(x,y)
#define SkrZoneScopedNCS(x,y,z) ZoneScopedNCS(x,y,z)

#define SkrAllocS(x,y,z) TracyAllocS(x,y,z)
#define SkrFreeS(x,y) TracyFreeS(x,y)
#define SkrSecureAllocS(x,y,z) TracySecureAllocS(x,y,z)
#define SkrSecureFreeS(x,y) TracySecureFreeS(x,y)

#define SkrAllocNS(x,y,z,w) TracyAllocNS(x,y,z,w)
#define SkrFreeNS(x,y,z) TracyFreeNS(x,y,z)
#define SkrSecureAllocNS(x,y,z,w) TracySecureAllocNS(x,y,z,w)
#define SkrSecureFreeNS(x,y,z) TracySecureFreeNS(x,y,z)

#define SkrMessageS(x,y,z) TracyMessageS(x,y,z)
#define SkrMessageLS(x,y) TracyMessageLS(x,y)
#define SkrMessageCS(x,y,z,w) TracyMessageCS(x,y,z,w)
#define SkrMessageLCS(x,y,z) TracyMessageLCS(x,y,z)

#define SkrSourceCallbackRegister(x,y) TracySourceCallbackRegister(x,y)
#define SkrParameterRegister(x,y) TracyParameterRegister(x,y)
#define SkrParameterSetup(x,y,z,w) TracyParameterSetup(x,y,z,w)
#define SkrIsConnected TracyIsConnected
#define SkrSetProgramName(x) TracySetProgramName(x)

#define SkrFiberEnter(x) TracyFiberEnter(x)
#define SkrFiberLeave TracyFiberLeave

#endif

#pragma endregion