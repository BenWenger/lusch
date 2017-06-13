
#ifndef LUSCH_ERROR_H_INCLUDED
#define LUSCH_ERROR_H_INCLUDED

#include <QString>
#include <stdexcept>

namespace lsh
{
    class Error : public std::runtime_error
    {
    public:
        Error(const char* msg) : runtime_error(msg) {}
        Error(const std::string& msg) : runtime_error(msg) {}
        Error(const QString& msg) : runtime_error(msg.toStdString()) {}
    };
}

#endif