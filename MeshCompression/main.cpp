#include "stdafx.h"
#include "meshcompression.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MeshCompression w;
    w.show();
    return a.exec();
}
