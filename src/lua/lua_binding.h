#ifndef LUSCH_LUA_LUA_BINDING_H_INCLUDED
#define LUSCH_LUA_LUA_BINDING_H_INCLUDED

/*
    This comment is sort of a continuation of the one found in lua_wrapper.h.  Read that first.


    During a Lua callback, the only information you're given is a lua_State* pointer, as well as any upvalues
    that are provided with the function.  As mentioned in lua_wrapper.h, I can't really use upvalues for my
    partiuclar needs, so that leaves me with just the lua_State*.

    However, when a callback is called, I need to call another function with a Lua wrapper object.  How can
    I get the Lua wrapper object from just the lua_State?

    Also... for a function like lsh.get(), there needs to be an associated Project object.  How can I get that
    object?  Without just using globals?

    Similar to my solution for functions (see lua_function.h), my solution here was a global registry.


    LuaBinding<T> has a static registry of all the T's that are "bound" to a specific lua_State*.  Classes
    which need to be bound to a Lua state (Lua, Project) should derive from this class.  The lua_State*
    can then be used as a key to retrieve the bound object.

    Objects can be bound to multiple lua_State*s, but each lua_State* can only be bound to one object of each
    type.



    TODO -- There is a slight possibility for a memory leak here.  The T's remove themselves from the registry
    upon destruction, but the lua_State*s don't!  Therefore, this registry could theoretically grow if one Project
    persists while multiple lua_State*s are built and destroyed, leaving X number of lua_State*s in the registry
    that won't get cleaned up until the bound Project is destroyed.

    I'm not TOO worried about this because in practice that won't happen, and even if it does the memory loss will
    be minimal.
 */

#include <lua/lua.h>
#include <map>

namespace lsh
{
    template <typename T>
    class LuaBinding
    {
    public:
                    LuaBinding() = default;
        virtual     ~LuaBinding();
                    LuaBinding(LuaBinding&&) = delete;              // moving derived classes is actually OK, just not with default behavior.
                    LuaBinding(const LuaBinding&) = delete;         //   any derived move should call moveBindings.
        LuaBinding& operator = (LuaBinding&&) = delete;
        LuaBinding& operator = (const LuaBinding&) = delete;
        
        static T*   fromLuaState(lua_State* L);                     // Getting an object from the global registry
        void        addBinding(lua_State* L);                       // Bind this + given lua_State
        void        removeBinding(lua_State* L);                    // Remove the binding of this + the given lua_State
        void        removeAllBindings();                            // Remove ALL bindings of this + all lua_States

    protected:
        void        moveBindings(T&& rhs);                          // Effectively a move assignment.  Removes all existing bindings and replaces them with rhs's
        void        takeBindingsFrom(T* old);                       // Takes old's bindings and adds them to this, but does not remove any of this's old bindings.

    private:
        static std::map<lua_State*, T*>     bindings;

    };

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    template <typename T>
    std::map<lua_State*, T*> LuaBinding<T>::bindings;


    ///////////////////////////////////////////
    ///////////////////////////////////////////

    template <typename T>
    LuaBinding<T>::~LuaBinding()
    {
        removeAllBindings();
    }

    template <typename T>
    void LuaBinding<T>::removeAllBindings()
    {
        T* obj = static_cast<T*>(this);

        auto i = bindings.begin();
        while(i != bindings.end())
        {
            if(i->second == obj)        i = bindings.erase(i);
            else                        ++i;
        }
    }

    template <typename T>
    void LuaBinding<T>::removeBinding(lua_State* L)
    {
        T* obj = static_cast<T*>(this);

        auto i = bindings.find(L);
        if(i != bindings.end())
        {
            if(i->second == obj)
                bindings.erase(i);
        }
    }

    template <typename T>
    void LuaBinding<T>::addBinding(lua_State* L)
    {
        bindings[L] = static_cast<T*>(this);
    }

    template <typename T>
    T* LuaBinding<T>::fromLuaState(lua_State* L)
    {
        auto i = bindings.find(L);
        if(i == bindings.end())     return nullptr;

        return i->second;
    }

    template <typename T>
    void LuaBinding<T>::moveBindings(T&& rhs)
    {
        removeAllBindings();
        takeBindingsFrom(&rhs);
    }

    template <typename T>
    void LuaBinding<T>::takeBindingsFrom(T* old)
    {
        for(auto& i : bindings)
        {
            if(i.second == old)
                i.second = static_cast<T*>(this);
        }
    }
}

#endif