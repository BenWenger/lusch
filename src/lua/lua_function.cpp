#include "lua_function.h"

namespace lsh
{
    //  Instantiate the one non-templated static
    std::map<std::string, int (*)(Lua&)>    LuaFunction::globalMap;
}