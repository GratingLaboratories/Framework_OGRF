#include "stdafx.h"
#include "meshprogram.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setFont(QFont("Inziu Iosevka SC"));
    MeshProgram w;
    w.show();
    return a.exec();
}
