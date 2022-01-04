//
// Created by vincent on 2022/1
//
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

enum { BUF_SIZE = 1024 };

/*
 * tcp::socket 对象封装类, 异步的多个实例
 */
class Session: public std::enable_shared_from_this<Session> {
private:
    tcp::socket socket_;
    std::array<char, BUF_SIZE> buffer_;
    /* 暂不使用lambda定义handle */
    void DoRead() {
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                std::bind(&Session::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void OnRead(boost::system::error_code ec, size_t length) {
        if (!ec) {
            DoWrite();
            return;
        }

        if (ec == boost::asio::error::eof) {
            std::cerr << "Socket read EOF: " << ec.message() << std::endl;
        } else if (ec == boost::asio::error::operation_aborted) {
            // The socket of this connection has been closed.
            // This happens, e.g., when the server was stopped by a signal (Ctrl-C).
            std::cerr << "Socket conn abort: " << ec.message() << std::endl;
        } else {
            std::cerr << "Socket OnRead err msg: " << ec.message() << std::endl;
        }
    }

    void DoWrite() {
        socket_.async_write_some(boost::asio::buffer(buffer_),
                                 std::bind(&Session::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void OnWrite(boost::system::error_code ec, size_t length) {
        if (!ec) {
            DoRead();
        }
    }
public:
    Session(tcp::socket socket): socket_(std::move(socket)) {
        std::cout << "new Session" << std::endl;
    }

    void Start() {
        DoRead();
    }
};

/*
 * tcp::acceptor(类似steady_timer) 对象封装类
 */
class Server {
private:
    tcp::acceptor acceptor_;
    void DoAccept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket){
            if (!ec) {
                std::make_shared<Session>(std::move(socket))->Start();
            }
            DoAccept();
        });
    }
public:
    Server(boost::asio::io_context& ioContext, std::uint16_t port):
        acceptor_(ioContext, tcp::endpoint(tcp::v4(), port)) {
        DoAccept();
    }
};

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    std::uint16_t port = std::atoi(argv[1]);
    boost::asio::io_context ioContext;
    Server server{ioContext, port};
    ioContext.run();
    return 0;
}

