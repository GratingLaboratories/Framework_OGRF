#include "stdafx.h"
#include "TextConfigLoader.h"
#include <QFile>
#include <iostream>
#include <QString>

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

        // blank line all whole line comment.
        if (line.isEmpty() || line.startsWith(';'))
            continue;

        QString key, temp;
        bool key_end = false;
        bool in_string = false;
        bool in_space = false;
        bool ok = true;

        for (auto c : line)
        {
            // if state is in_string, put into temp anyway.
            if (in_string && c != '\"')
            {
                temp.push_back(c);
                continue;
            }

            if (in_space && c.isSpace())
            {
                continue;
            }

            // throw remaining characters.
            if (c == ';')
                break;
            // put into temp for non-space and not '\"'.
            else if (!c.isSpace() && c != '\"')
                temp.push_back(c);
            // switch in_string state;
            else if (c == '\"')
            {
                if (in_string && !key_end)
                {
                    key = temp;
                    temp.clear();
                    key_end = true;
                }
                in_string = !in_string;
            }
            // inner space
            else if (c.isSpace() && !key_end)
            {
                key = temp;
                temp.clear();
                key_end = true;
                in_space = true;
            }
            else
            {
                std::cerr << "Config Parse Fail at line:\n\t"
                    << line.toStdString() << std::endl;
                ok = false;
            }
        }
        if (ok)
            config_map_[key] = temp;
    }
    config_file.close();
}

TextConfigLoader::~TextConfigLoader()
{
}

QColor TextConfigLoader::get_color(const QString& str)
{
    auto r = str + "_r";
    auto g = str + "_g";
    auto b = str + "_b";
    return{ config_map_[r].toInt(), config_map_[g].toInt(), config_map_[b].toInt() };
}

std::array<GLfloat, 3> TextConfigLoader::get_colorf(const QString& str)
{
    auto r = str + "_rf";
    auto g = str + "_gf";
    auto b = str + "_bf";
    return{ config_map_[r].toFloat(), config_map_[g].toFloat(), config_map_[b].toFloat() };
}
