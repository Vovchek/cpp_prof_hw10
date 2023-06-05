#pragma once

/**
 * @file server.h
 * @brief Provides network server interface to command bulks processor.
 * @author Christopher M. Kohlhoff (adapted by Vladimir Chekal)
 * @date June 2023
 **/

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

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
                    cmd->onInput(inp);

                    do_read(cmd);
                }
                else
                {
                    // cmd->terminate();
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
    std::shared_ptr<OstreamLogger> coutPtr_;
    std::shared_ptr<FileLogger> filePtr_;
    static inline std::unique_ptr<CommandProcessor> pCommands_;
};
