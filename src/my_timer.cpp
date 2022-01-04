//
// Created by vincent on 2022/1
//
#include <iostream>
#include <chrono>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

void Print(boost::system::error_code ec, int* count, boost::asio::steady_timer* timer)
{
    if (*count < 3) {
        std::cout << "current count:" << *count << std::endl;
        (*count)++;
        timer->expires_after(std::chrono::seconds(1));
        timer->async_wait(std::bind(&Print, std::placeholders::_1, count, timer));
    }
}

int main()
{
    boost::asio::io_context ioContext;
    boost::asio::steady_timer steadyTimer{ioContext, std::chrono::seconds(1)};
    int count = 0;
    steadyTimer.async_wait(std::bind(&Print, std::placeholders::_1, &count, &steadyTimer));

    ioContext.run();
    std::cout << "Final count is " << count << std::endl;
    return 0;
}
