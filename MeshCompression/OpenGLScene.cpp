#include "stdafx.h"

#include "OpenGLScene.h"

void OpenGLScene::clear()
{
    vbuffer.clear();
    ebuffer.clear();
    models_.clear();
    ref_mesh_from_name_.clear();
}

bool OpenGLScene::open(const QString& name)
{
    clear();
    msg_.reset_indent();
    msg_.log("Read Scene from file ", name, INFO_MSG);

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
                msg_.log(QString("buffer size: VBO:%0\tVEO:%1").arg(vbuffer.size()).arg(ebuffer.size()), BUFFER_INFO_MSG);
                msg_.log("", BUFFER_INFO_MSG);
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

bool OpenGLScene::open_by_obj(const QString& name)
{
    msg_.reset_indent();
    msg_.log("Read OBJ from file ", name, INFO_MSG);

    models_.push_back(std::make_shared<OpenGLMesh>());
    auto model = models_.back();
    model->name_ = "Main";
    //map_name_tag_set_[model->name_] = QStringSet{};
    model->position_ = { 0.0, 0.0 ,0.0 };
    model->color_ = { 1.0, 1.0 ,1.0 };
    model->need_scale_ = false;
    model->scale_ = 1.0;
    model->need_centralize_ = false;
    model->use_face_normal_ = false;
    model->show_tetra_ = false;
    model->file_location_ = "";
    model->file_name_ = name;
    model->mesh_extension_ = "";
    model->slice(slice_config_);

    model->init();
    ref_mesh_from_name_[model->name_] = model;

    msg_.log("Read Complete.", model->name_, INFO_MSG);

    return true;
}

bool OpenGLScene::open_by_obj(const QString& file, const QString& name)
{
    msg_.reset_indent();
    msg_.log("Read OBJ from file ", file, INFO_MSG);

    models_.push_back(std::make_shared<OpenGLMesh>());
    auto model = models_.back();
    model->name_ = name;
    model->position_ = { 0.0, 0.0 ,0.0 };
    model->color_ = { 1.0, 1.0 ,1.0 };
    model->need_scale_ = false;
    model->scale_ = 1.0;
    model->need_centralize_ = false;
    model->use_face_normal_ = false;
    model->show_tetra_ = false;
    model->file_location_ = "";
    model->file_name_ = file;
    model->mesh_extension_ = "";
    model->slice(slice_config_);

    model->init();
    ref_mesh_from_name_[model->name_] = model;

    msg_.log("Read Complete.", model->name_, INFO_MSG);

    return true;
}

void OpenGLScene::add_model(OpenGLMesh& mesh)
{
    models_.push_back(std::make_shared<OpenGLMesh>());
    auto &model = *models_.back();
    model = mesh;
    model.update();
    ref_mesh_from_name_[model.name_] = std::make_shared<OpenGLMesh>(model);
}

void OpenGLScene::remove_model(const QString& name)
{
    for (auto m_iter = models_.begin(); m_iter != models_.end(); ++m_iter)
    {
        if ((*m_iter)->name_ == name)
        {
            models_.erase(m_iter);
            ref_mesh_from_name_.erase(name);
            break;
        }
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
    msg_.log("Scene Info:", INFO_MSG);
    msg_.indent(1);
    scene_name_ = json_["SceneName"].toString();
    scene_description_ = json_["Description"].toString();
    file_location_ = json_["FileLocation"].toString();
    msg_.log("SceneName:\t", scene_name_, INFO_MSG);
    msg_.log("Description:\t", scene_description_, INFO_MSG);
    msg_.log("FileLocation:\t", file_location_, INFO_MSG);

    msg_.indent(2);
    auto models_jarray = json_["Models"].toArray();
    for (auto ele : models_jarray)
    {
        auto model_jobj = ele.toObject();
        models_.push_back(std::make_shared<OpenGLMesh>());
        auto model = models_.back();
        model->name_ = model_jobj["Name"].toString();
        map_name_tag_set_[model->name_] = QStringSet{};
        model->position_ = tran_arr_to_vec3(model_jobj["Position"].toArray());
        if (!model_jobj["Color"].isArray())
            model->color_ = OpenGLMesh::DEFAULT_COLOR;
        else
            model->color_ = tran_arr_to_vec3(model_jobj["Color"].toArray());
        model->need_scale_ = model_jobj["NeedScale"].toBool();
        model->need_centralize_ = model_jobj["NeedCentralize"].toBool();
        model->use_face_normal_ = model_jobj["UseFaceNormal"].toBool();
        model->show_tetra_ = model_jobj["ShowTetra"].toBool() & NEED_TETRA;
        model->scale_ = model_jobj["Scale"].toDouble();
        model->file_location_ = file_location_;
        model->file_name_ = model_jobj["FileName"].toString();
        model->mesh_extension_ = model_jobj["MeshExtension"].toString();
        // TODO tags

        model->init();
        ref_mesh_from_name_[model->name_] = model;

        msg_.log("Read mesh:\t", model->name_, INFO_MSG);
    }

    msg_.log("", INFO_MSG);
    msg_.reset_indent();
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
        fcnt += model->ebuffer.size() / VERTICES_PER_FACE;
    }
}

// return whether buffer should get renew.
// whether or not, changed_ turn to false. 
bool OpenGLScene::changed()
{
    bool changed = false;
    for (auto model : models_)
    {
        if (model->changed())
        {
            changed = true;
        }
    }
    if (changed)
        update();
    return changed;
}

void OpenGLScene::slice(const SliceConfig& slice_config)
{
    this->slice_config_ = slice_config;
    // TODO 
    // current: slice only the first mesh in the scene.
    if (!models_.empty())
        models_[0]->slice(slice_config);
}

std::shared_ptr<OpenGLMesh> OpenGLScene::get(const QString& model_name) const
{
    auto it = ref_mesh_from_name_.find(model_name);
    if (it != ref_mesh_from_name_.end())
        return it->second;
    else
        return nullptr;
}

std::shared_ptr<OpenGLMesh> OpenGLScene::get_by_tag(const QString& tag) const
{
    auto it = map_name_tag_set_.find(tag);
    if (it != map_name_tag_set_.end())
        return get(*it->second.cbegin());
    else
        return nullptr;
}
