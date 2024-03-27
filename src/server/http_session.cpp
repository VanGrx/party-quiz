#include "http_session.h"
#include "pages.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "utils.h"
#include "websocket_session.h"

namespace net = boost::asio; // from <boost/asio.hpp>

void HttpSession::do_close() {
  // Send a TCP shutdown
  beast::error_code ec;
  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

  // At this point the connection is closed gracefully
}

//-------------------------------------------------------------------------------------------------

void HttpSession::run() {
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.
  net::dispatch(
      stream_.get_executor(),
      beast::bind_front_handler(&HttpSession::do_read, shared_from_this()));
}

//-------------------------------------------------------------------------------------------------

void HttpSession::do_read() {
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  req_ = {};

  // Set the timeout.
  stream_.expires_after(std::chrono::seconds(30));

  // Read a request
  http::async_read(
      stream_, buffer_, req_,
      beast::bind_front_handler(&HttpSession::on_read, shared_from_this()));
}

//-------------------------------------------------------------------------------------------------

void HttpSession::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  // This means they closed the connection
  if (ec == http::error::end_of_stream)
    return do_close();

  if (ec)
    return fail(ec, "read");

  // See if it is a WebSocket Upgrade
  if (websocket::is_upgrade(req_)) {
    // Create a websocket session, transferring ownership
    // of both the socket and the HTTP request.
    auto newSession = std::make_shared<WebSocketSession>(
        stream_.release_socket(), callbackReceiver);
    callbackReceiver->webSocketConnected(newSession);
    newSession->do_accept(std::move(req_));
    return;
  }

  // Send the response
  handle_request(*doc_root_, std::move(req_), lambda_);
}

//-------------------------------------------------------------------------------------------------

