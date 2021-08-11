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
#include "exception.hpp"
//#include "logger.hpp"

using namespace boost;

namespace zasio{
    class asio_server{
        public:
        asio_server():_status(1){//{{{
            _connection_manager = make_shared<connection_manager>();
            init_asio();
            set_disconnect_handler(boost::bind(&asio_server::on_disconnect, this, ::_1));
        }//}}}
        void init_asio(){//{{{
            _io_service = make_shared<asio::io_service>();
            _acceptor = make_shared<asio::ip::tcp::acceptor>(ref(*_io_service));
 //           _signals = make_shared<asio::signal_set>(*_io_service, SIGINT, SIGTERM, SIGQUIT);
 //           _signals->async_wait(bind(&asio_server::_handle_stop, this));
        }//}}}
        void init(uint16_t port){//{{{
            _listen(port);
            _start_accept();
        }//}}}
        void init(const std::string& ip, uint16_t port){//{{{
            _listen(ip, port);
            _start_accept();
        }//}}}
        void run(){//{{{
           // _logger->write(trivial::trace, "asio server run.");
            _io_service->run();
        }//}}}
        void run(std::size_t size){//{{{
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
        }//}}}
        void set_accept_handler(accept_handler handler){//{{{
            _accept_handler = handler;
        }//}}}
        void set_message_handler(message_handler m_handler){//{{{
            _m_handler = m_handler;
        }//}}}
        void set_read_comp_handler(read_comp_handler rc_handler){//{{{
            _rc_handler = rc_handler;
        }//}}}
        void set_disconnecting_handler(disconnecting_handler disconning_handler){//{{{
            _disconning_handler = disconning_handler;
        }//}}}
        void set_disconnect_handler(disconnect_handler disconn_handler){//{{{
            _disconn_handler = disconn_handler;
        }//}}}
        void set_close_handler(close_handler c_handler){//{{{
            _close_handler = c_handler;
        }//}}}
       connection_ptr get_conn_from_hdl(connection_hdl hdl){//{{{
           return hdl.lock();
       }//}}}
       virtual void on_message(connection_hdl conn_hdl, char* message) = 0;
       virtual size_t read_complete(char* buff, const system::error_code& err, size_t bytes) = 0;
        const std::set<connection_ptr>& get_connections(){
            return _connection_manager->get_connections();
        }
        protected:
        void _start_accept() {//{{{
            connection_ptr conn = make_shared<connection>(_io_service);
            connection_hdl w(conn);
            conn->set_handle(w);
            conn->set_message_handler(_m_handler);
            conn->set_read_comp_handler(_rc_handler);
            //conn->set_disconnect_handler(_disconn_handler);
            //conn->set_close_handler(_close_handler);
            _acceptor->async_accept(conn->get_socket(),
                    boost::bind(&asio_server::_handle_accept, this, conn,
                        asio::placeholders::error));
        }//}}}
        void on_disconnect(connection_hdl conn_hdl){//{{{
            connection_ptr conn = get_conn_from_hdl(conn_hdl);
            if(_disconning_handler){
                _disconning_handler(conn);
            }
            _connection_manager->stop(conn);
        }//}}}
        void _handle_accept(connection_ptr conn, const system::error_code& error) {//{{{
            if(error){
                throw zexception(error, "handle accept error:" + error.message());
            }
            if(_accept_handler){
                _accept_handler(conn);
            }
            _connection_manager->start(conn);

            _start_accept();
        }//}}}
        void _listen(uint16_t port) {//{{{
            asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port);
            _listen(ep);
        }//}}}
        void _listen(const std::string& ip, uint16_t port) {//{{{
            asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
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
            else {
                _handle_stop();
            }
        }//}}}
            void _handle_stop(){//{{{
                _status = 0;
                _acceptor->close();
                _connection_manager->stop_all();
                _io_service->stop();
            }//}}}

        shared_ptr<asio::io_service> _io_service;
        shared_ptr<asio::ip::tcp::acceptor> _acceptor;
        shared_ptr<asio::signal_set> _signals;
        shared_ptr<connection_manager> _connection_manager;
        //shared_ptr<logger> _logger;
        accept_handler _accept_handler;
        message_handler _m_handler;
        read_comp_handler _rc_handler;
        disconnect_handler _disconn_handler;
        disconnecting_handler _disconning_handler;
        close_handler _close_handler;
        int _status;
    };
}

#endif
