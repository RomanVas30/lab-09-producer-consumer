#include <header.hpp>

#include <string>
#include <vector>
#include <src/gumbo.h>
#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace http = boost::beast::http;

void download(const std::string& host) {
    const std::string target = "/echo";
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    boost::asio::ip::tcp::socket socket(ioc);
    boost::asio::connect(socket, resolver.resolve(host, "80"));
    http::request<http::string_body> req(http::verb::get, target, 11);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    http::write(socket, req);
    {
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(socket, buffer, res);
        std::cout << res << std::endl;
    }
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    return;
}



int main() {
	download("yandex.ru");
}
