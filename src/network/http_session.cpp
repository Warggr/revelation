// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_session.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include "visitor.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <variant>
#include <list>

#ifdef HTTP_CONTROLLED_SERVER
#include "launch_game.hpp"
#include "control/agent.hpp"
#include "setup/team.hpp"
#endif

using json = nlohmann::json;

class WwwDataVisitor : public WriterVisitor {
    const std::string _data;
    using KeyValueStore = std::list<std::pair<std::string_view, std::string_view>>;
    KeyValueStore parsed_data;
public:
    WwwDataVisitor(const std::string_view& data): _data(data){
        for(uint cursor = 0; cursor != _data.size() + 1; ){
            auto posEqual = _data.find('=', cursor);
            if(posEqual == std::string_view::npos)
                throw std::invalid_argument("Missing a =");
            std::string_view key( _data.data() + cursor, posEqual - cursor);
            cursor = posEqual + 1;
            auto posAnd = _data.find('&', posEqual+1);
            if(posAnd == std::string_view::npos)
                posAnd = _data.size();
            std::string_view value( _data.data() + cursor, posAnd - cursor);
            cursor = posAnd + 1;
            parsed_data.emplace_back(std::make_pair( key, value ));
        }
    }
    error_type visit(const std::string_view& key, std::string& value) override {
        auto iter = std::find_if(parsed_data.begin(), parsed_data.end(),
                  [&key](const std::pair<std::string_view, std::string_view> iter){ return iter.first == key; });
        if(iter == parsed_data.end()) return not_found;
        value = iter->second;
        parsed_data.erase(iter);
        return found;
    }
    error_type visit(const std::string_view& key, bool& value) override {
        std::string rawValue;
        auto result = visit(key, rawValue);
        value = result; // HTML checkboxes: no value provided when false, value provided when true
        return found;
    }
    error_type visit(const std::string_view& key, short& value) override {
        std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
        value = std::stoi(rawValue); // I know, this is inefficient and overflow-unsafe
        return found;
    }
    error_type visit(const std::string_view& key, unsigned char& value) override {
        std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
        value = std::stoi(rawValue); // I know, this is inefficient and overflow-unsafe
        return found;
    }
    error_type visit(const std::string_view& key, float& value) override {
        std::string rawValue; auto result = visit(key, rawValue); if(result == not_found) return not_found;
        value = std::stof(rawValue); // I know, this is inefficient and overflow-unsafe
        return found;
    }
    [[nodiscard]] bool empty() const { return parsed_data.empty(); }
    [[nodiscard]] std::string_view anyKey() const { return parsed_data.front().first; }
};

#ifdef HTTP_SERVE_FILES
std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);
beast::string_view mime_type(beast::string_view path);
#endif

HttpSession::HttpSession(tcp::socket&& socket, Server_impl& server)
    : socket_(std::move(socket))
    , server(server)
{
    std::cout << "(async listener) Creating HttpSession\n";
}

void HttpSession::run(){
    // Read a request
    http::async_read(socket_, buffer_, req_,
        [&](error_code ec, std::size_t bytes){
            on_read(ec, bytes);
        });
}

// Report a failure
void HttpSession::fail(error_code ec, char const* what){
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted) return;
    std::cerr << what << ": " << ec.message() << "\n";
}

//Writes the read values to @param roomId and @param agentId and returns a bool to indicate errors, C-style.
bool read_request_path(boost::string_view& str, unsigned short& retVal){
    unsigned int iter = 0;
    if(str[iter++] != '/') return false;
    retVal = 0;
    do{
        char digit = str[iter++] - '0';
        if(0 > digit or digit > 9) return false;
        retVal = 10*retVal + digit;
    } while(iter < str.size() and str[iter] != '/');
    str = str.substr(iter);
    return true;
}

template<typename HttpBodyType>
void HttpSession::sendResponse(http::response<HttpBodyType>&& res){
    std::cout << "(http) " << res.result_int() << ' ' << req_.method_string() << ' '
        << req_.target() << ' ' << res.payload_size().get() << "B\n";
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::access_control_allow_origin, "*");
    res.keep_alive(req_.keep_alive());

    res.prepare_payload();
    // The lifetime of the message has to extend
    // for the duration of the async operation so
    // we use a shared_ptr to manage it.
    using response_type = typename std::decay<decltype(res)>::type;
    auto sp = std::make_shared<response_type>(std::forward<decltype(res)>(res));

    // Write the response
    http::async_write(this->socket_, *sp,
                      [&, sp](error_code ec, std::size_t bytes){ on_write(ec, bytes, sp->need_eof()); });
}