void HttpSession::on_write(bool close, beast::error_code ec,
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

//-------------------------------------------------------------------------------------------------

template <class Body, class Allocator>
bool HttpSession::checkRequest(
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

//-------------------------------------------------------------------------------------------------

http::status HttpSession::parseBodyFromFile(const std::string path,
                                            http::file_body::value_type &body) {

  beast::error_code ec;

  // Attempt to open the file
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if (ec == beast::errc::no_such_file_or_directory)
    return http::status::not_found;

  // Handle an unknown error
  if (ec)
    return http::status::internal_server_error;

  return http::status::ok;
}

//-------------------------------------------------------------------------------------------------

std::string HttpSession::createPageRedirect(const int ID,
                                            const std::string &page) {
  rapidjson::Document d;

  d.SetObject();

  rapidjson::Value jsonPage(page.c_str(), page.size(), d.GetAllocator());

  d.AddMember("sessionID", ID, d.GetAllocator());
  d.AddMember("redirect", jsonPage, d.GetAllocator());

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType,
                    rapidjson::ASCII<>>
      writer(buffer);

  d.Accept(writer);
  return buffer.GetString();
}

//-------------------------------------------------------------------------------------------------

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void HttpSession::handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {

  if (!checkRequest(req))
    return send(createErrorResponse(std::move(req), http::status::bad_request,
                                    "Illegal request"));

  std::string targetResource = std::string(req.target());

  size_t pos = targetResource.find('?');
  targetResource = targetResource.substr(0, pos);

  const bool isScoreboardRequest =
      (req.target() == "/" || targetResource == pages::initPage ||
       targetResource == pages::scoreboardPage);
  const bool isPlayerRequest = (targetResource == pages::playerInitPage ||
                                targetResource == pages::playerPage);

  // Player called
  if (isPlayerRequest) {

    handlePlayerRequest(doc_root, std::move(req), std::move(send));

  } else if (isScoreboardRequest) {

    handleScoreboardRequest(doc_root, std::move(req), std::move(send));
  } else
    return returnRequestedPage(path_cat(doc_root, req.target()), std::move(req),
                               send);
}

//-------------------------------------------------------------------------------------------------

template <class Body, class Allocator, class Send>
void HttpSession::handlePlayerRequest(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {

  if (req.method() == http::verb::get) {

    // Parse the values given
    std::map<std::string, std::string> parsed_values =
        parseRequestTarget(std::string(req.target()));

    // Just give the page if no params are given
    if (parsed_values.empty())
      return returnRequestedPage(path_cat(doc_root, req.target()),
                                 std::move(req), send);

    if (parsed_values["status"] != "") {

      auto cookieList = parseBasicCookie(std::string(req[http::field::cookie]));

      if (cookieList["sessionID"] == "")
        return send(createErrorResponse(std::move(req),
                                        http::status::bad_request,
                                        "Cookie with session ID not found!"));

      int id = std::stoi(cookieList["sessionID"]);

      std::string message = callbackReceiver->getPlayerStatusJSONString(id);

      return returnRequestedJSON(message, std::move(req), send);
    }
  }

  // Respond to POST request
  if (req.method() == http::verb::post) {

    // Parse the values given
    std::map<std::string, std::string> parsed_values =
        parseRequestBody(req.body());

    bool isValid = parsed_values.size() == 2;
    bool isPlayerEnter =
        parsed_values["roomNumber"] != "" && parsed_values["username"] != "";
    bool isPlayerAnswerGiven =
        parsed_values["answer"] != "" && parsed_values["playerID"] != "";

    if (!isValid || (!isPlayerEnter && !isPlayerAnswerGiven)) {
      return send(createErrorResponse(std::move(req), http::status::bad_request,
                                      "Bad params given!"));
    }

    if (isPlayerAnswerGiven) {

      int answer = stoi(parsed_values["answer"]);

      auto cookieList = parseBasicCookie(std::string(req[http::field::cookie]));

      int playerID = stoi(cookieList["sessionID"]);

      callbackReceiver->answerGiven(playerID, answer);

      std::string message = "status:'OK'";
      return returnRequestedJSON(message, std::move(req), send);

    } else if (isPlayerEnter) {
      int roomNumber = stoi(parsed_values["roomNumber"]);
      std::string username = parsed_values["username"];

      int playerID = callbackReceiver->playerEntered(roomNumber, username);

      if (playerID == 0)
        return send(createErrorResponse(std::move(req),
                                        http::status::bad_request,
                                        "Illegal request. No such room!"));

      std::string message = createPageRedirect(playerID, pages::playerPage);

      return returnRequestedJSON(message, std::move(req), send);
    } else {
      return send(createErrorResponse(std::move(req), http::status::bad_request,
                                      "Bad params given!"));
    }
  }
}

//-------------------------------------------------------------------------------------------------

template <class Body, class Allocator, class Send>
void HttpSession::handleScoreboardRequest(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {

  if (req.method() == http::verb::get) {

    // Parse the values given
    std::map<std::string, std::string> parsed_values =
        parseRequestTarget(std::string(req.target()));

    // Just give the page if no params are given
    std::string target = std::string(req.target());
    if (target == "/")
      target = pages::initPage;

    if (parsed_values.empty())
      return returnRequestedPage(path_cat(doc_root, target), std::move(req),
                                 send);

    if (parsed_values["status"] != "") {

      auto cookieList = parseBasicCookie(std::string(req[http::field::cookie]));

      if (cookieList["sessionID"] == "")
        return send(createErrorResponse(std::move(req),
                                        http::status::bad_request,
                                        "Cookie with session ID not found!"));

      std::string message = callbackReceiver->getGameStatusJSONString();

      int id = stoi(cookieList["sessionID"]);

      if (!callbackReceiver->gameExists(id))
        return send(createErrorResponse(
            std::move(req), http::status::bad_request, "Game ID not found!"));

      return returnRequestedJSON(message, std::move(req), send);
    } else if (parsed_values["gameStart"] != "") {
      callbackReceiver->startGame();
      std::string message = "{}";
      return returnRequestedJSON(message, std::move(req), send);
    } else if (parsed_values["scores"] != "") {
      std::string message = callbackReceiver->getScoresJSONString();

      return returnRequestedJSON(message, std::move(req), send);
    }
  }

  // Respond to POST request
  if (req.method() == http::verb::post) {

    // Parse the values given
    std::map<std::string, std::string> parsed_values =
        parseRequestBody(req.body());

    int gameID = callbackReceiver->gameInitCallback();

    std::string message = createPageRedirect(gameID, pages::scoreboardPage);

    return returnRequestedJSON(message, std::move(req), send);
  }
}

//-------------------------------------------------------------------------------------------------

template <class Body, class Allocator>
http::response<http::string_body> HttpSession::createErrorResponse(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    http::status status, std::string what) {

  if (status == http::status::not_found) {
    what = "The resource '" + std::string(what) + "' was not found.";
  } else if (status == http::status::internal_server_error) {
    what = "An error occurred: '" + std::string(what) + "'";
  }

  http::response<http::string_body> res{status, req.version()};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "text/html");
  res.keep_alive(req.keep_alive());
  res.body() = std::string(what);
  res.prepare_payload();
  return res;
}

//-------------------------------------------------------------------------------------------------

template <class Body, class Allocator, class Send>
void HttpSession::returnRequestedPage(
    const std::string &path,
    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {

  http::file_body::value_type body;

  http::status status = parseBodyFromFile(path, body);

  switch (status) {
  case http::status::not_found:
    return send(
        createErrorResponse(std::move(req), status, std::string(req.target())));
  case http::status::internal_server_error:
    return send(
        createErrorResponse(std::move(req), status, "Internal server error"));
  default:
    break;
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

//-------------------------------------------------------------------------------------------------

template <class Body, class Allocator, class Send>
void HttpSession::returnRequestedJSON(
    const std::string &jsonString,
    http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {

  size_t size = jsonString.size();

  http::response<http::string_body> res{
      std::piecewise_construct, std::make_tuple(std::move(jsonString)),
      std::make_tuple(http::status::ok, req.version())};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, mime_type("path.json"));
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return send(std::move(res));
}
