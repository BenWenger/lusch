#ifndef LUSCH_CORE_PROJECTDATA_H_INCLUDED
#define LUSCH_CORE_PROJECTDATA_H_INCLUDED

#include <memory>
#include <QObject>

namespace lsh
{
    class ProjectData : public QObject
    {
        Q_OBJECT

    public:
        enum class Type {   Null,   Int,    Dbl,    Str,    Obj     };

        typedef std::unique_ptr<ProjectData>    Ptr;
        typedef std::int64_t                    int_t;

        virtual ~ProjectData() = default;


        Type            getType() const             { return type;  }

        std::string     asString() const            { return v_str; }
        int_t           asInt() const               { return v_int; }
        double          asDbl() const               { return v_dbl; }
        
    public slots:
        void            setNull()                   { type = Type::Null;                emit dataChanged(this); }
        void            set(const std::string& v)   { type = Type::Str;     v_str = v;  emit dataChanged(this); }
        void            set(int_t v)                { type = Type::Int;     v_int = v;  emit dataChanged(this); }
        void            set(double v)               { type = Type::Dbl;     v_dbl = v;  emit dataChanged(this); }


    signals:
        void    dataChanged(ProjectData* dat);

    private:
        Type            type;
        int_t           v_int;
        double          v_dbl;
        std::string     v_str;
        // TODO object

    };

}

#endif