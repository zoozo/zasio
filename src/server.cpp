#include "asio_server.hpp"

using namespace zoozo;

class server : public zasio::asio_server{
     void on_message(zasio::connection_hdl conn_hdl, std::string& message){
         std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
         zasio::connection_ptr conn = get_conn_from_hdl(conn_hdl);
         conn->send(message);
    }
};
int main()
{
    server s;
    s.init(6666);
    s.run(3);
    //s.run();

    return 0;
}
