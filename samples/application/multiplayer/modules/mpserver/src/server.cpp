#include "MPServer/server.h"
#include "platform/thread.h"
#include "platform/time.h"
#include "simdjson.h"
#include "steam/isteamnetworkingutils.h"
#include "steam/isteamnetworkingsockets.h"
#include "steam/steamnetworkingsockets.h"
#include "misc/log.h"
#include "MPShared/server_world.h"
#include "MPShared/signal_client.h"

static SteamNetworkingMicroseconds g_logTimeZero;
static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg )
{
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	if ( eType <= k_ESteamNetworkingSocketsDebugOutputType_Msg )
	{
		SKR_LOG_INFO( "%10.6f %s\n", time*1e-6, pszMsg );
	}
	if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug )
	{
		// !KLUDGE! Our logging (which is done while we hold the lock)
		// is occasionally triggering this assert.  Just ignroe that one
		// error for now.
		// Yes, this is a kludge.
		if ( strstr( pszMsg, "SteamNetworkingGlobalLock held for" ) )
			return;

		SKR_ASSERT( !"TEST FAILED" );
	}
}

int InitializeSockets()
{
	// Initialize library, with the desired local identity
	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
	SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Debug, DebugOutput );
	SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_LogLevel_P2PRendezvous, k_ESteamNetworkingSocketsDebugOutputType_Debug );
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendBufferSize, 1024 * 1024 * 10);
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_Unencrypted, 2);
	//SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Send, 100);
	//SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, 000);
    SteamDatagramErrMsg errMsg;
    if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
    {
        SKR_LOG_FATAL( "GameNetworkingSockets_Init failed.  %s", errMsg );
        return 1;
    }

	const char* turnList = "turn:benzzzx.ticp.io:3478";
	// Hardcode STUN servers
	SteamNetworkingUtils()->SetGlobalConfigValueString( k_ESteamNetworkingConfig_P2P_STUN_ServerList, "stun:benzzzx.ticp.io:3478");

	// Hardcode TURN servers
	// comma seperated setting lists
	const char* userList = "admin"; 
	const char* passList = "aaa"; 

	SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_TURN_ServerList, turnList);
	SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_TURN_UserList, userList);
	SteamNetworkingUtils()->SetGlobalConfigValueString(k_ESteamNetworkingConfig_P2P_TURN_PassList, passList);
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendRateMin , std::numeric_limits<int32_t>::min());
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_SendRateMax , std::numeric_limits<int32_t>::max());

	// Allow sharing of any kind of ICE address.
	// We don't have any method of relaying (TURN) in this example, so we are essentially
	// forced to disclose our public address if we want to pierce NAT.  But if we
	// had relay fallback, or if we only wanted to connect on the LAN, we could restrict
	// to only sharing private addresses.
	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_P2P_Transport_ICE_Enable, k_nSteamNetworkingConfig_P2P_Transport_ICE_Enable_All );
    
    return 0;
}

int gVirtualPortLocal = 0;
HSteamListenSocket g_hListenSock;

const char *pszTrivialSignalingService = "benzzzx.ticp.io:10000";



MPServerWorld* g_world;
// Called when a connection undergoes a state transition.
void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo )
{
	// What's the state of the connection?
	switch ( pInfo->m_info.m_eState )
	{
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
    {
		SKR_LOG_INFO( "[%s] %s, reason %d: %s\n",
			pInfo->m_info.m_szConnectionDescription,
			( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ? "closed by peer" : "problem detected locally" ),
			pInfo->m_info.m_eEndReason,
			pInfo->m_info.m_szEndDebug
		);
		// Close our end
		SteamNetworkingSockets()->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
    }
    break;

	case k_ESteamNetworkingConnectionState_None:
		// Notification that a connection was destroyed.  (By us, presumably.)
		// We don't need this, so ignore it.
		break;

	case k_ESteamNetworkingConnectionState_Connecting:

		// Is this a connection we initiated, or one that we are receiving?
		if ( g_hListenSock != k_HSteamListenSocket_Invalid && pInfo->m_info.m_hListenSocket == g_hListenSock )
		{
			SKR_LOG_INFO( "[%s] Accepting\n", pInfo->m_info.m_szConnectionDescription );
            SteamNetworkingSockets()->AcceptConnection( pInfo->m_hConn );
            g_world->AddConnection(pInfo->m_hConn);
		}
		else
		{
			// Note that we will get notification when our own connection that
			// we initiate enters this state.
			SKR_LOG_INFO( "[%s] Entered connecting state\n", pInfo->m_info.m_szConnectionDescription );
		}
		break;

	case k_ESteamNetworkingConnectionState_FindingRoute:
		// P2P connections will spend a brief time here where they swap addresses
		// and try to find a route.
		SKR_LOG_INFO( "[%s] finding route\n", pInfo->m_info.m_szConnectionDescription );
		break;

	case k_ESteamNetworkingConnectionState_Connected:
		// We got fully connected
		SKR_LOG_INFO( "[%s] connected\n", pInfo->m_info.m_szConnectionDescription );
		break;

	default:
		SKR_ASSERT( false );
		break;
	}
}

int main(int argc, char** argv)
{
    //auto str = "{\"frame\":1,\"inputs\":[{\"move\":{\"x\":0,\"y\":0}}]}";
    //ReceiveInput(str, strlen(str));
	srand((unsigned int)time(nullptr));
    skr::task::scheduler_t scheduler;
    scheduler.initialize({});
    scheduler.bind();
	SteamNetworkingIdentity identityLocal; identityLocal.ParseString("str:server");
    for(int i=0; i<argc; ++i)
    {
		const char *pszSwitch = argv[i];
        auto GetArg = [&]() -> const char * {
			if ( i + 1 >= argc )
				SKR_LOG_FATAL( "Expected argument after %s", pszSwitch );
			return argv[++i];
		};
		auto ParseIdentity = [&]( SteamNetworkingIdentity &x ) {
			const char *pszArg = GetArg();
			if ( !x.ParseString( pszArg ) )
				SKR_LOG_FATAL( "'%s' is not a valid identity string", pszArg );
		};
        if( !strcmp(pszSwitch, "--identity"))
            ParseIdentity( identityLocal );
    }
    if(InitializeSockets())
        return 1;
    SteamNetworkingSockets()->ResetIdentity(&identityLocal);
    SteamDatagramErrMsg errMsg;
    // Create the signaling service
    auto signaling = CreateTrivialSignalingClient( pszTrivialSignalingService, SteamNetworkingSockets(), errMsg );
    if ( signaling == nullptr )
        SKR_LOG_FATAL( "Failed to initializing signaling client.  %s", errMsg );

    MPServerWorld world;
	g_world = &world;
    world.Initialize();
        
    SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged( OnSteamNetConnectionStatusChanged );
    
    SKR_LOG_INFO( "Creating listen socket, local virtual port %d\n", gVirtualPortLocal );
    g_hListenSock = SteamNetworkingSockets()->CreateListenSocketP2P( gVirtualPortLocal, 0, nullptr );
    SKR_ASSERT(g_hListenSock);
    
    for(;;)
    {
        if(signaling)
            signaling->Poll();
        SteamNetworkingSockets()->RunCallbacks();
        world.Update();
        dualJ_wait_all();
        //std::this_thread::sleep_for(std::chrono::seconds::zero());
    }
    scheduler.unbind();
}