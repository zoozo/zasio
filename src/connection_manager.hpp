#ifndef ZOOZO_ZASIO_CONNECTION_MANAGER_HPP
#define ZOOZO_ZASIO_CONNECTION_MANAGER_HPP

#include <set>
#include "connection.hpp"

using namespace boost;

namespace zoozo{
namespace zasio{
    class connection_manager{
        public:
        connection_manager():_type(1){ }

        void start(connection_ptr c)
        {
            _connections.insert(c);
            if(_type == 1) c->start();
        }

        void set_type(const int type){
            _type = type;
        }
        void stop(connection_ptr c)
        {
            _connections.erase(c);
            c->stop();
        }

        void stop_all()
        {
            std::for_each(_connections.begin(), _connections.end(), bind(&connection::stop, _1));
            _connections.clear();
        }
        std::set<connection_ptr> get_connections(){
            return _connections;
        }
        private:
        std::set<connection_ptr> _connections;
        int _type;
    };
}
}

#endif
