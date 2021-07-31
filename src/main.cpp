#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "server.h"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

bool has_only_digits(const std::string &s) {
  return s.find_first_not_of("0123456789") == std::string::npos;
}

bool checkArguments(int argc, char *argv[]) {

  if (argc != 5)
    return false;

  if (!has_only_digits(argv[2]) || !has_only_digits(argv[4]))
    return false;

  try {
    net::ip::make_address(argv[1]);
  } catch (const std::exception &) {
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {

  if (!checkArguments(argc, argv)) {
    std::cerr
        << "Usage: http-server-async <address> <port> <doc_root> <threads>\n"
        << "Example:\n"
        << "    http-server-async 0.0.0.0 8080 . 1\n";
    return EXIT_FAILURE;
  }

  Server server(argv);

  server.run();

  return EXIT_SUCCESS;
}
