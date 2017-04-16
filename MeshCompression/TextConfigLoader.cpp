#include "stdafx.h"
#include "TextConfigLoader.h"
#include <QFile>


TextConfigLoader::TextConfigLoader(const QString& filename) : filename_(filename)
{
    // Read shader source code from files.
    QFile config_file{ filename };
    config_file.open(QFile::ReadOnly | QFile::Text);
    QTextStream tsv{ &config_file };
    QString line;
    while (!tsv.atEnd())
    {
        line = tsv.readLine();
        QTextStream ls{ &line };
        if (line.isEmpty() || line.startsWith(';'))
            continue;
        QString name;
        QString val;
        ls >> name >> val;
        config_map_[name] = val;
    }
    config_file.close();
}

TextConfigLoader::~TextConfigLoader()
{
}
