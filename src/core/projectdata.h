#ifndef LUSCH_CORE_PROJECTDATA_H_INCLUDED
#define LUSCH_CORE_PROJECTDATA_H_INCLUDED

#include <memory>
#include <QObject>
#include "lua/objects/lua_object.h"
#include "util/qtjson.h"

namespace lsh
{
    class ProjectData : public QObject
    {
        Q_OBJECT

    public:
        enum class Type {   Null,   Bool,   Int,    Dbl,    Str,    Obj     };

        typedef std::unique_ptr<ProjectData>    Ptr;
        typedef std::int64_t                    int_t;

        virtual         ~ProjectData() = default;


        Type            getType() const             { return type;  }

        std::string     asString() const            { return v_str; }
        int_t           asInt() const               { return v_int; }
        bool            asBool() const              { return v_bool; }
        double          asDbl() const               { return v_dbl; }
        LuaObject::Ptr  asObj() const               { return v_obj; }

        bool            shouldSaveToJson() const    { return type != Type::Null;        }       // TODO, this may change for some objects.
        json::value     toJson() const;
        
    public slots:
        void            setNull()                   { type = Type::Null;                v_obj.reset();  emit dataChanged(this); }
        void            set(bool v)                 { type = Type::Str;     v_bool = v; v_obj.reset();  emit dataChanged(this); }
        void            set(const std::string& v)   { type = Type::Str;     v_str = v;  v_obj.reset();  emit dataChanged(this); }
        void            set(int_t v)                { type = Type::Int;     v_int = v;  v_obj.reset();  emit dataChanged(this); }
        void            set(double v)               { type = Type::Dbl;     v_dbl = v;  v_obj.reset();  emit dataChanged(this); }
        void            set(const LuaObject::Ptr& v){ type = Type::Obj;     v_obj = v;                  emit dataChanged(this); }


    signals:
        void            dataChanged(ProjectData* dat);

    private:
        Type            type = Type::Null;
        bool            v_bool = false;
        int_t           v_int = 0;
        double          v_dbl = 0;
        std::string     v_str;
        LuaObject::Ptr  v_obj;
    };
    
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////
    
    inline json::value ProjectData::toJson() const
    {
        switch(type)
        {
        case Type::Bool:        return json::value( v_bool );
        case Type::Int:         return json::value( v_int );
        case Type::Dbl:         return json::value( v_dbl );
        case Type::Str:         return json::value( v_str );
        case Type::Obj:         /* TODO */      break;
        }

        return json::value();
    }

}

#endif