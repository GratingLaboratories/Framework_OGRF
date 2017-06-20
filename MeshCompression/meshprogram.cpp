#include "stdafx.h"

#include "renderingwidget.h"
#include "meshprogram.h"

MeshProgram::MeshProgram(TextConfigLoader &gui_config, QWidget *parent)
    : QMainWindow(parent), gui_config_(gui_config)
{
    renderingwidget_ = new RenderingWidget(this);
    connect(
        this, SIGNAL(SendLayerConfig(const LayerConfig &)),
        renderingwidget_, SLOT(LayerConfigChanged(const LayerConfig &))
        ); //TODO

    setCentralWidget(new QWidget);

    // Default windows SIZE, minimize.
    renderingwidget_->setMinimumHeight(gui_config_.get_int("GL_Widget_Min_Height"));
    renderingwidget_->setMinimumWidth(gui_config_.get_int("GL_Widget_Min_Width"));
    renderingwidget_->setMaximumHeight(gui_config_.get_int("GL_Widget_Max_Height"));
    renderingwidget_->setMaximumWidth(gui_config_.get_int("GL_Widget_Max_Width"));

    setGeometry(
        gui_config_.get_int("Set_Geometry_X"),
        gui_config_.get_int("Set_Geometry_Y"),
        0, 0);
    resize(0, 0);

    CreateActions();
    CreateMenus();
    CreateToolBars();
    CreateStatusBar();
    CreateRenderGroup();

    // Vertical Box Layout (on the left)
    QVBoxLayout *layout_left = new QVBoxLayout;
    layout_left->addWidget(groupbox_render_);
    layout_left->addStretch(1);
    layout_left->addWidget(groupbox_layer_);
    layout_left->addStretch(1);
    layout_left->addWidget(groupbox_control_);
    layout_left->addStretch(2);

    // Horizontal Box Layout
    QHBoxLayout *layout_main = new QHBoxLayout;
    layout_main->addLayout(layout_left);
    layout_main->setStretch(0, 0);
    layout_main->addWidget(renderingwidget_);
    layout_main->setStretch(1, 1);

    centralWidget()->setLayout(layout_main);

    toolbar_file_->setVisible(false);
}

void MeshProgram::CreateActions()
{
    action_new_ = new QAction(QIcon(":/MeshCompression/Resources/images/new.png"), tr("&New"), this);
    action_new_->setShortcut(QKeySequence::New);
    action_new_->setStatusTip(tr("Create a new file"));

    action_open_ = new QAction(QIcon(":/MeshCompression/Resources/images/open.png"), tr("&Open..."), this);
    action_open_->setShortcuts(QKeySequence::Open);
    action_open_->setStatusTip(tr("Open an existing file"));
    connect(action_open_, SIGNAL(triggered()), renderingwidget_, SLOT(ReadScene()));

    action_save_ = new QAction(QIcon(":/MainWindow/save.png"), tr("&Save"), this);
    action_save_->setShortcuts(QKeySequence::Save);
    action_save_->setStatusTip(tr("Save the document to disk"));
    connect(action_save_, SIGNAL(triggered()), renderingwidget_, SLOT(WriteMesh()));

    action_saveas_ = new QAction(tr("Save &As..."), this);
    action_saveas_->setShortcuts(QKeySequence::SaveAs);
    action_saveas_->setStatusTip(tr("Save the document under a new name"));

    action_loadmesh_ = new QAction(tr("open_scene"), this);
    action_loadmesh_->setIcon(QIcon(tr(":/MainWindow/open.png")));
    action_loadtexture_ = new QAction(tr("LoadTexture"), this);
    action_background_ = new QAction(tr(""), this);

    action_convert_ = new QAction(tr("Convert"), this);
    action_param_ = new QAction(tr("Param"), this);

    connect(action_loadmesh_, SIGNAL(triggered()), renderingwidget_, SLOT(ReadScene()));
    connect(action_loadtexture_, SIGNAL(triggered()), renderingwidget_, SLOT(LoadTexture()));
    connect(action_background_, SIGNAL(triggered()), renderingwidget_, SLOT(SetBackground()));

    action_open_mesh = new QAction(tr("[&Open one mesh]"), this);
    connect(action_open_mesh, SIGNAL(triggered()), renderingwidget_, SLOT(OpenOneMesh()));

    action_get_skeleton = new QAction(tr("[&Skeleton]"));
    connect(action_get_skeleton, SIGNAL(triggered()), renderingwidget_, SLOT(Skeleton()));

    action_load_skeleton = new QAction(tr("[&Load Skeleton]"));
    connect(action_load_skeleton, SIGNAL(triggered()), renderingwidget_, SLOT(Load_Skeleton()));

    action_main_solution = new QAction(tr("[&Main Solution]"));
    connect(action_main_solution, SIGNAL(triggered()), renderingwidget_, SLOT(Main_Solution()));
}

