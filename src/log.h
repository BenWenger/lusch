#ifndef LUSCH_LOG_H_INCLUDED
#define LUSCH_LOG_H_INCLUDED

#include <stdexcept>
#include <QString>

namespace lsh
{
    class LoggerWindow;

    /*
     * This class is a static instance, which effectively acts as a singleton.  It will automatically bind
     *   (and log to) the most recent LoggerWindow object that is created
     */

    class Log
    {
    public:
        enum Level
        {
            Debug = 0,
            Info,
            Warning,
            Error
        };

        static void     log(const QString& msg, Level lvl);

        static void     log(std::exception& e);
        static void     err(const QString& msg)     { log(msg, Level::Error);       }
        static void     wrn(const QString& msg)     { log(msg, Level::Warning);     }
        static void     inf(const QString& msg)     { log(msg, Level::Info);        }
        static void     dbg(const QString& msg)     { log(msg, Level::Debug);       }
        static void     err(const std::string& msg) { err( QString::fromStdString(msg) );   }
        static void     wrn(const std::string& msg) { wrn( QString::fromStdString(msg) );   }
        static void     inf(const std::string& msg) { inf( QString::fromStdString(msg) );   }
        static void     dbg(const std::string& msg) { dbg( QString::fromStdString(msg) );   }
        static void     err(const char* msg)        { log(msg, Level::Error);       }
        static void     wrn(const char* msg)        { log(msg, Level::Warning);     }
        static void     inf(const char* msg)        { log(msg, Level::Info);        }
        static void     dbg(const char* msg)        { log(msg, Level::Debug);       }

    private:
        friend class LoggerWindow;
        static void             loggerWindowCreated(LoggerWindow* wnd);
        static void             loggerWindowDestroyed(LoggerWindow* wnd);

        static LoggerWindow*    logger;
        
        Log() = delete;
        ~Log() = delete;
        Log(const Log&) = delete;
    };

}

#endif