#pragma once
#include "QJson.h"
#include "ConsoleMessageManager.h"
#include "OpenGLMesh.h"
#include <QString>
#include <vector>
#include <map>
#include <memory>

class OpenGLScene
{
public:
    OpenGLScene() = delete;
    ~OpenGLScene();
    OpenGLScene(ConsoleMessageManager &msg) : msg_(msg) {  }
    void clear();
    bool open(const QString &name);
    bool changed();

    std::vector<GLfloat> vbuffer;
    std::vector<GLuint> ebuffer;

private:
    bool BuildFromJson();
    void update();

    ConsoleMessageManager &msg_;
    QJsonObject json_;

    QString scene_name_;
    QString scene_description_;
    QString file_location_;
    std::vector<std::shared_ptr<OpenGLMesh>> models_;
    std::map<QString, std::shared_ptr<OpenGLMesh>> ref_mesh_from_name_;

};

