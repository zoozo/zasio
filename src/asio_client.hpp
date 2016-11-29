#ifndef ZOOZO_ZASIO_CLIENT_HPP
#define ZOOZO_ZASIO_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace boost;

namespace zoozo{
namespace zasio{
typedef function<void(std::string)> client_message_handler;
class asio_client {
    public:
    asio_client(){//{{{
        init_asio();
        set_client_message_handler(bind(&asio_client::on_message, this, ::_1));
    }//}}}
    void init_asio(){//{{{
        _io_service = make_shared<asio::io_service>();
        _socket = make_shared<asio::ip::tcp::socket>(*_io_service);
        _signals = make_shared<asio::signal_set>(*_io_service, SIGINT, SIGTERM, SIGQUIT);
        _signals->async_wait(bind(&asio_client::_handle_stop, this));
    }//}}}
    void connect(const std::string& ip, const uint16_t port) {//{{{
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
        _socket->async_connect(ep, bind(&asio_client::_handle_connect, this, asio::placeholders::error));
    }//}}}
    void run(){//{{{
        _io_service->run();
    }//}}}
    void set_client_message_handler(client_message_handler m_handler){//{{{
        _m_handler = m_handler;
    }//}}}
    virtual void on_message(std::string& message) = 0;

    private:
    void _handle_connect(const system::error_code& err) {//{{{
        if (!err) {
            asio::async_read(*_socket, _buffer, asio::transfer_at_least(1),
                    bind(&asio_client::_handle_read, this,
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
        }
        else {
            std::cout << "Error: " << err.message() << "\n";
        }
    }//}}}
    void _handle_read(const system::error_code& error, size_t bytes_transferred) {//{{{
        if (!error) {
            std::istream is(&_buffer);
            is >> _message;

            if(_m_handler){
                _m_handler(_message);
            }
            asio::async_read(*_socket, _buffer, asio::transfer_at_least(1),
                    bind(&asio_client::_handle_read, this,
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
        }
    }//}}}
    void _handle_stop(){//{{{
        system::error_code ec;
        try{
            if(_socket->is_open()){
                _socket->shutdown(asio::ip::tcp::socket::shutdown_both);
                _socket->close(ec);
            }
        }catch(std::exception &e){
            std::cout<<e.what()<<std::endl;
        }

        if(ec){
            std::cout<<"stop connection error"<<std::endl;
        }

        _io_service->stop();
    }//}}}

    shared_ptr<asio::io_service> _io_service;
    shared_ptr<asio::ip::tcp::socket> _socket;
    shared_ptr<asio::signal_set> _signals;
    asio::streambuf _buffer;
    std::string _message;
    client_message_handler _m_handler;
};
}
}

#endif
