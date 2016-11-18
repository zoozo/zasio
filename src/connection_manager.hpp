#ifndef ZOOZO_ZASIO_CONNECTION_MANAGER_HPP
#define ZOOZO_ZASIO_CONNECTION_MANAGER_HPP

#include <set>
#include "connection.hpp"

using namespace boost;

namespace zoozo{
namespace zasio{
    class connection_manager{
        public:
        void start(connection_ptr c)
        {
            _connections.insert(c);
            c->start();
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
    };
}
}

#endif
