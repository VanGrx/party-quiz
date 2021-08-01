#include "server.h"
#include <thread>

Server::Server() {}

Server::Server(char *argv[])
    : threads{std::max<int>(1, std::atoi(argv[4]))},
      address{net::ip::make_address(argv[1])}, port{static_cast<unsigned short>(
                                                   std::atoi(argv[2]))},
      doc_root{std::make_shared<std::string>(argv[3])},
      ioc{std::max<int>(1, std::atoi(argv[4]))} {

  // Create and launch a listening port
  connectionListener =
      std::make_shared<Listener>(ioc, tcp::endpoint{address, port}, doc_root);

  connectionListener->run();
}

void Server::run() {

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([this] { ioc.run(); });
  ioc.run();
}