void HttpSession::on_read(error_code ec, std::size_t){
    // This means they closed the connection
    if(ec == http::error::end_of_stream){
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        server.askForHttpSessionDeletion(this);
        return;
    }

    // Handle the error, if any
    if(ec){
        server.askForHttpSessionDeletion(this);
        return fail(ec, "read");
    }

    // Returns a bad request response
    auto const bad_request = [&](boost::beast::string_view why){
        http::response<http::string_body> res{http::status::bad_request, req_.version()};
        res.set(http::field::content_type, "text/html");
        res.body() = why.to_string();
        return res;
    };

    // Returns a not found response
    auto const not_found = [&](boost::beast::string_view target){
        http::response<http::string_body> res{http::status::not_found, req_.version()};
        res.set(http::field::content_type, "text/html");
        res.body() = "The resource '" + target.to_string() + "' was not found.";
        return res;
    };

    [[ maybe_unused ]] auto const redirect = [&](boost::beast::string_view location){
        http::response<http::string_body> res{http::status::moved_permanently, req_.version()};
        res.set(http::field::location, location);
        return res;
    };

    auto const jsonResponse = [&](const json& j){
        http::response<http::string_body> res{ http::status::ok, req_.version() };
        res.set(http::field::content_type, "application/json");
        res.body() = j.dump();
        return res;
    };

    [[ maybe_unused ]] auto const server_error = [&](boost::beast::string_view what){
        http::response<http::string_body> res{http::status::internal_server_error, req_.version()};
        res.set(http::field::content_type, "text/html");
        res.body() = "An error occurred: '" + what.to_string() + "'";
        return res;
    };

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(req_)){
        std::cout << "(async http) websocket upgrade heard!\n";
        // The websocket connection is established! Transfer the socket and the request to the server
        auto request_path = req_.target();
        RoomId roomId; AgentId agentId;
        if(not read_request_path(request_path, roomId)
        or not read_request_path(request_path, agentId))
            return sendResponse(bad_request("Wrong path"));

        if(server.getRooms().find(roomId) == server.getRooms().end())
            return sendResponse(not_found("Room not found"));

        ServerRoom& room = server.getRooms().find(roomId)->second;
        auto spec = room.addSpectator(socket_, agentId);
        if (!spec)
            return sendResponse(bad_request("Room did not accept you"));

        spec->connect(std::move(req_));
        server.askForHttpSessionDeletion(this); //don't schedule any further network operations, delete this, and die.
        return;
    } else if(req_.method() == boost::beast::http::verb::options) {
        //see https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS
        http::response<http::string_body> res{http::status::no_content, req_.version()};
        res.set(http::field::access_control_allow_methods, "POST, GET, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type");
        res.set(http::field::access_control_max_age, "86400");
        return sendResponse(std::move(res));
    }
