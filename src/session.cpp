#include "session.h"
#include "pages.h"
#include "referee.h"

void Session::do_close() {
  // Send a TCP shutdown
  beast::error_code ec;
  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

  // At this point the connection is closed gracefully
}

void Session::run() {
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.
  net::dispatch(
      stream_.get_executor(),
      beast::bind_front_handler(&Session::do_read, shared_from_this()));
}

void Session::do_read() {
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  req_ = {};

  // Set the timeout.
  stream_.expires_after(std::chrono::seconds(30));

  // Read a request
  http::async_read(
      stream_, buffer_, req_,
      beast::bind_front_handler(&Session::on_read, shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  // This means they closed the connection
  if (ec == http::error::end_of_stream)
    return do_close();

  if (ec)
    return fail(ec, "read");

  // Send the response
  handle_request(*doc_root_, std::move(req_), lambda_);
}

void Session::on_write(bool close, beast::error_code ec,
                       std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "write");

  if (close) {
    // This means we should close the connection, usually because
    // the response indicated the "Connection: close" semantic.
    return do_close();
  }

  // We're done with the response so delete it
  res_ = nullptr;

  // Read another request
  do_read();
}

template <class Body, class Allocator>
bool Session::checkRequest(
    http::request<Body, http::basic_fields<Allocator>> &req) {
  // Make sure we can handle the method
  if (req.method() != http::verb::get && req.method() != http::verb::post)
    return false;

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return false;

  return true;
}

enum parseFromFileError
Session::parseBodyFromFile(const std::string path,
                           http::file_body::value_type &body) {

  beast::error_code ec;

  // Attempt to open the file
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if (ec == beast::errc::no_such_file_or_directory)
    return parseFromFileError::NOT_FOUND;

  // Handle an unknown error
  if (ec)
    return parseFromFileError::SERVER_ERROR;

  return parseFromFileError::OK;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void Session::handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {

  // Returns a bad request response
  auto const bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Returns a not found response
  auto const not_found = [&req](beast::string_view target) {
    http::response<http::string_body> res{http::status::not_found,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
  };

  // Returns a server error response
  auto const server_error = [&req](beast::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  if (!checkRequest(req))
    return send(bad_request("Illegal request"));

  http::file_body::value_type body;

  // Parse the values given
  std::map<std::string, std::string> parsed_values = parseBody(req.body());

  // Respond to GET request
  if (req.method() == http::verb::get) {

    // TODO: Select page user wants to open first
    std::string path = path_cat(doc_root, pages::initPage);

    parseFromFileError errorCode = parseBodyFromFile(path, body);

    switch (errorCode) {
    case parseFromFileError::OK:
      break;
    case parseFromFileError::NOT_FOUND:
      return send(not_found(req.target()));
    case parseFromFileError::SERVER_ERROR:
      return send(server_error("Internal server error"));
    }

    // Cache the size since we need it after the move
    auto const size = body.size();

    http::response<http::file_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
  }

  // Respond to POST request
  if (req.method() == http::verb::post) {
    // TODO: Get parameters and call the callback we need

    if (parsed_values.size() != 1) {
      // Error
    }

    int playerNumber = stoi(parsed_values["numberOfPlayers"]);

    actorMutex.lock();
    actor = std::static_pointer_cast<Actor>(
        std::make_shared<Referee>(playerNumber));
    actorMutex.unlock();

    callbackReceiver->gameInitCallback(playerNumber);

    std::string path = path_cat(doc_root, pages::scoreboardPage);

    parseFromFileError errorCode = parseBodyFromFile(path, body);

    switch (errorCode) {
    case parseFromFileError::OK:
      break;
    case parseFromFileError::NOT_FOUND:
      return send(not_found(req.target()));
    case parseFromFileError::SERVER_ERROR:
      return send(server_error("Internal server error"));
    }

    // Cache the size since we need it after the move
    auto const size = body.size();

    http::response<http::file_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
  }
}
