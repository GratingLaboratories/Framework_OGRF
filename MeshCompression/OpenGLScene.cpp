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
            if (BuildFromJson())
            {
                update();
                msg_.log("build from json complete.", INFO_MSG);
                msg_.log(QString("buffer size: VAO:%0\tVEO:%1").arg(vbuffer.size()).arg(ebuffer.size()), BUFFER_INFO_MSG);
                return true;
            }
            else
            {
                msg_.log("build from json failed.", WARNING_MSG);
                return false;
            }
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

QVector3D tran_arr_to_vec3(const QJsonArray &arr)
{
    return{
        static_cast<float>(arr.at(0).toDouble()),
        static_cast<float>(arr.at(1).toDouble()),
        static_cast<float>(arr.at(2).toDouble())
    };
}

bool OpenGLScene::BuildFromJson()
{
    scene_name_ = json_["SceneName"].toString();
    scene_description_ = json_["Description"].toString();
    file_location_ = json_["FileLocation"].toString();

    auto models_jarray = json_["Models"].toArray();
    for (auto ele : models_jarray)
    {
        auto model_jobj = ele.toObject();
        models_.push_back(std::make_shared<OpenGLMesh>());
        auto model = models_.back();
        model->name_ = model_jobj["Name"].toString();
        model->position_ = tran_arr_to_vec3(model_jobj["Position"].toArray());
        if (!model_jobj["Color"].isArray())
            model->color_ = OpenGLMesh::DEFAULT_COLOR;
        else
            model->color_ = tran_arr_to_vec3(model_jobj["Color"].toArray());
        model->need_scale_ = model_jobj["NeedScale"].toBool();
        model->need_centralize_ = model_jobj["NeedCentralize"].toBool();
        model->use_face_normal_ = model_jobj["UseFaceNormal"].toBool();
        model->scale_ = model_jobj["Scale"].toDouble();
        model->file_location_ = file_location_;
        model->file_name_ = model_jobj["FileName"].toString();
        model->mesh_extension_ = model_jobj["MeshExtension"].toString();

        model->init();
        ref_mesh_from_name_[model->name_] = model;
    }

    return true;
}

void OpenGLScene::update()
{
    vbuffer.clear();
    ebuffer.clear();
    int vcnt = 0;
    int fcnt = 0;
    for (auto model : models_)
    {
        vbuffer.insert(vbuffer.end(), model->vbuffer.cbegin(), model->vbuffer.cend());
        ebuffer.insert(ebuffer.end(), model->ebuffer.cbegin(), model->ebuffer.cend());
        std::for_each(ebuffer.begin() + fcnt * VERTICES_PER_FACE, ebuffer.end(), [vcnt](GLuint &x) { x += vcnt; });
        vcnt += model->vbuffer.size() / TOTAL_ATTRIBUTE_SIZE;
        fcnt += model->mesh().n_faces();
    }


    //assert(ebuffer.size() == mesh_.n_faces() * VERTICES_PER_FACE);
}

// return whether buffer should get renew.
// whether or not, changed_ turn to false. 
bool OpenGLScene::changed()
{
    bool changed = false;
    for (auto model : models_)
    {
        if (model->changed())
            changed = true;
    }
    return changed;
}