#define ADD_ENDPOINT(endpoint, thismethod) else if(req_.target() == endpoint and req_.method() == boost::beast::http::verb::thismethod)
#ifdef HTTP_CONTROLLED_SERVER
    ADD_ENDPOINT("/room/grammar", get){
        http::response<http::string_body> res{ http::status::ok, req_.version() };
        res.set(http::field::content_type, "application/json");
        res.body() = grammar_as_json_string;
        return sendResponse(std::move(res));
    }
    ADD_ENDPOINT("/room", post){
        AgentDescription agentsDescr;
        if(req_.body().empty()){
            agentsDescr = { AgentDescriptor::NETWORK, AgentDescriptor::NETWORK };
        } else {
            auto encoding = req_[http::field::content_type];
            if(encoding != "application/json")
                return sendResponse(bad_request("Need JSON description of agents"));
            try{
                json agentDescrJson = json::parse(req_.body());
                agentsDescr = parseAgents(agentDescrJson);
            } catch(json::parse_error& err) {
                return sendResponse(bad_request(err.what()));
            } catch(AgentCreationException& err){
                return sendResponse(bad_request(err.what()));
            }
        }
        auto [roomId, room] = server.addRoom();
        room.launchGame(roomId, std::move(agentsDescr));

        http::response<http::string_body> res{ http::status::created, req_.version() };
        res.set(http::field::content_type, "text/plain");
        res.body() = std::to_string(roomId);
        return sendResponse(std::move(res));
    }
    ADD_ENDPOINT("/unit", post){
        auto encoding = req_[http::field::content_type];
        if(encoding != "application/x-www-form-urlencoded")
            return sendResponse(bad_request("Form should be urlencoded"));
        try{
            WwwDataVisitor visitor(req_.body());
            ImmutableCharacter chr(visitor);
            if(not visitor.empty())
                throw std::invalid_argument(std::string("Extra value: ").append(visitor.anyKey()));
            if(server.repo.addCharacter(std::move(chr)))
                return sendResponse( http::response<http::string_body>( http::status::ok, req_.version() ) );
            else
                return sendResponse( http::response<http::string_body>( http::status::conflict, req_.version() ) );
        } catch(const std::invalid_argument& err) {
            return sendResponse(bad_request(err.what()));
        }
    }
    ADD_ENDPOINT("/team", post){
        auto encoding = req_[http::field::content_type];
        if(encoding != "application/x-www-form-urlencoded")
            return sendResponse(bad_request("Form should be urlencoded"));
        try{
            WwwDataVisitor visitor(req_.body());
            std::array<CharacterId, 6> characters;
            char buffer[] = "0";
            std::string noCharacter = "";
            for(unsigned char i = 0; i<characters.size(); i++){
                buffer[0] = '0' + i;
                characters[i] = visitor.get(buffer, &noCharacter);
            }
            std::string teamName = visitor.get("name", (std::string*)nullptr);
            if(not visitor.empty())
                throw std::invalid_argument(std::string("Extra value: ").append(visitor.anyKey()));
            if(server.repo.createTeam(characters, teamName))
                return sendResponse( http::response<http::string_body>( http::status::ok, req_.version() ) );
            else
                return sendResponse( http::response<http::string_body>( http::status::conflict, req_.version() ) );
        } catch(const std::invalid_argument& err){
            return sendResponse(bad_request(err.what()));
        }
    }
    ADD_ENDPOINT("/team/random", post){
        Generator seed = getRandom();
        server.repo.mkRandom(seed);
        return sendResponse( http::response<http::string_body>( http::status::ok, req_.version() ) );;
    }
    ADD_ENDPOINT("/team/", get){
        json characters = json::array();
        for(const auto& [ name, character ] : server.repo.getCharacters()){
            characters.push_back(character);
        }
        json teams = json::array();
        for(const auto& [ name, team ] : server.repo.getTeams() ){
            teams.push_back(team);
        }
        json j = { {"teams", teams}, {"characters", characters} };
        return sendResponse(jsonResponse(j));
    }
#endif
    ADD_ENDPOINT("/room/", get){
        json j = json::array();
        for(const auto& [ roomId, room ] : server.getRooms()){
            json agents = json::array({ "Unavailable", "Unavailable" });
            unsigned nbSpectators = 0;
            for(const auto& [ agentId, agent ] : room.getSessions()){
                agents[agentId-1] = (
                        agent->isConnected() ? std::string("Connected") :
                        agent->isClaimed() ? std::string("claimed") :
                        std::string("free"));
            }
            for(const auto& spectator: room.getSpectators()){
                if(spectator->id != 0)
                    agents[spectator->id-1] = (spectator->isConnected() ? std::string("connected") : std::string("disconnected"));
                else nbSpectators++;
            }
            j.push_back(json({
                {"id", roomId},
                {"agents", agents},
                {"spectators", nbSpectators}
            }));
        }
        return sendResponse(jsonResponse(j));
    }
