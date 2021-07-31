#ifndef UTILS_H
#define UTILS_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

namespace beast = boost::beast; // from <boost/beast.hpp>

constexpr unsigned int POINTS_FOR_CORRECT_ANSWER = 10;

enum parseFromFileError { OK = 0, NOT_FOUND, SERVER_ERROR };

void fail(beast::error_code ec, char const *what);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path);

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path);

std::string curlCollect(std::string url);

std::map<std::string, std::string> parseBody(const std::string &data);

static const std::string b =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; //=

class Base64 {
public:
  typedef unsigned char uchar;
  static std::string encode(const std::string &in) {
    std::string out;

    int val = 0, valb = -6;
    for (uchar c : in) {
      val = (val << 8) + c;
      valb += 8;
      while (valb >= 0) {
        out.push_back(b[(val >> valb) & 0x3F]);
        valb -= 6;
      }
    }
    if (valb > -6)
      out.push_back(b[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
      out.push_back('=');
    return out;
  }

  static std::string decode(const std::string &in) {

    std::string out;

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
      T[b[i]] = i;

    int val = 0, valb = -8;
    for (uchar c : in) {
      if (T[c] == -1)
        break;
      val = (val << 6) + T[c];
      valb += 6;
      if (valb >= 0) {
        out.push_back(char((val >> valb) & 0xFF));
        valb -= 8;
      }
    }
    return out;
  }
};

#endif // UTILS_H
