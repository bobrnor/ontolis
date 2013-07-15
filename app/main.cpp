#include "OLSMainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OLSMainWindow w;
    w.show();
    
    return a.exec();
}
