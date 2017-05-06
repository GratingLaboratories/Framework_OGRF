#pragma once
#include <QString>
#include <QColor>
#include <map>
#include <array>

class TextConfigLoader
{
public:
    TextConfigLoader(const QString &filename);
    ~TextConfigLoader();
    QString get_string(const QString &str) { return config_map_[str]; }
    float   get_value(const QString &str) { return config_map_[str].toFloat(); }
    int     get_int(const QString &str) { return config_map_[str].toInt(); }
    bool    get_bool(const QString &str) { return config_map_[str] == "True"; }
    QColor  get_color(const QString &str);
    std::array<GLfloat, 3>  get_colorf(const QString &str);

private:
    QString filename_;
    std::map<QString, QString> config_map_;
};

