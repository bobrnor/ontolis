#include "OntolisWindow.h"
#include <QApplication>

#include <QDebug>

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    OntolisWindow w;
    w.show();
    
    return a.exec();
}
