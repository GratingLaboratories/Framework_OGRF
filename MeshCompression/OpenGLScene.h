#pragma once
#include "QJson.h"
#include "ConsoleMessageManager.h"
#include <QString>

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
};