#ifdef HTTP_SERVE_FILES
    const char doc_api_path[] = "/files";
    constexpr unsigned int doc_api_path_len = sizeof(doc_api_path) / sizeof(char) - 1;
    if(req_.method() == http::verb::get or req_.method() == http::verb::head){
        if(boost::beast::iequals(req_.target(), "/"))
            return sendResponse(redirect("/files/"));

        boost::string_view req_path;

        if(boost::beast::iequals(doc_api_path, req_.target().substr(0, doc_api_path_len))){
            req_path = req_.target().substr(doc_api_path_len);
            auto const posParams = req_path.rfind("?");
            if(posParams != beast::string_view::npos) req_path = req_path.substr(0, posParams);

            // Request path must be absolute and not contain ".."
            if(req_path.empty() or req_path[0] != '/' or req_path.find("..") != boost::beast::string_view::npos)
                return sendResponse(bad_request("Illegal request-target"));
        }
        else if(boost::beast::iequals(req_.target(), "/favicon.ico")) req_path = "/favicon.ico";
        else return sendResponse(bad_request("File not found"));

        auto path = path_cat(server.doc_root, req_path);
        if(req_path.back() == '/') path.append("index.html");
        // Attempt to open the file
        boost::beast::error_code error;
        http::file_body::value_type body;
        body.open(path.c_str(), boost::beast::file_mode::scan, error);

        // File doesn't exist
        if(error == boost::system::errc::no_such_file_or_directory) return sendResponse(not_found(req_.target()));
        // Unknown error
        if(error) return sendResponse(server_error(error.message()));

        // Cache the size since we need it after the move
        auto const size = body.size();

        // Respond to HEAD request
        if(req_.method() == http::verb::head){
            http::response<http::empty_body> res{http::status::ok, req_.version()};
            res.set(http::field::content_type, mime_type(path));
            res.content_length(size);
            return sendResponse(std::move(res));
        } else { // Respond to GET request
            http::response<http::file_body> res{
                    std::piecewise_construct,
                    std::make_tuple(std::move(body)),
                    std::make_tuple(http::status::ok, req_.version())};
            res.set(http::field::content_type, mime_type(path));
            res.content_length(size);
            return sendResponse(std::move(res));
        }
    }
#endif
    return sendResponse(bad_request("Not found"));
}

void HttpSession::on_write(error_code ec, std::size_t, bool close){
    // Handle the error, if any
    if(ec) return fail(ec, "write");

    if(close){
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        server.askForHttpSessionDeletion(this);
        return;
    }

    // Clear contents of the request message, otherwise the read behavior is undefined.
    req_ = {};

    // Read another request
    http::async_read(socket_, buffer_, req_,
        [&](error_code ec, std::size_t bytes){ on_read(ec, bytes); });
}

#ifdef HTTP_SERVE_FILES
// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path){
    using boost::beast::iequals;
    auto const pos = path.rfind(".");
    if(pos == beast::string_view::npos) return "application/text";
    const beast::string_view ext = path.substr(pos+1);
    if(iequals(ext, "htm"))  return "text/html";
    if(iequals(ext, "html")) return "text/html";
    if(iequals(ext, "php"))  return "text/html";
    if(iequals(ext, "css"))  return "text/css";
    if(iequals(ext, "txt"))  return "text/plain";
    if(iequals(ext, "js"))   return "application/javascript";
    if(iequals(ext, "json")) return "application/json";
    if(iequals(ext, "xml"))  return "application/xml";
    if(iequals(ext, "swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, "flv"))  return "video/x-flv";
    if(iequals(ext, "png"))  return "image/png";
    if(iequals(ext, "jpe"))  return "image/jpeg";
    if(iequals(ext, "jpeg")) return "image/jpeg";
    if(iequals(ext, "jpg"))  return "image/jpeg";
    if(iequals(ext, "gif"))  return "image/gif";
    if(iequals(ext, "bmp"))  return "image/bmp";
    if(iequals(ext, "ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, "tiff")) return "image/tiff";
    if(iequals(ext, "tif"))  return "image/tiff";
    if(iequals(ext, "svg"))  return "image/svg+xml";
    if(iequals(ext, "svgz")) return "image/svg+xml";
    return "application/text";
}
#endif

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(boost::beast::string_view base, boost::beast::string_view path){
    if(base.empty()) return path.to_string();
    std::string result = base.to_string();
#if BOOST_MSVC
    constexpr char path_separator = '\\';
    if(result.back() == path_separator) result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    constexpr char path_separator = '/';
    if(result.back() == path_separator) result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}
