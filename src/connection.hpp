#ifndef ZOOZO_ZASIO_CONNECTION_HPP
#define ZOOZO_ZASIO_CONNECTION_HPP

#include <boost/function.hpp>

using namespace boost;

namespace zoozo{
namespace zasio{
    typedef shared_ptr<asio::ip::tcp::socket> socket_ptr;
    typedef boost::function<void(socket_ptr)> message_handler;

    class connection{
        public:
        connection(shared_ptr<asio::io_service> io_service){//{{{
            _socket = make_shared<asio::ip::tcp::socket>(*io_service);
        }//}}}
        void start(){//{{{
            _socket->async_read_some(boost::asio::buffer(data_, max_length),
                    boost::bind(&connection::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
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
        asio::ip::tcp::socket& get_socket(){//{{{
            return *_socket;
        }//}}}
        void set_message_handler(message_handler m_handler){//{{{
            _m_handler = m_handler;
        }//}}}
        private:
        void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {//{{{
            if (!error)
            {
                std::string str = "sss";

                   if(_m_handler){
                   _m_handler(_socket);
                   }
                boost::asio::async_write(*_socket,
                        boost::asio::buffer(str),
                        boost::bind(&connection::handle_write, this,
                            boost::asio::placeholders::error));
            }
        }//}}}
        void handle_write(const boost::system::error_code& error) {//{{{
            if (!error) {
                _socket->async_read_some(boost::asio::buffer(data_, max_length),
                        boost::bind(&connection::handle_read, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            }
        }//}}}

        shared_ptr<asio::ip::tcp::socket> _socket;
        enum { max_length = 1024 };
        char data_[max_length];
        message_handler _m_handler;
    };
    typedef shared_ptr<connection> connection_ptr;
}
}

#endif
