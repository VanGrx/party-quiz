#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "server.h"

namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

bool has_only_digits(const std::string &s) {
  return s.find_first_not_of("0123456789") == std::string::npos;
}

bool checkArguments(int argc, char *argv[]) {

  if (argc != 4)
    return false;

  if (!has_only_digits(argv[1]) || !has_only_digits(argv[3]))
    return false;

  return true;
}

std::string getIPAddress() {
  char buffer[1024];
  struct sockaddr_in serv;
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    return "";
  }

  const char *google_dns_server = "8.8.8.8";
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = inet_addr(google_dns_server);
  serv.sin_port = htons(53);

  if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
    close(sockfd);
    return "";
  }

  getsockname(sockfd, (struct sockaddr *)&name, &namelen);
  close(sockfd);

  const char *ip = inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
  if (ip == nullptr) {
    return "";
  }

  return std::string(ip);
}

int main(int argc, char *argv[]) {

  if (!checkArguments(argc, argv)) {
    std::cerr << "Usage: PartyQuiz <port> <doc_root> <threads>\n"
              << "Example:\n"
              << "   PartyQuiz 8080 . 1\n";
    return EXIT_FAILURE;
  }

  auto ip_address_str = getIPAddress();

  if (ip_address_str.empty()) {
    std::cerr << "Error connecting to the internet\n";
  }

  auto threads = std::max<int>(1, std::atoi(argv[3]));
  auto address = net::ip::make_address(ip_address_str);
  auto port = static_cast<unsigned short>(std::atoi(argv[1]));
  auto doc_root = std::string(argv[3]);

  std::shared_ptr<Server> server = std::make_shared<Server>(threads, doc_root);

  if (!server->init(tcp::endpoint{address, port})) {
    std::cerr << "Init failed!\n";
    return EXIT_FAILURE;
  }

  std::cout << "Listening on " << ip_address_str << "\n";

  server->start();

  return EXIT_SUCCESS;
}
