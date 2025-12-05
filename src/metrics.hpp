#pragma once

#include <string>
#include <cstdlib>      // std::getenv
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

namespace metrics {

// Один внутренний helper: послать строку в StatsD (Telegraf)
inline void send_statsd(const std::string& line)
{
    // Хост и порт берём из env, как у тебя в контейнере:
    // STATSD_HOST=telegraf, STATSD_PORT=8125
    static std::string host = [] {
        if (const char* h = std::getenv("STATSD_HOST")) return std::string(h);
        return std::string("telegraf");
    }();

    static std::string port = [] {
        if (const char* p = std::getenv("STATSD_PORT")) return std::string(p);
        return std::string("8125");
    }();

    addrinfo hints{};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    addrinfo* res = nullptr;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0 || !res) {
        if (res) freeaddrinfo(res);
        return;
    }

    int sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        return;
    }

    // Нам не важно, дошёл или нет, главное не упасть
    ::sendto(sock, line.data(), line.size(), 0, res->ai_addr, res->ai_addrlen);
    ::close(sock);
    freeaddrinfo(res);
}

// Основной метод: вызвать из роутера после обработки запроса
inline void track_request(const std::string& route,
                          int status_code,
                          std::size_t body_size)
{
    // 1) счётчик по route+status
    std::string m1 = "auth_requests,route=" + route +
                     ",status=" + std::to_string(status_code) + ":1|c";
    send_statsd(m1);

    // 2) total по route
    std::string m2 = "auth_requests_total,route=" + route + ":1|c";
    send_statsd(m2);

    // 3) размер payload — как gauge, отдельный measurement
    //    auth_payload_bytes_v2,route=/auth/login,status=501:1234|g
    std::string m3 = "auth_payload_bytes_v2,route=" + route +
                     ",status=" + std::to_string(status_code) +
                     ":" + std::to_string(static_cast<long long>(body_size)) + "|g";
    send_statsd(m3);
}


} // namespace metrics
