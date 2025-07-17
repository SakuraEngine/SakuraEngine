#include "SkrV8/v8_inspector.hpp"
#include "SkrV8/v8_context.hpp"
#include "v8-platform.h"
#include "libplatform/libplatform.h"
#include "SkrV8/v8_isolate.hpp"

namespace skr
{
V8WebSocketServer::V8WebSocketServer() = default;
V8WebSocketServer::~V8WebSocketServer()
{
    shutdown();
}

// init & shutdown
bool V8WebSocketServer::init(int port)
{
    shutdown();

    _port = port;

    // init websocket server
    _ws_service.onopen = [this](const WebSocketChannelPtr& channel, const HttpRequestPtr& req) {
        if (!_main_channel)
        {
            _main_channel = channel;
            SKR_LOG_DEBUG(u8"V8WebSocketServer: new channel connected");
        }
    };
    _ws_service.onmessage = [this](const WebSocketChannelPtr& channel, const std::string& msg) {
        std::lock_guard<std::mutex> _lck(_mutex);
        _received_messages.push_back(String::From(msg.data(), msg.size()));
    };
    _ws_service.onclose = [this](const WebSocketChannelPtr& channel) {
        _main_channel.reset();
        SKR_LOG_DEBUG(u8"V8WebSocketServer: channel closed");
    };

    // init http service
    _http_service.GET("/json/version", [this](HttpRequest* req, HttpResponse* resp) {
        return resp->String(_json_version.data_raw());
    });
    _http_service.GET("/json/list", [this](HttpRequest* req, HttpResponse* resp) {
        return resp->String(_json_list.data_raw());
    });

    // startup server
    _ws_server         = hv::WebSocketServer(&_ws_service);
    _ws_server.service = &_http_service;
    _ws_server.setPort(_port);
    _ws_server.setThreadNum(1);
    _ws_server.start();

    _combine_json_response();

    _is_running = true;

    return true;
}
void V8WebSocketServer::shutdown()
{
    if (_is_running)
    {
        _ws_server.stop();
        _main_channel.reset();
        _is_running = false;
    }
}
bool V8WebSocketServer::is_init() const
{
    return _is_running;
}

// message
void V8WebSocketServer::send_message(StringView message)
{
    if (_main_channel)
    {
        _main_channel->send(message.data_raw());
    }
    else
    {
        SKR_LOG_ERROR(u8"V8WebSocketServer: no main channel, message not sent!");
    }
}
void V8WebSocketServer::pump_messages()
{
    std::lock_guard _lck(_mutex);
    for (auto& message : _received_messages)
    {
        on_message_received.invoke(message);
    }
    _received_messages.clear();
}

// json response
void V8WebSocketServer::_combine_json_response()
{
    _json_version.clear();
    _json_version.append(u8R"__({
        "Browser": "SakuraEngine",
        "Protocol-Version": "1.1"
    })__");

    _json_list.clear();
    _json_list.append(u8"[{");
    _json_list.append(u8R"("description": "SakuraEngine Inspector",)");
    _json_list.append(u8R"("id": "0",)");
    _json_list.append(u8R"("title": "SakuraEngine Inspector",)");
    _json_list.append(u8R"("type": "node",)");
    _json_list.append(format(u8R"("webSocketDebuggerUrl": "ws://127.0.0.1:{}")", _port));
    _json_list.append(u8"}]");
}

// override
void V8InspectorChannel::sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message)
{
    String message_str;
    if (message->string().is8Bit())
    {
        message_str = String::From((const char*)message->string().characters8(), message->string().length());
    }
    else
    {
        message_str = String::From((const wchar_t*)message->string().characters16(), message->string().length());
    }

    server->send_message(message_str);
}
void V8InspectorChannel::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message)
{
    String message_str;
    if (message->string().is8Bit())
    {
        message_str = String::From((const char*)message->string().characters8(), message->string().length());
    }
    else
    {
        message_str = String::From((const wchar_t*)message->string().characters16(), message->string().length());
    }

    server->send_message(message_str);
}
void V8InspectorChannel::flushProtocolNotifications()
{
    // do nothing
}

// init & shutdown
void V8InspectorClient::init(V8Isolate* isolate)
{
    _isolate = isolate;

    // TODO. session & channel 根据连接情况动态创建

    // pass server
    _channel.server = server;

    // create v8 data
    _inspector = v8_inspector::V8Inspector::create(isolate->v8_isolate(), this);
    _session   = _inspector->connect(
        kContextGroupId,
        &_channel,
        v8_inspector::StringView{ (const uint8_t*)kContextName.data(), kContextName.size() },
        v8_inspector::V8Inspector::ClientTrustLevel::kFullyTrusted,
        v8_inspector::V8Inspector::SessionPauseState::kWaitingForDebugger
    );

    // register callback
    _on_message_received_handle = server->on_message_received.bind_lambda(
        [this](StringView message) {
            _session->dispatchProtocolMessage(
                { (const uint8_t*)message.data_raw(),
                  message.length_buffer() }
            );
        }
    );
}
void V8InspectorClient::shutdown()
{
    // unregister callback
    server->on_message_received.remove(_on_message_received_handle);

    _session.reset();
    _inspector.reset();
}
bool V8InspectorClient::is_init() const
{
    return _session != nullptr && _inspector != nullptr;
}

// override
void V8InspectorClient::runMessageLoopOnPause(int contextGroupId)
{
    if (_is_runing_message_loop) { return; }

    _is_runing_message_loop = true;
    _run_message_loop       = true;

    while (_run_message_loop)
    {
        server->pump_messages();
        _isolate->pump_message_loop();
    }

    _run_message_loop       = false;
    _is_runing_message_loop = false;
}
void V8InspectorClient::quitMessageLoopOnPause()
{
    _run_message_loop = false;
}
void V8InspectorClient::runIfWaitingForDebugger(int contextGroupId)
{
    // pause_on_next_statement(u8"for test");
    _run_message_loop = true;
    _connected        = true;
}

// notify
void V8InspectorClient::notify_context_created(V8Context* context)
{
    v8::HandleScope handle_scope(_isolate->v8_isolate());

    v8_inspector::StringView name_view{
        (const uint8_t*)context->name().data_raw(),
        context->name().length_buffer()
    };

    v8_inspector::V8ContextInfo info{
        context->v8_context().Get(_isolate->v8_isolate()),
        kContextGroupId,
        name_view,
    };
    _inspector->contextCreated(info);
}
void V8InspectorClient::notify_context_destroyed(V8Context* context)
{
    v8::HandleScope handle_scope(_isolate->v8_isolate());
    _inspector->contextDestroyed(context->v8_context().Get(_isolate->v8_isolate()));
}

// debug control
void V8InspectorClient::pause_on_next_statement(StringView reason)
{
    v8_inspector::StringView reason_view{ (const uint8_t*)reason.data_raw(), reason.length_buffer() };
    _session->schedulePauseOnNextStatement(reason_view, reason_view);
}
void V8InspectorClient::run_message_loop_on_next_pause()
{
    _run_message_loop = true;
}

} // namespace skr