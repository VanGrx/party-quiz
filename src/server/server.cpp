#include "server.h"
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <thread>

Server::Server() {}

Server::Server(char *argv[])
    : threads{std::max<int>(1, std::atoi(argv[4]))},
      address{net::ip::make_address(argv[1])}, port{static_cast<unsigned short>(
                                                   std::atoi(argv[2]))},
      doc_root{std::make_shared<std::string>(argv[3])}, ioc{threads} {

  // Create and launch a listening port
  connectionListener =
      std::make_shared<Listener>(ioc, tcp::endpoint{address, port}, doc_root);

  connectionListener->run();
}

void Server::run() {

  // Capture SIGINT and SIGTERM to perform a clean shutdown
  net::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&](beast::error_code const &, int) {
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.
    ioc.stop();
  });
  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([this] { ioc.run(); });
  ioc.run();

  // (If we get here, it means we got a SIGINT or SIGTERM)

  // Block until all the threads exit
  for (auto &t : v)
    t.join();
}
