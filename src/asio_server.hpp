#ifndef ZOOZO_ZASIO_SERVER_HPP
#define ZOOZO_ZASIO_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <vector>
#include "connection_manager.hpp"

using namespace boost;

namespace zoozo{
namespace zasio{
    class asio_server{
        public:
        asio_server(){//{{{
            _connection_manager = make_shared<connection_manager>();
            init_asio();
            set_message_handler(boost::bind(&asio_server::on_message, this, ::_1, ::_2));
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
            _start_accept();
        }//}}}
        void run(){//{{{
            _io_service->run();
        }//}}}
        void run(std::size_t size){
            std::vector<boost::shared_ptr<boost::thread> > threads;

            for (std::size_t i = 0; i < size; ++i)
            {
                boost::shared_ptr<boost::thread> thread(new boost::thread(
                            boost::bind(&asio_server::run, this)));
                threads.push_back(thread);
            }

            // Wait for all threads in the pool to exit.
            for (std::size_t i = 0; i < threads.size(); ++i)
                threads[i]->join();
        }
        void set_message_handler(message_handler m_handler){//{{{
            _m_handler = m_handler;
        }//}}}
       connection_ptr get_conn_from_hdl(connection_hdl hdl){//{{{
           return hdl.lock();
       }//}}}
       virtual void on_message(connection_hdl conn_hdl, std::string& message) = 0;
        protected:
        void _start_accept() {//{{{
            connection_ptr conn = make_shared<connection>(_io_service);
            connection_hdl w(conn);
            conn->set_handle(w);
            conn->set_message_handler(_m_handler);
            _acceptor->async_accept(conn->get_socket(),
                    boost::bind(&asio_server::_handle_accept, this, conn,
                        asio::placeholders::error));
        }//}}}
        void _handle_accept(connection_ptr conn, const system::error_code& error) {//{{{
            if (!error) { _connection_manager->start(conn);
            }

            _start_accept();
        }//}}}
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

        protected:
        shared_ptr<asio::io_service> _io_service;
        shared_ptr<asio::ip::tcp::acceptor> _acceptor;
        shared_ptr<asio::signal_set> _signals;
        shared_ptr<connection_manager> _connection_manager;
        message_handler _m_handler;
    };
}
}

#endif
