#ifndef ZOOZO_ZASIO_LOGGER_H
#define ZOOZO_ZASIO_LOGGER_H

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace trivial = logging::trivial;

namespace zoozo{
namespace zasio{
    class logger{
        public:
        logger(const std::string& filename){//{{{
            logging::add_file_log
                (
                 keywords::file_name = filename,
                 // This makes the sink to write log records that look like this:
                 // YYYY-MM-DD HH:MI:SS [normal] A normal severity message
                 // YYYY-MM-DD HH:MI:SS [error] An error severity message
                 keywords::open_mode = (std::ios::out | std::ios::app),
                 keywords::auto_flush = true,
                 keywords::format =
                 (
                  expr::stream
                  << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                  << " [" << logging::trivial::severity
                  << "] " << expr::smessage
                 )
                );
            logging::add_common_attributes();
        }//}}}
        void write(trivial::severity_level level, const std::string& msg){//{{{
            BOOST_LOG_SEV(_lg, level) << msg;
        }//}}}

        private:
        src::severity_logger<trivial::severity_level> _lg;
    };
}
}
#endif
