#ifndef ZOOZO_ZASIO_EXCEPTION_H
#define ZOOZO_ZASIO_EXCEPTION_H

#include <exception>
#include <boost/system/error_code.hpp>

namespace zasio{
    class zexception : public std::exception{
        public:
            explicit zexception(boost::system::error_code ec, const std::string& what )
                :_code(ec), _what(what) { }
            explicit zexception(boost::system::error_code ec)
                :_code(ec), _what(ec.message()) {}

            ~zexception() throw() {}

            boost::system::error_code code() const throw()
            {
                return _code;
            }
            virtual const char* what() const throw()
            {
                return _what.c_str();
            }

        private:
            boost::system::error_code _code;
            const std::string _what;
    };
}
#endif
