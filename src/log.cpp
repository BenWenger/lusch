
#include "log.h"
#include "gui/loggerwindow.h"

namespace lsh
{
    LoggerWindow*       Log::logger = nullptr;

    void Log::loggerWindowCreated(LoggerWindow* wnd)
    {
        logger = wnd;
    }
    
    void Log::loggerWindowDestroyed(LoggerWindow* wnd)
    {
        if(wnd == logger)
            logger = nullptr;
    }
    
    void Log::log(const QString& msg, Level level)
    {
        if(logger)
            logger->log(msg, level);
    }

    void Log::log(std::exception& e)
    {
        auto ae = dynamic_cast<std::bad_alloc*>(&e);
        if(ae)
            log( QString("Memory allocation error:  ") + ae->what(), Level::Error );
        else
            log( e.what(), Level::Error );
    }

}