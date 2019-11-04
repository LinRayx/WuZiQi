#include "widget.h"

#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{


//    p->test();
    QApplication a(argc, argv);
    Widget w;

    QRect screenrect = a.primaryScreen()->geometry();
    w.move(screenrect.left(), screenrect.top());
    w.show();
    return a.exec();
}
