#ifndef ZOOZO_ZASIO_CONNECTION_HPP
#define ZOOZO_ZASIO_CONNECTION_HPP

#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <array>

using namespace boost;

using boost::weak_ptr;
#define MAX_LEN 1024 

namespace zoozo{
namespace zasio{
    class connection;
    typedef weak_ptr<connection> connection_hdl;
    typedef shared_ptr<connection> connection_ptr;
    typedef boost::function<void(connection_hdl, shared_ptr<std::string>, system::error_code)> message_handler;
    typedef boost::function<void(connection_hdl)> disconnect_handler;
    typedef boost::function<void(connection_hdl)> close_handler;

    class connection : public enable_shared_from_this<connection>{
        public:
        connection(shared_ptr<asio::io_service> io_service){//{{{
            _socket = make_shared<asio::ip::tcp::socket>(*io_service);
            _message = boost::make_shared<std::string>();
        }//}}}
        void start(){//{{{
            /*
            get_socket().async_read_some( asio::buffer(_buffer, MAX_LEN),
                    bind(&connection::handle_read, shared_from_this(),
                        asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            */
            asio::async_read(get_socket(), asio::buffer(_buffer), 
                    bind(&connection::read_complete, shared_from_this(),
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred),
                    bind(&connection::handle_read, shared_from_this(),
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
            /*
            asio::async_read_until(get_socket(), _buffer, '\n',
                    bind(&connection::handle_read, shared_from_this(),
                        asio::placeholders::error));
            */
        }//}}}
        void stop(){//{{{
            boost::system::error_code ec;
            try{
                if(_socket->is_open()){
                    _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    _socket->close(ec);
                }
            }catch(std::exception &e){
                std::cout<<e.what()<<std::endl;
            }

            if(ec){
                std::cout<<"stop connection error"<<std::endl;
            }

        }//}}}
       void set_handle(connection_hdl conn_hdl){//{{{
           _connection_hdl = conn_hdl;
       }//}}}
        asio::ip::tcp::socket& get_socket(){//{{{
            return *_socket;
        }//}}}
        void set_message_handler(message_handler m_handler){//{{{
            _m_handler = m_handler;
        }//}}}
        void set_disconnect_handler(disconnect_handler disconn_handler){//{{{
            _disconn_handler = disconn_handler;
        }//}}}
        void set_close_handler(close_handler c_handler){//{{{
            _close_handler = c_handler;
        }//}}}
        void send(const std::string& message){//{{{
            asio::async_write(get_socket(),
                    asio::buffer(message),
                    bind(&connection::handle_write, shared_from_this(),
                        asio::placeholders::error));
        }//}}}
        void send_then_read(const std::string& message){//{{{
            asio::async_write(get_socket(),
                    asio::buffer(message),
                    bind(&connection::handle_write_then_read, shared_from_this(),
                        asio::placeholders::error));
        }//}}}
        private:
        size_t read_complete(const system::error_code & err, size_t bytes) {
            if ( err) return 0;
            //size_t already_read_ = bytes;
            if(_total_size == 0 && bytes > 12 && _buffer[bytes - 1] == 0x01){
                try{
                    _total_size = std::stoi(&_buffer[12]) + 7 + bytes;
                }
                catch(std::exception& e){
                    _ec = system::errc::make_error_code(system::errc::protocol_error);
                    return 0;
                }
            }
            if(_total_size > 0 && bytes == _total_size) return 0;
            return 1;
        }
        void handle_read(const system::error_code& error, size_t bytes_transferred) {//{{{
            _total_size = 0;
            *_message = "";
       // void handle_read(const system::error_code& error) {//{{{
            if(error == asio::error::eof){
                if(_close_handler){
                    _close_handler(_connection_hdl);
                }
                _disconn_handler(_connection_hdl);
                *_message = "";
            }
            else {

/*
                std::ostringstream out;
                out<<&_buffer;
                *_message=out.str();
*/
                *_message = _buffer;
                memset(_buffer, 0, sizeof _buffer);
                if(_m_handler){
                    _m_handler(_connection_hdl, _message, _ec);
                }
                _ec = system::errc::make_error_code(system::errc::success);
                start();
            }
        }//}}}
        //}}}
        void handle_write_then_read(const system::error_code& error) {//{{{
            if (!error) {
                /*
            get_socket().async_read_some( asio::buffer(_buffer, MAX_LEN),
                    bind(&connection::handle_read, shared_from_this(),
                        asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
                */
                asio::async_read(get_socket(), asio::buffer(_buffer),
                        bind(&connection::read_complete, shared_from_this(),
                            asio::placeholders::error,
                            asio::placeholders::bytes_transferred),
                        bind(&connection::handle_read, shared_from_this(),
                            asio::placeholders::error,
                            asio::placeholders::bytes_transferred));
                //asio::async_read_until(get_socket(), _buffer, bind(&connection::match_new_line, shared_from_this()),
                /*
                asio::async_read_until(get_socket(), _buffer, '\n',
                        bind(&connection::handle_read, shared_from_this(),
                            asio::placeholders::error));
                */
            }
        }//}}}
        void handle_write(const system::error_code& error) {//{{{
            if (!error) {
            }
        }//}}}

        shared_ptr<asio::ip::tcp::socket> _socket;
        message_handler _m_handler;
        disconnect_handler _disconn_handler;
        close_handler _close_handler;
        shared_ptr<std::string> _message;
        //std::string _message;
        //asio::streambuf _buffer;
        char _buffer[MAX_LEN];
        int _read_bytes;
        size_t _total_size;
        system::error_code _ec;
        connection_hdl  _connection_hdl;
    };
}
}

#endif
