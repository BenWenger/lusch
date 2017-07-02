#include "gui/luschapp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    lsh::LuschApp w;
    w.show();
    return a.exec();
}
