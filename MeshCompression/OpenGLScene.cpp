#include "stdafx.h"
#include "OpenGLScene.h"

bool OpenGLScene::open(const QString& name)
{
    QString file_content;
    QFile file;
    file.setFileName(name);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    file_content = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(file_content.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            json_ = jsonDocument.object();
            return BuildFronJson();
        }
        else
        {
            msg_.log("json file is not a object.", WARNING_MSG);
            return false;
        }
    }
    else
    {
        msg_.log(QString("error when parse json file, msg = %0").arg(error.errorString()), WARNING_MSG);
        return false;
    }
}

OpenGLScene::~OpenGLScene()
{
}

bool OpenGLScene::BuildFronJson()
{
    scene_name_ = json_["SceneName"].toString();
    scene_description_ = json_["Description"].toString();
    file_location_ = json_["FileLocation"].toString();

    auto models_jarray = json_["Models"].toArray();
    for (auto ele : models_jarray)
    {
        auto model_jobj = ele.toObject();

        model_jobj["Name"].toString();
        model_jobj["Position"].toArray();
        model_jobj["NeedScale"].toBool();
        model_jobj["Scale"].toDouble();
    }

    return true;
}
