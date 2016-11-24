#include "asio_client.hpp"

using namespace boost;

class client : public zoozo::zasio::asio_client{
    void on_message(std::string& message){
        std::cout<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        std::cout<<message<<std::endl;
    }
};

int main()
{
    client c;
    c.connect("10.0.2.15", 6666);
    c.run();

    return 0;
}
