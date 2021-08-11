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
//typedef function<void(std::string&)> client_message_handler;
typedef function<void(char*)> client_message_handler;
typedef function<void()> client_accept_handler;
typedef function<size_t(char*, const system::error_code&, size_t)> client_read_comp_handler;
class asio_client {
    public:
    asio_client(){//{{{
        init_asio();
        //set_client_message_handler(bind(&asio_client::on_message, this, ::_1));
    }//}}}
    void init_asio(){//{{{
        _io_service = make_shared<asio::io_service>();
        _socket = make_shared<asio::ip::tcp::socket>(*_io_service);
        _acceptor = make_shared<asio::ip::tcp::acceptor>(ref(*_io_service));
        _signals = make_shared<asio::signal_set>(*_io_service, SIGINT, SIGTERM, SIGQUIT);
        _signals->async_wait(bind(&asio_client::_handle_stop, this));
    }//}}}
    void connect(const std::string& ip, const uint16_t port) {//{{{
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
        _acceptor->async_accept(ep, bind(&asio_client::_handle_accept, this, asio::placeholders::error));
        return _socket->connect(ep);
        //_acceptor->accept(*_socket);
    }//}}}
    void async_connect(const std::string& ip, const uint16_t port) {//{{{
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
        _acceptor->async_accept(ep, bind(&asio_client::_handle_accept, this, asio::placeholders::error));
        _socket->async_connect(ep, bind(&asio_client::_handle_connect, this, asio::placeholders::error));
    }//}}}
    void run(){//{{{
        _io_service->run();
    }//}}}
    void stop(){//{{{
        _io_service->stop();
    }//}}}
    void reset(){//{{{
        _io_service->reset();
    }//}}}
    void close(){//{{{
        _acceptor->close();
        _socket->close();
    }//}}}
    void set_client_message_handler(client_message_handler m_handler){//{{{
        _m_handler = m_handler;
    }//}}}
    void set_client_accept_handler(client_accept_handler accept_handler){//{{{
        _accept_handler = accept_handler;
    }//}}}
    void set_client_read_comp_handler(client_read_comp_handler rc_handler){//{{{
        _rc_handler = rc_handler;
    }//}}}
    asio::ip::tcp::socket& get_socket(){//{{{
        return *_socket;
    }//}}}

    //virtual void on_message(char*) = 0;
    //virtual size_t read_complete(char*, const system::error_code&, size_t) = 0;

    private:
    void _handle_connect(const system::error_code& err) {//{{{
        if (!err) {
            asio::async_read(*_socket, asio::buffer(_buffer, BUFFER_SIZE), 
                    bind(&asio_client::_read_complete, this, 
                        _buffer, asio::placeholders::error, 
                        asio::placeholders::bytes_transferred),
                    bind(&asio_client::_handle_read, this,
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
        }
        else {
        }
    }//}}}
    size_t _read_complete(char* buff, const system::error_code& err, size_t bytes) {//{{{
        if(!err && _rc_handler){
            return _rc_handler(buff, err, bytes);
        }
        return 0;
    }//}}}
    void _handle_accept(const system::error_code& error) {//{{{
        if(!error) {
            if(_accept_handler){
                _accept_handler();
            }
        }
    }//}}}
    void _handle_read(const system::error_code& error, size_t bytes_transferred) {//{{{
        if (!error) {
            if(_m_handler){
                _m_handler(_buffer);
            }
            asio::async_read(*_socket, asio::buffer(_buffer, BUFFER_SIZE), 
                    bind(&asio_client::_read_complete, this, 
                        _buffer, asio::placeholders::error, 
                        asio::placeholders::bytes_transferred),
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
                _acceptor->close(ec);
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
    shared_ptr<asio::ip::tcp::acceptor> _acceptor;
    shared_ptr<asio::signal_set> _signals;
    char _buffer[BUFFER_SIZE];
    client_message_handler   _m_handler;
    client_accept_handler    _accept_handler;
    client_read_comp_handler _rc_handler;
};
}
}

#endif
