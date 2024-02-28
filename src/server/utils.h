#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <map>

namespace beast = boost::beast; // from <boost/beast.hpp>

void fail(beast::error_code ec, char const *what);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path);

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path);

std::map<std::string, std::string> parseRequestTarget(const std::string &data);
std::map<std::string, std::string> parseRequestBody(const std::string &data);
std::map<std::string, std::string> parseBasicCookie(const std::string &data);

// ---------------------------------------------------------------------------------------

#endif // SERVER_UTILS_H
