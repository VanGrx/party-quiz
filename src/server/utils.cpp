#include "utils.h"
#include <iostream>
void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path) {
  if (base.empty())
    return std::string(path);
  std::string result(base);
#ifdef BOOST_MSVC
  char constexpr path_separator = '\\';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
  for (auto &c : result)
    if (c == '/')
      c = path_separator;
#else
  char constexpr path_separator = '/';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
#endif
  return result;
}

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path) {
  using beast::iequals;
  auto const ext = [&path] {
    auto const pos = path.rfind(".");
    if (pos == beast::string_view::npos)
      return beast::string_view{};
    return path.substr(pos);
  }();
  if (iequals(ext, ".htm"))
    return "text/html";
  if (iequals(ext, ".html"))
    return "text/html";
  if (iequals(ext, ".php"))
    return "text/html";
  if (iequals(ext, ".css"))
    return "text/css";
  if (iequals(ext, ".txt"))
    return "text/plain";
  if (iequals(ext, ".js"))
    return "application/javascript";
  if (iequals(ext, ".json"))
    return "application/json";
  if (iequals(ext, ".xml"))
    return "application/xml";
  if (iequals(ext, ".swf"))
    return "application/x-shockwave-flash";
  if (iequals(ext, ".flv"))
    return "video/x-flv";
  if (iequals(ext, ".png"))
    return "image/png";
  if (iequals(ext, ".jpe"))
    return "image/jpeg";
  if (iequals(ext, ".jpeg"))
    return "image/jpeg";
  if (iequals(ext, ".jpg"))
    return "image/jpeg";
  if (iequals(ext, ".gif"))
    return "image/gif";
  if (iequals(ext, ".bmp"))
    return "image/bmp";
  if (iequals(ext, ".ico"))
    return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff"))
    return "image/tiff";
  if (iequals(ext, ".tif"))
    return "image/tiff";
  if (iequals(ext, ".svg"))
    return "image/svg+xml";
  if (iequals(ext, ".svgz"))
    return "image/svg+xml";
  return "application/text";
}

std::map<std::string, std::string> parseRequestTarget(const std::string &data) {

  std::string requestString = data;
  size_t pos;
  std::map<std::string, std::string> parsed_values;

  if ((pos = requestString.find('?')) != std::string::npos) {
    requestString = requestString.substr(pos + 1);
    parsed_values = parseRequestBody(requestString);
  }

  return parsed_values;
}

std::map<std::string, std::string> parseRequestBody(const std::string &data) {
  enum class States { Name, Ignore, Value };

  std::map<std::string, std::string> parsed_values;
  std::string name;
  std::string value;

  States state = States::Name;
  for (char c : data) {

    switch (state) {
    case States::Name:
      if (c != '=') {
        name += c;
      } else {
        state = States::Ignore;
      }
      break;
    case States::Ignore:
      if (c != '=') {
        state = States::Value;
        value += c;
      }
      break;
    case States::Value:
      if (c != '&') {
        value += c;
      } else {
        parsed_values.insert(std::make_pair(name, value));
        name = "";
        value = "";
        state = States::Name;
      }
      break;
    }
  }
  if (value != "")
    parsed_values.insert(std::make_pair(name, value));
  return parsed_values;
}

std::map<std::string, std::string> parseBasicCookie(const std::string &data) {
  enum class States { Name, Ignore, Value, WhiteSpace };

  std::map<std::string, std::string> parsed_values;
  std::string name;
  std::string value;

  States state = States::Name;
  for (char c : data) {

    switch (state) {
    case States::Name:
      if (c != '=') {
        name += c;
      } else {
        state = States::Ignore;
      }
      break;
    case States::Ignore:
      if (c != '=') {
        state = States::Value;
        value += c;
      }
      break;
    case States::Value:
      if (c != ';') {
        value += c;
      } else {
        parsed_values.insert(std::make_pair(name, value));
        name = "";
        value = "";
        state = States::WhiteSpace;
      }
      break;
    case States::WhiteSpace:
      if (c != ' ') {
        state = States::Name;
      }
      break;
    }
  }
  if (value != "")
    parsed_values.insert(std::make_pair(name, value));
  return parsed_values;
}
