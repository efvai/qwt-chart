#include "qwtchart.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QwtChart w;
    w.show();
    return a.exec();
}
