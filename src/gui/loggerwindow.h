#ifndef LUSCH_GUI_LOGGERWINDOW_H_INCLUDED
#define LUSCH_GUI_LOGGERWINDOW_H_INCLUDED

#include <QTextEdit>
#include "log.h"

namespace lsh
{

    class LoggerWindow : public QTextEdit
    {
        Q_OBJECT

    public:
                        LoggerWindow(QWidget *parent = Q_NULLPTR);
        virtual         ~LoggerWindow();

        void            log(const QString& str, Log::Level level);
    };

}


#endif