void MeshProgram::CreateMenus()
{
    //menu_file_ = menuBar()->addMenu(tr("&File"));
    //menu_file_->setStatusTip(tr("File menu"));
    //menu_file_->addAction(action_new_);
    //menu_file_->addAction(action_open_);
    //menu_file_->addAction(action_save_);
    //menu_file_->addAction(action_saveas_);
}

void MeshProgram::CreateToolBars()
{
    toolbar_file_ = addToolBar(tr("File"));
    toolbar_file_->addAction(action_new_);
    toolbar_file_->addAction(action_open_);
    toolbar_file_->addAction(action_save_);

    toolbar_basic_ = addToolBar(tr("Basic"));
    toolbar_basic_->addAction(action_loadmesh_);
    toolbar_basic_->addAction(action_save_);
    toolbar_basic_->addAction(action_background_);
    toolbar_basic_->addAction(action_open_mesh);
    toolbar_basic_->addAction(action_get_skeleton);
    toolbar_basic_->addAction(action_load_skeleton);
    toolbar_basic_->addAction(action_main_solution);
    //toolbar_basic_->addAction(action_convert_);
    //toolbar_basic_->addAction(action_param_);
}

void MeshProgram::CreateStatusBar()
{
    label_meshinfo_ = new QLabel(QString("Mesh: p: %1 e: %2 f: %3\t").arg(0).arg(0).arg(0));
    label_meshinfo_->setAlignment(Qt::AlignCenter);
    label_meshinfo_->setMinimumSize(label_meshinfo_->sizeHint());

    label_operatorinfo_ = new QLabel();
    label_operatorinfo_->setAlignment(Qt::AlignVCenter);

    connect(renderingwidget_, SIGNAL(meshInfo(int, int, int)), this, SLOT(ShowMeshInfo(int, int, int)));

    statusBar()->addWidget(label_operatorinfo_);
    connect(renderingwidget_, SIGNAL(operatorInfo(QString)), label_operatorinfo_, SLOT(setText(QString)));
}

void MeshProgram::CreateRenderGroup()
{
    // Group Render
    checkbox_dept_ = new QCheckBox(tr("Depth_Test"), this);
    connect(checkbox_dept_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawPoint(bool)));
    checkbox_dept_->setChecked(true);

    checkbox_cull_ = new QCheckBox(tr("Cull_Face"), this);
    connect(checkbox_cull_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawEdge(bool)));
    checkbox_cull_->setChecked(true);

    checkbox_face_ = new QCheckBox(tr("Face"), this);
    connect(checkbox_face_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawFace(bool)));
    checkbox_face_->setChecked(true);

    checkbox_light_ = new QCheckBox(tr("Indication"), this);
    connect(checkbox_light_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckLight(bool)));

    checkbox_texture_ = new QCheckBox(tr("/"), this);
    connect(checkbox_texture_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawTexture(bool)));

    checkbox_axes_ = new QCheckBox(tr("/"), this);
    connect(checkbox_axes_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawAxes(bool)));

    groupbox_render_ = new QGroupBox(tr("Render"), this);

    // Group Control
    pushbutton_compress_ = new QPushButton(tr("Apply"), this);
    connect(pushbutton_compress_, SIGNAL(clicked()), this, SLOT(ControlLineEvent()));

    lineedit_control_ = new QLineEdit(tr(""), this);
    lineedit_control_->setMinimumWidth(300);
    connect(lineedit_control_, SIGNAL(returnPressed()), this, SLOT(ControlLineEvent()));
    connect(this, SIGNAL(SendCmdText(const QString &)),
        renderingwidget_, SLOT(ControlLineEvent(const QString &)));

    groupbox_control_ = new QGroupBox("Control", this);

    // Group Layer
    lineedit_mask_ = new QLineEdit(tr("00111022200"), this);
    lineedit_mask_->setMinimumWidth(300);
    connect(lineedit_mask_, SIGNAL(returnPressed()), this, SLOT(LayerTextEvent()));
    lineedit_ppl_  = new QLineEdit(tr("11.3"), this);
    lineedit_ppl_->setMinimumWidth(300);
    connect(lineedit_ppl_, SIGNAL(returnPressed()), this, SLOT(LayerTextEvent()));

    slider_offset_block_ = new QSlider(Qt::Orientation::Horizontal);
    slider_offset_block_->setMaximum(100);
    slider_offset_block_->setMinimum(0);
    slider_offset_block_->setValue(50);
    connect(slider_offset_block_, SIGNAL(valueChanged(int)), this, SLOT(LayerSliderEvent(int)));
    slider_offset_grid_ = new QSlider(Qt::Orientation::Horizontal);
    slider_offset_grid_->setMaximum(100);
    slider_offset_grid_->setMinimum(0);
    slider_offset_grid_->setValue(50);
    connect(slider_offset_grid_, SIGNAL(valueChanged(int)), this, SLOT(LayerSliderEvent(int)));

    groupbox_layer_ = new QGroupBox("Layer", this);

    // Layout for Group Boxes
    QVBoxLayout* render_layout = new QVBoxLayout(groupbox_render_);
    render_layout->addWidget(checkbox_dept_);
    render_layout->addWidget(checkbox_cull_);
    render_layout->addWidget(checkbox_face_);
    render_layout->addWidget(checkbox_light_);
    render_layout->addWidget(checkbox_texture_);
    render_layout->addWidget(checkbox_axes_);

    QVBoxLayout* control_layout = new QVBoxLayout(groupbox_control_);
    control_layout->addWidget(lineedit_control_);
    control_layout->addWidget(pushbutton_compress_);

    QGridLayout* layer_layout = new QGridLayout;
    layer_layout->addWidget(lineedit_mask_, 0, 0);
    layer_layout->addWidget(lineedit_ppl_, 1, 0);
    layer_layout->addWidget(slider_offset_block_, 2, 0); 
    layer_layout->addWidget(slider_offset_grid_, 3, 0); 
    groupbox_layer_->setLayout(layer_layout);
}

