#ifndef LUSCH_GUI_CONTROLS_FILENAMESELECTOR_H_INCLUDED
#define LUSCH_GUI_CONTROLS_FILENAMESELECTOR_H_INCLUDED

#include "util/filename.h"
#include <QHBoxLayout>

class QLineEdit;
class QPushButton;
class QCheckBox;

namespace lsh
{
    class FileNameSelector : public QHBoxLayout
    {
    public:
        enum Flags
        {
            DefaultToAbsolute =     (1<<0),
            Directory =             (1<<1),
            Write =                 (1<<2),

            None =                  0
        };
        FileNameSelector(QWidget* parent, Flags flgs = Flags::None, const FileName& initial = FileName(), const FileName& rootpath = FileName(),
                         const QString& browsemsg = QString(), const QString& browseFilters = QString());

        FileName        getValue() const;
        void            setValue(const FileName& val);
        bool            getAbsolute() const;
        void            setAbsolute(bool relative);

    private:
        int             flags;
        QString         browseText;
        QString         filters;
        QLineEdit*      textBox = nullptr;
        QPushButton*    browseBtn = nullptr;
        QCheckBox*      absoluteChk = nullptr;
        QWidget*        parentWidget = nullptr;

        FileName        root;
        bool            canBeRelative = false;

        void            onCheck(int state);
        void            onBrowse(bool);
    };
}


#endif