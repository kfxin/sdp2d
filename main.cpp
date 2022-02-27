#include "seismicdataprocessing2d.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SeismicDataProcessing2D w;
    w.show();   
    return a.exec();
}