void MeshProgram::ShowMeshInfo(int npoint, int nedge, int nface) const
{
    label_meshinfo_->setText(QString("Mesh: p: %1 e: %2 f: %3\t").arg(npoint).arg(nedge).arg(nface));
}

void MeshProgram::ControlLineEvent()
{
    emit SendCmdText(this->lineedit_control_->text());
    this->lineedit_control_->clear();
}

// NO USE
void MeshProgram::SliceCheckboxEvent(bool)
{
    return;
}

void MeshProgram::LayerSliderEvent(int)
{
    layer_config_.mask = this->lineedit_mask_->text().toStdString();
    layer_config_.num_layer = layer_config_.mask.length();
    bool ok;
    auto temp_ppl = this->lineedit_ppl_->text().toFloat(&ok);
    if (ok)
        layer_config_.ppl = temp_ppl;
    {
        auto value = static_cast<float>(slider_offset_block_->value());
        auto range = slider_offset_block_->maximum() - slider_offset_block_->minimum();
        value -= range / 2.0f;
        value /= range;
        layer_config_.offset_block = static_cast<int>(value * layer_config_.num_layer);
    }
    {
        auto value = static_cast<float>(slider_offset_grid_->value());
        auto range = slider_offset_grid_->maximum() - slider_offset_grid_->minimum();
        value -= range / 2.0f;
        value /= range;
        layer_config_.offset_grid = value;
    }
    SendLayerConfig(layer_config_);
}

void MeshProgram::LayerTextEvent()
{
    layer_config_.mask = this->lineedit_mask_->text().toStdString();
    layer_config_.num_layer = layer_config_.mask.length();
    bool ok;
    auto temp_ppl = this->lineedit_ppl_->text().toFloat(&ok);
    if (ok)
        layer_config_.ppl = temp_ppl;
    {
        auto value = static_cast<float>(slider_offset_block_->value());
        auto range = slider_offset_block_->maximum() - slider_offset_block_->minimum();
        value -= range / 2.0f;
        value /= range;
        layer_config_.offset_block = static_cast<int>(value * layer_config_.num_layer);
    }
    {
        auto value = static_cast<float>(slider_offset_grid_->value());
        auto range = slider_offset_grid_->maximum() - slider_offset_grid_->minimum();
        value -= range / 2.0f;
        value /= range;
        layer_config_.offset_grid = value;
    }
    SendLayerConfig(layer_config_);
}

void MeshProgram::OpenFile() const
{

}

void MeshProgram::ShowAbout()
{
    QMessageBox::information(this, "About QtMeshFrame-1.0.1",

        QString("<h3>This MeshFrame provides some operations about *.obj files sunch as") +
        " IO, render with points , edges, triangles or textures and some interactions with mouse."
        " A fix light source is provided for you."
        "This is a basic and raw frame for handling meshes. The mesh is of half_edge struct.\n"
        "Please contact" "<font color=blue> wkcagd@mail.ustc.edu.cn<\\font><font color=black>, Kang Wang if you has any questions.<\\font><\\h3>"
        ,
        QMessageBox::Ok);
}