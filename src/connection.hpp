#ifndef ZOOZO_ZASIO_CONNECTION_HPP
#define ZOOZO_ZASIO_CONNECTION_HPP

#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost;

using boost::weak_ptr;
namespace zoozo{
namespace zasio{
    class connection;
    typedef weak_ptr<connection> connection_hdl;
    typedef shared_ptr<connection> connection_ptr;
    typedef boost::function<void(connection_hdl, std::string)> message_handler;
    typedef boost::function<void(connection_hdl)> disconnect_handler;
    typedef boost::function<void(connection_hdl)> close_handler;

    class connection : public enable_shared_from_this<connection>{
        public:
        connection(shared_ptr<asio::io_service> io_service){//{{{
            _socket = make_shared<asio::ip::tcp::socket>(*io_service);
        }//}}}
        void start(){//{{{
            /*
            asio::async_read(get_socket(), _buffer, asio::transfer_at_least(1),
                    bind(&connection::handle_read, shared_from_this(),
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
            */
            asio::async_read_until(get_socket(), _buffer, '\n',
                    bind(&connection::handle_read, shared_from_this(),
                        asio::placeholders::error));
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
        //void handle_read(const system::error_code& error, size_t bytes_transferred) {//{{{
        //    std::cout<<bytes_transferred<<std::endl;
        void handle_read(const system::error_code& error) {//{{{
            if(error == asio::error::eof){
                if(_close_handler){
                    _close_handler(_connection_hdl);
                }
                _disconn_handler(_connection_hdl);
            }
            else {
                std::istream is(&_buffer);
                //is >> _message;
                std::getline(is, _message);
                /*
                char c;
                _message = "";
                while(is.get(c)){
                    _message += c;
                }
                */
                if(_m_handler){
                    _m_handler(_connection_hdl, _message);
                }
                start();
            }
        }//}}}
        //}}}
        void handle_write_then_read(const system::error_code& error) {//{{{
            if (!error) {
                /*
                asio::async_read(get_socket(), _buffer, asio::transfer_at_least(1),
                        bind(&connection::handle_read, shared_from_this(),
                            asio::placeholders::error,
                            asio::placeholders::bytes_transferred));
                */
                //asio::async_read_until(get_socket(), _buffer, bind(&connection::match_new_line, shared_from_this()),
                asio::async_read_until(get_socket(), _buffer, '\n',
                        bind(&connection::handle_read, shared_from_this(),
                            asio::placeholders::error));
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
        std::string _message;
        asio::streambuf _buffer;
        connection_hdl  _connection_hdl;
    };
}
}

#endif
