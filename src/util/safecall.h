#ifndef LUSCH_UTIL_SAFECALL_H_INCLUDED
#define LUSCH_UTIL_SAFECALL_H_INCLUDED

#include "log.h"

#define BEGIN_SAFE      try {
#define END_SAFE        } catch(std::exception& e) { e.what(); }    \
                        catch(...) { Log::err("Internal Error:  Unexpected value caught during safeCall"); }


#endif
