#include "oauth.h"
// #include "helpers.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>

#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::async;
using namespace geode::utils;

namespace helpers {
    extern std::function<void(web::WebResponse)> webHandler;
}

size_t writeCallback(void* data, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)data, size * nmemb);
    return size * nmemb;
}

struct code_request {
    std::string code;
};

void oauth::serverThread() {

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    // listen socket
    auto lsock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(lsock, (sockaddr*)&addr, sizeof(addr));
    listen(lsock, 1);

    // client socket
    auto csock = accept(lsock, nullptr, nullptr);
    if (csock == INVALID_SOCKET) {
        closesocket(lsock);
        WSACleanup();
        return;
    }

    int timeout = 10000;
    setsockopt(csock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    char buffer[4096] = {};
    if (recv(csock, buffer, 4095, 0) <= 0) {
        closesocket(csock);
        closesocket(lsock);
        WSACleanup();
        return;
    }

    std::string request(buffer);
    size_t pos = request.find("GET /?code=");
    if (pos != std::string::npos) {

        auto start = pos + 11;
        auto end = request.find(' ', start);
        std::string oauth_code = request.substr(start, end - start);
        // geode::log("got code {} ", oauth_code);

        // todo: find a better way to do this
        matjson::Value json = matjson::Value::parse("{\"code\": \"" + oauth_code + "\"}").unwrap();

        static TaskHolder<web::WebResponse> listener;

        auto req = web::WebRequest();
        req.header("Content-Type", "application/json");
        req.bodyJSON(json);

        // geode::prelude::log::info("sending 12345");

        listener.spawn(
            req.post("http://localhost:3000/auth"),
            helpers::webHandler
        );

        // geode::prelude::log::info("sent");
        // listener.setFilter(req.post("http://localhost:3000/auth"));

    }

    auto response =
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<h2 style='font-family:sans-serif'>All set!</h2>"
        "<p style='font-family:sans-serif'>You can close this tab and go back to Geometry dash!</p>"
        "<script>window.close();</script>";
    send(csock, response, (int)strlen(response), 0);

    closesocket(csock);
    closesocket(lsock);
    WSACleanup();
}