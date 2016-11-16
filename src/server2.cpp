#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using namespace std;

const int max_length = 1024;

void session(tcp::socket sock)
{
  cout<<__FUNCTION__<<endl;
  try
  {
    for (;;)
    {
        cout<<__LINE__<<endl;
      char data[max_length];

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      boost::asio::write(sock, boost::asio::buffer(data, length));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_service& io_service, unsigned short port)
{
    cout<<__FUNCTION__<<endl;
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    cout<<__LINE__<<endl;
    tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    server(io_service, std::atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
