#pragma once
#include <boost/beast/http.hpp>
#include "../../infra/db.hpp"

namespace http = boost::beast::http;

// Возвращает готовый HTTP-ответ
http::response<http::string_body> auth_register_handle(const http::request<http::string_body>& req, DbPool& pool);
