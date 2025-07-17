#include "hv/WebSocketServer.h"
#include "hv/WebSocketClient.h"
#include <WebSocketSample/web_socket_cmds.hpp>
#include <SkrCore/log.hpp>

namespace skr
{
int _main_server()
{
    using namespace hv;

    WebSocketChannelPtr first_channel = nullptr;
    WebSocketService    ws;
    ws.onopen = [&](const WebSocketChannelPtr& channel, const HttpRequestPtr& req) {
        if (!first_channel)
        {
            first_channel = channel;
        }
        SKR_LOG_INFO(u8"圆头: GET %s", req->Path().c_str());
    };
    ws.onmessage = [](const WebSocketChannelPtr& channel, const std::string& msg) {
        SKR_LOG_INFO(u8"圆头收到: %.*s", (int)msg.size(), msg.data());
        channel->send("我要管理!!!!!");
    };
    ws.onclose = [](const WebSocketChannelPtr& channel) {
        SKR_LOG_INFO(u8"圆头似了");
    };

    HttpService http;
    http.GET("/work", [](HttpRequest* req, HttpResponse* resp) {
        SKR_LOG_INFO(u8"圆头似乎要工作了");
        resp->content_type = TEXT_PLAIN;
        resp->FillContentType();
        resp->headers["Content-Type"] += "; charset=utf-8";
        resp->body = "我要管理!!!!!!!";
        return 200;
    });

    WebSocketServer server(&ws);
    server.setPort(9999);
    server.setThreadNum(4);
    server.service = &http;
    server.start();
    while (true)
    {
        if (first_channel)
        {
            first_channel->send("我们的架构非常优秀！！");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}
int _main_client()
{
    using namespace hv;

    WebSocketClient ws;
    ws.onopen = []() {
        SKR_LOG_INFO(u8"连接圆头");
    };
    ws.onmessage = [](const std::string& msg) {
        SKR_LOG_INFO(u8"收到圆头: %.*s", (int)msg.size(), msg.data());
    };
    ws.onclose = []() {
        SKR_LOG_INFO(u8"圆头似了");
    };

    // reconnect: 1,2,4,8,10,10,10...
    reconn_setting_t reconn;
    reconn_setting_init(&reconn);
    reconn.min_delay    = 1000;
    reconn.max_delay    = 10000;
    reconn.delay_policy = 2;
    ws.setReconnect(&reconn);

    ws.open("ws://127.0.0.1:9999/test");

    while (true)
    {
        if (!ws.isConnected()) break;
        ws.send("圆头，快工作");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

void WebSocketSampleCli::exec()
{
    if (kind == u8"client")
    {
        SKR_LOG_FMT_INFO(u8"圆头客户端开始运行");
        _main_client();
    }
    else if (kind == u8"server")
    {
        SKR_LOG_FMT_INFO(u8"圆头服务端开始运行");
        _main_server();
    }
    else
    {
        SKR_LOG_FMT_ERROR(u8"圆头不支持的类型：{}", kind);
    }
}
} // namespace skr