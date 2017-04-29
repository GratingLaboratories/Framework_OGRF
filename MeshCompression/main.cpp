#include "stdafx.h"
#include "meshprogram.h"
#include <QtWidgets/QApplication>
#include "TextConfigLoader.h"

int main(int argc, char *argv[])
{
    TextConfigLoader gui_config{ "./config/gui.config" };
    auto global_font = gui_config.get_string("Global_Font");

    QApplication a(argc, argv);
    a.setFont(QFont(global_font));
    MeshProgram w{ gui_config };
    w.show();
    return a.exec();
}
