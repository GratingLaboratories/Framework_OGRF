#pragma once
#include "QJson.h"
#include "ConsoleMessageManager.h"
#include "OpenGLMesh.h"
#include <QString>
#include <vector>

class OpenGLScene
{
public:
    OpenGLScene() = delete;
    OpenGLScene(ConsoleMessageManager &msg) : msg_(msg) {  }
    bool open(const QString &name);
    ~OpenGLScene();

private:
    ConsoleMessageManager &msg_;
    QJsonObject json_;

    bool BuildFronJson();
    QString scene_name_;
    QString scene_description_;
    QString file_location_;
    std::vector<OpenGLMesh> models_;
};

