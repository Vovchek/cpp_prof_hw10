#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <thread>

#include "bulk.h"

using boost::asio::ip::tcp;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    void start(CommandProcessor *cmd)
    {
        do_read(cmd);
    }

private:
    void do_read(CommandProcessor *cmd)
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                [this, self, cmd](boost::system::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        std::string inp{data_, length};
                        //std::cout << "session : " << this << std::endl;
                        for (auto start = inp.find_first_not_of("\n\r\0"),
                                    end = inp.find_first_of("\n\r\0", start);
                                start != std::string::npos;
                                start = inp.find_first_not_of("\n\r\0", end),
                                    end = inp.find_first_of("\n\r\0", start))
                        {
                            cmd->onInput(inp.substr(start, end != std::string::npos ? (end - start) : (inp.length() - start)));
                        }
                        do_read(cmd);
                    } else {
                        //cmd->terminate();
                    }
                });
    }

    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_context &io_context, short port, size_t bulk_size)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        pCommands_ = std::make_unique<CommandProcessor>(bulk_size);
        coutPtr_ = OstreamLogger::create(pCommands_.get());
        filePtr_ = FileLogger::create(pCommands_.get());
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<session>(std::move(socket))->start(pCommands_.get());
                }

                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    std::shared_ptr<OstreamLogger> coutPtr_; // = OstreamLogger::create(&commands);
    std::shared_ptr<FileLogger> filePtr_;    // = FileLogger::create(&commands);
public:
    static inline std::unique_ptr<CommandProcessor> pCommands_;
};
