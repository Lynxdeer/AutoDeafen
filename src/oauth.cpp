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

extern std::string CLIENT_ID;
extern std::string CLIENT_SECRET;

namespace helpers {
    extern std::function<void(web::WebResponse)> webHandler;
}

size_t writeCallback(void* data, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)data, size * nmemb);
    return size * nmemb;
}

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

        std::string params =
            "client_id=" + CLIENT_ID +
            "&client_secret=" + CLIENT_SECRET +
            "&grant_type=authorization_code"
            "&code=" + oauth_code +
            "&redirect_uri=http://localhost:8000";


        static TaskHolder<web::WebResponse> listener;

        auto req = web::WebRequest();
        req.header("Content-Type", "application/x-www-form-urlencoded");
        req.body(std::vector<uint8_t>(params.begin(), params.end()));

        geode::prelude::log::info("sending auth");

        // The code for where the backend points to is in the "backend" folder of this repository
        listener.spawn(
            req.post("https://discord.com/api/oauth2/token"),
            helpers::webHandler
        );

        geode::prelude::log::info("sent auth");
        // listener.setFilter(req.post("http://localhost:3000/auth"));

    }

    // todo: only make it say all set when you're actually all set
    auto response =
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<h2 style='font-family:sans-serif'>All set!</h2>"
        "<p style='font-family:sans-serif'>You can close this tab and go back to Geometry dash!</p>"
        "<script>window.close();</script>"; // todo this isnt working
    send(csock, response, (int)strlen(response), 0);

    closesocket(csock);
    closesocket(lsock);
    WSACleanup();
}