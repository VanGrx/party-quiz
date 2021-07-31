#ifndef SERVER_H
#define SERVER_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "listener.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class Server
{
public:
    Server();

    Server(char* argv[]);

    void run();

private:
    int threads;
    boost::asio::ip::address address;
    unsigned short port;
    std::shared_ptr<std::string> doc_root;
    std::shared_ptr<listener> connectionListener;
    net::io_context ioc;

};

#endif // SERVER_H
