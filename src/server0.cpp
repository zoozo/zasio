#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <exception>
#include <boost/system/error_code.hpp>
#include <functional>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <set>


using boost::asio::ip::tcp;
class connection;
typedef boost::asio::io_service * io_service_ptr;
typedef connection * connection_ptr;
//typedef boost::function<void(connection_ptr, std::string)> message_handler;
typedef boost::function<void(std::string)> message_handler2;
//typedef boost::function<void()> message_handler2;
class connection{//{{{
public:
  connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  void set_message_handler2(message_handler2 m_handler2){
      std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
      _m_handler2 = m_handler2;
  }
  tcp::socket& socket()
  {
    return socket_;
  }

  void stop()
  {
      boost::system::error_code ec;
      try{
      if(socket_.is_open()){
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
          socket_.close(ec);
      }
      }catch(std::exception &e){
          std::cout<<e.what()<<std::endl;
      }

      if(ec){
          std::cout<<"error\n"<<std::endl;
      }
  }
  void start()
  {
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&connection::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
  }

private:
  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      std::string str = "sss";

      /*
      if(_m_handler){
          _m_handler(str);
      }
      */
      if(_m_handler2){
          _m_handler2(str);
      }
      boost::asio::async_write(socket_,
          boost::asio::buffer(str, bytes_transferred),
          boost::bind(&connection::handle_write, this,
            boost::asio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }

  void handle_write1(const boost::system::error_code& error)
  {
  }
  void handle_write(const boost::system::error_code& error)
  {
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
          boost::bind(&connection::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  //message_handler _m_handler;
  message_handler2 _m_handler2;
};//}}}

class connection_manager{
    public:
    void start(connection_ptr c)
    {
        _connections_ptr.insert(c);
        c->start();
    }

    void stop(connection_ptr c)
    {
        _connections_ptr.erase(c);
        c->stop();
    }

    void stop_all()
    {
        std::for_each(_connections_ptr.begin(), _connections_ptr.end(),
                boost::bind(&connection::stop, _1));
        _connections_ptr.clear();
    }

    private:
      /// The managed connections.
        std::set<connection_ptr> _connections_ptr;
};

class asio_server{
    public:
    asio_server(){
        _connection_manager = new connection_manager();

        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        init_asio();
    }
    void init(){
        listen(6666);
        _signals = new boost::asio::signal_set(*_io_service, SIGINT, SIGTERM, SIGQUIT);
 //       _signals->add(SIGINT);
 //       _signals->add(SIGTERM);
        _signals->async_wait(boost::bind(&asio_server::handle_stop, this));
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        start_accept();
    }
    void start_accept()
    {
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        connection* new_connection = new connection(*_io_service);
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        new_connection->set_message_handler2(_m_handler2);
        _acceptor->async_accept(new_connection->socket(),
                boost::bind(&asio_server::handle_accept, this, new_connection,
                    boost::asio::placeholders::error));
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    }
    void handle_accept(connection* new_connection, const boost::system::error_code& error)
    {
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        if (!error)
        {
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            _connection_manager->start(new_connection);
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        }
        else
        {
            delete new_connection;
        }

        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        start_accept();
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    }

    void init_asio(io_service_ptr ptr) {
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        _io_service = ptr;
        _acceptor = boost::make_shared<boost::asio::ip::tcp::acceptor>(
                    boost::ref(*_io_service));
    }
    /*
    boost::asio::ip::tcp::endpoint ep(int internet_protocol, uint16_t port){
        //listen(ep);
    }
    */
    void listen(uint16_t port) {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), port);
        listen(ep);
    }
    void listen(boost::asio::ip::tcp::endpoint const & ep) {
        //boost::system::error_code ec;
        boost::system::error_code ec;
        _acceptor->open(ep.protocol(), ec);
        //listen(ep,ec);
        if (ec) { 
            std::cout<<"error:"<<__LINE__<<std::endl;
            //throw boost::system::system_error{boost::system::error_code::make_error_code(boost::system::not_supported)};
        }
        _acceptor->set_option(boost::asio::socket_base::reuse_address(true),ec);
        if (ec) { 
            std::cout<<"error:"<<__LINE__<<std::endl;
            //throw boost::system::system_error{boost::system::error_code::make_error_code(boost::system::not_supported)};
        }
        _acceptor->bind(ep, ec);
        if (ec) { 
            std::cout<<"error:"<<boost::system::error_code()<<__LINE__<<std::endl;
            //throw boost::system::system_error{boost::system::error_code::make_error_code(boost::system::not_supported)};
        }
        _acceptor->listen(boost::asio::socket_base::max_connections, ec);
        if (ec) { 
            std::cout<<"error:"<<__LINE__<<std::endl;
            //throw boost::system::system_error{boost::system::error_code::make_error_code(boost::system::not_supported)};
        }
    }
    void init_asio(){
        init_asio(new boost::asio::io_service());
    }
    void run(){
        _io_service->run();
    }
    void handle_stop()
    {
        // The server is stopped by cancelling all outstanding asynchronous
        // operations. Once all operations have finished the io_service::run() call
        // will exit.
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        _acceptor->close();
        _connection_manager->stop_all();
        _io_service->stop();


    }
    /*
    void set_message_handler(message_handler m_handler){
        _m_handler = m_handler;
    }
    */
    void set_message_handler2(message_handler2 m_handler2){
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        _m_handler2 = m_handler2;
    }
    public:
    
    private: 
    boost::asio::io_service *_io_service;
    //boost::asio::ip::tcp::socket *_socket; 
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    boost::asio::signal_set *_signals;
    connection_manager *_connection_manager;
    //message_handler _m_handler;
    message_handler2 _m_handler2;



};
class server{
    public:
    server(){
        _server = new asio_server();
        _server->set_message_handler2(boost::bind(&server::on_message, this, ::_1));
        _server->init();
    }
    void on_message(std::string message){
        std::cout<<message<<std::endl;
    }
    void run(){
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        _server->run();
    }
    private:
    asio_server *_server;
};

int main()
{
  try
  {
    boost::asio::io_service io_service;
    server s;
    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
