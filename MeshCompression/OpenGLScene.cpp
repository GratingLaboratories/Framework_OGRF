#include "stdafx.h"
#include "OpenGLScene.h"

bool OpenGLScene::open(const QString& name)
{
    QString val;
    QFile file;
    file.setFileName(name);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(val.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (jsonDocument.isObject()) {
            json_ = jsonDocument.object();
            QVariantMap result = jsonDocument.toVariant().toMap();
            qDebug() << "Name:" << json_["SceneName"].toString();
            qDebug() << "FileName:" << json_["FileLocation"].toString();
            auto array = json_["Models"].toArray();
            for(auto ele : array)
            {
                auto model = ele.toObject();
                qDebug() << "Model Name:" << model["Name"].toString();

            }

            ////foreach(QVariant plugin, result["plug-ins"].toList()) {
            ////    qDebug() << "\t-" << plugin.toString();
            ////}

            ////QVariantMap nestedMap = result["indent"].toMap();
            ////qDebug() << "length:" << nestedMap["length"].toInt();
            ////qDebug() << "use_space:" << nestedMap["use_space"].toBool();
        }
    }
    else {
        qFatal(error.errorString().toUtf8().constData());
        exit(1);
    }

    return true;
}

OpenGLScene::~OpenGLScene()
{
}
