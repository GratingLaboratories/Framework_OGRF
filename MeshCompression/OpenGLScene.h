#pragma once
#include "QJson.h"
#include "ConsoleMessageManager.h"
#include "OpenGLMesh.h"
#include <QString>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include "SliceConfig.h"

class OpenGLScene
{
public:
    OpenGLScene() = delete;
    ~OpenGLScene();
    OpenGLScene(ConsoleMessageManager &msg) : msg_(msg) {  }
    void clear();
    bool open(const QString &name);
    bool open_by_obj(const QString &file);
    bool open_by_obj(const QString &file, const QString &name);
    void add_model(OpenGLMesh &mesh);
    void remove_model(const QString &name);
    bool changed();
    int  model_number() const { return models_.size(); }
    void slice(const SliceConfig &slice_config);// { slice_config_ = slice_config; }
    std::shared_ptr<OpenGLMesh> get(const QString &model_name) const;
    std::shared_ptr<OpenGLMesh> get_by_tag(const QString &tag) const;

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
    using QStringSet = std::set<QString>;
    std::map<QString, QStringSet> map_name_tag_set_;
    std::map<QString, QStringSet> map_tag_name_set_;

    std::vector<std::shared_ptr<OpenGLMesh>> models_;
    std::map<QString, std::shared_ptr<OpenGLMesh>> ref_mesh_from_name_;
    SliceConfig slice_config_;
};

