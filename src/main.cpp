#include "gui/luschapp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    lsh::LuschApp w;
    w.setGeometry(100, 100, 1200, 600);     // TODO this is temporary
    w.show();
    return a.exec();
}
