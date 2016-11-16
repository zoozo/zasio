#include "asio_server.hpp"

using namespace zoozo;

class server : public zasio::asio_server{
     void on_message(zasio::socket_ptr socket){
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    }
};
int main()
{
    server s;
    s.init(6666);
    s.run();

    return 0;
}
