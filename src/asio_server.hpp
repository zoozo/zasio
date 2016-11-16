#ifndef ZOOZO_ZASIO_SERVER_HPP
#define ZOOZO_ZASIO_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include "connection_manager.hpp"

using namespace boost;

namespace zoozo{
namespace zasio{
    class asio_server{
        public:
        asio_server(){//{{{
            _connection_manager = make_shared<connection_manager>();
            init_asio();
            set_message_handler(boost::bind(&asio_server::on_message, this, ::_1));
        }//}}}
        void init_asio(){//{{{
            //_io_service =  new asio::io_service();
            _io_service = make_shared<asio::io_service>();
            _acceptor = make_shared<asio::ip::tcp::acceptor>(ref(*_io_service));
            _signals = make_shared<asio::signal_set>(*_io_service, SIGINT, SIGTERM, SIGQUIT);
            _signals->async_wait(bind(&asio_server::_handle_stop, this));
        }//}}}
        void init(uint16_t port){//{{{
            _listen(port);
            start_accept();
        }//}}}
        void run(){
            _io_service->run();
        }
        void set_message_handler(message_handler m_handler){
            _m_handler = m_handler;
        }
        virtual void on_message(socket_ptr socket) = 0;
        private:
        void start_accept()
        {
            shared_ptr<connection> conn = make_shared<connection>(_io_service);
            conn->set_message_handler(_m_handler);
            _acceptor->async_accept(conn->get_socket(),
                    boost::bind(&asio_server::handle_accept, this, conn,
                        asio::placeholders::error));
        }

        void handle_accept(shared_ptr<connection> conn, const system::error_code& error)
        {
            if (!error)
            {
                _connection_manager->start(conn);
            }

            start_accept();
        }
        void _listen(uint16_t port) {//{{{
            asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port);
            _listen(ep);
        }//}}}
        void _listen(asio::ip::tcp::endpoint const & ep) {//{{{
            system::error_code ec;
            _acceptor->open(ep.protocol(), ec);
            if (!ec) {
                _acceptor->set_option(asio::socket_base::reuse_address(true), ec);
            }
            if (!ec) {
                _acceptor->bind(ep, ec);
            }
            if (!ec) {
                _acceptor->listen(asio::socket_base::max_connections, ec);
            }
            else
            {
                _handle_stop();
            }
        }//}}}
            void _handle_stop()//{{{
            {
                _acceptor->close();
                //_connection_manager->stop_all();
                _io_service->stop();
            }//}}}

        //asio::io_service* _io_service;
        private:
        shared_ptr<asio::io_service> _io_service;
        shared_ptr<asio::ip::tcp::acceptor> _acceptor;
        shared_ptr<asio::signal_set> _signals;
        shared_ptr<connection_manager> _connection_manager;
        message_handler _m_handler;
    };
}
}

#endif
