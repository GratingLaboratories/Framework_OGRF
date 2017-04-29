#pragma once
#include <QString>
#include <map>

class TextConfigLoader
{
public:
    TextConfigLoader(const QString &filename);
    ~TextConfigLoader();
    QString get_string(const QString &str) { return config_map_[str]; }
    float get_value(const QString &str) { return config_map_[str].toFloat(); }
    int   get_int(const QString &str) { return config_map_[str].toInt(); }
    bool  get_bool(const QString &str) { return config_map_[str] == "True"; }

private:
    QString filename_;
    std::map<QString, QString> config_map_;
};

