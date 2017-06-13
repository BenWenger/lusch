
#include "loggerwindow.h"
#include "log.h"

namespace lsh
{

    LoggerWindow::LoggerWindow(QWidget* parent)
        : QTextEdit(parent)
    {
        Log::loggerWindowCreated(this);

        setReadOnly(true);
    }

    LoggerWindow::~LoggerWindow()
    {
        Log::loggerWindowDestroyed(this);
    }


    void LoggerWindow::log(const QString& msg, Log::Level level)
    {
        int weight =    QFont::Normal;
        bool italic =   false;
        QColor clr =    Qt::black;
        
        switch(level)
        {
        case Log::Level::Debug:
            clr =           QColor(0,128,0);
            italic =        true;
            break;
        case Log::Level::Warning:
            clr =           QColor(128,0,0);
            italic =        true;
            break;
        case Log::Level::Error:
            weight =        QFont::Bold;
            clr =           Qt::red;
            break;
        }

        /* QTextEdit is a little weird.  We can't change the style directly, as that will change the style of any highlighted portion
         *  (the user might have highlighted something to copy/paste it).  So instead, we have to create a cursor, put it at the end,
         *  set its format, then use it to insert text.
         *
         * After all that, we need to do a normal call to append with an empty string.  This will insert a newline in the log, AND it
         *  will automatically scroll the log down to keep the most recent entry visible.
         */
        
        // The format
        QTextCharFormat fmt;
        fmt.setFontWeight(weight);
        fmt.setFontItalic(italic);
        fmt.setForeground(QBrush(clr));

        // The cursor
        QTextCursor curs = textCursor();
        curs.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        curs.setCharFormat(fmt);

        // Use the cursor to insert the text
        curs.insertText(msg);

        // append empty string for newline and auto-scroll
        append("");
    }
}