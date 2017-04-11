#include "stdafx.h"

#include "renderingwidget.h"
#include "meshprogram.h"

MeshProgram::MeshProgram(QWidget *parent)
    : QMainWindow(parent)
{
    renderingwidget_ = new RenderingWidget(this);
    connect(this, SIGNAL(SendSliceConfig(const SliceConfig &)), renderingwidget_, SLOT(SliceConfigChanged(const SliceConfig &)));
    //renderingwidget_->grabKeyboard();
    setCentralWidget(new QWidget);

    // Default windows SIZE, minimize.
    renderingwidget_->setMinimumHeight(800);
    renderingwidget_->setMinimumWidth(800);
    setGeometry(100, 100, 0, 0);
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
    //layout_left->addWidget(groupbox_option_);
    //layout_left->addStretch(1);
    layout_left->addWidget(groupbox_slice_);
    layout_left->addStretch(1);
    layout_left->addWidget(groupbox_control_);
    layout_left->addStretch(2);

    // Horizontal Box Layout
    QHBoxLayout *layout_main = new QHBoxLayout;

    layout_main->addLayout(layout_left);
    layout_main->addWidget(renderingwidget_);
    layout_main->setStretch(0, 1);
    layout_main->setStretch(1, 5);

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

    action_save_ = new QAction(QIcon(":/MeshCompression/Resources/images/save.png"), tr("&Save"), this);
    action_save_->setShortcuts(QKeySequence::Save);
    action_save_->setStatusTip(tr("Save the document to disk"));
    connect(action_save_, SIGNAL(triggered()), renderingwidget_, SLOT(WriteMesh()));

    action_saveas_ = new QAction(tr("Save &As..."), this);
    action_saveas_->setShortcuts(QKeySequence::SaveAs);
    action_saveas_->setStatusTip(tr("Save the document under a new name"));
    //  connect(action_saveas_, SIGNAL(triggered()), imagewidget_, SLOT(SaveAs()));

    action_loadmesh_ = new QAction(tr("open_scene"), this);
    action_loadmesh_->setIcon(QIcon(tr(":/MainWindow/open.png")));
    action_loadtexture_ = new QAction(tr("LoadTexture"), this);
    //action_background_ = new QAction(tr("ChangeBackground"), this);
    action_background_ = new QAction(tr(""), this);

    action_convert_ = new QAction(tr("Convert"), this);
    action_param_ = new QAction(tr("Param"), this);

    connect(action_loadmesh_, SIGNAL(triggered()), renderingwidget_, SLOT(ReadScene()));
    connect(action_loadtexture_, SIGNAL(triggered()), renderingwidget_, SLOT(LoadTexture()));
    connect(action_background_, SIGNAL(triggered()), renderingwidget_, SLOT(SetBackground()));
    //connect(action_convert_, SIGNAL(triggered()), renderingwidget_, SLOT(Convert()));
    //connect(action_param_, SIGNAL(triggered()), renderingwidget_, SLOT(Param()));

    /// SKELETON BRANCH
    /// NO NEED TO MERGE THESE CHANGE
    action_open_mesh = new QAction(tr("[&Open one mesh]"), this);
    connect(action_open_mesh, SIGNAL(triggered()), renderingwidget_, SLOT(OpenOneMesh()));

    action_get_skeleton = new QAction(tr("[&Skeleton]"));
    connect(action_get_skeleton, SIGNAL(triggered()), renderingwidget_, SLOT(Skeleton()));

    // action_open_clear;
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
    //toolbar_basic_->addAction(action_loadtexture_);
    toolbar_basic_->addAction(action_background_);
    toolbar_basic_->addAction(action_open_mesh);
    toolbar_basic_->addAction(action_get_skeleton);
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


    //statusBar()->addWidget(label_meshinfo_);
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

    checkbox_light_ = new QCheckBox(tr("LUNUSE"), this);
    connect(checkbox_light_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckLight(bool)));

    checkbox_texture_ = new QCheckBox(tr("TUNUSE"), this);
    connect(checkbox_texture_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawTexture(bool)));

    checkbox_axes_ = new QCheckBox(tr("AUNUSE"), this);
    connect(checkbox_axes_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawAxes(bool)));

    groupbox_render_ = new QGroupBox(tr("Render"), this);

    // Group Option
    //checkbox_lowpoly_ = new QCheckBox(tr("Low Poly"), this);
    //connect(checkbox_lowpoly_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckLowPoly(bool)));
    //checkbox_show_result_ = new QCheckBox(tr("Show Result"), this);
    //connect(checkbox_show_result_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckShowResult(bool)));
    //checkbox_show_diff_ = new QCheckBox(tr("Show Difference"), this);
    //connect(checkbox_show_diff_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckShowDiff(bool)));

    //groupbox_option_ = new QGroupBox(tr("View"), this);

    // Group Control
    pushbutton_compress_ = new QPushButton(tr("Apply"), this);
    connect(pushbutton_compress_, SIGNAL(clicked()), this, SLOT(ControlLineEvent()));

    lineedit_control_ = new QLineEdit(tr(""), this);
    lineedit_control_->setMinimumWidth(300);
    connect(lineedit_control_, SIGNAL(returnPressed()), this, SLOT(ControlLineEvent()));
    connect(this, SIGNAL(SendCmdText(const QString &)),
        renderingwidget_, SLOT(ControlLineEvent(const QString &)));

    groupbox_control_ = new QGroupBox("Control", this);

    // Group Slice
    checkbox_Xrev_ = new QCheckBox(tr("rX"), this);
    connect(checkbox_Xrev_, SIGNAL(clicked(bool)), this, SLOT(SliceCheckboxEvent(bool)));
    checkbox_Yrev_ = new QCheckBox(tr("rY"), this);
    connect(checkbox_Yrev_, SIGNAL(clicked(bool)), this, SLOT(SliceCheckboxEvent(bool)));
    checkbox_Zrev_ = new QCheckBox(tr("rZ"), this);
    connect(checkbox_Zrev_, SIGNAL(clicked(bool)), this, SLOT(SliceCheckboxEvent(bool)));
    slider_X = new QSlider(Qt::Orientation::Horizontal);
    slider_X->setMaximum(100);
    slider_X->setMinimum(0);
    connect(slider_X, SIGNAL(valueChanged(int)), this, SLOT(SliceSliderEvent(int)));
    slider_Y = new QSlider(Qt::Orientation::Horizontal);
    slider_Y->setMaximum(100);
    slider_Y->setMinimum(0);
    connect(slider_Y, SIGNAL(valueChanged(int)), this, SLOT(SliceSliderEvent(int)));
    slider_Z = new QSlider(Qt::Orientation::Horizontal);
    slider_Z->setMaximum(100);
    slider_Z->setMinimum(0);
    connect(slider_Z, SIGNAL(valueChanged(int)), this, SLOT(SliceSliderEvent(int)));

    groupbox_slice_ = new QGroupBox("Slice", this);


    // Layout for Group Boxes
    QVBoxLayout* render_layout = new QVBoxLayout(groupbox_render_);
    render_layout->addWidget(checkbox_dept_);
    render_layout->addWidget(checkbox_cull_);
    render_layout->addWidget(checkbox_face_);
    render_layout->addWidget(checkbox_texture_);
    render_layout->addWidget(checkbox_light_);
    render_layout->addWidget(checkbox_axes_);

    //QVBoxLayout* option_layout = new QVBoxLayout(groupbox_option_);
    //option_layout->addWidget(checkbox_lowpoly_);
    //option_layout->addWidget(checkbox_show_result_);
    //option_layout->addWidget(checkbox_show_diff_);

    QVBoxLayout* control_layout = new QVBoxLayout(groupbox_control_);
    control_layout->addWidget(lineedit_control_);
    control_layout->addWidget(pushbutton_compress_);

    QGridLayout* slice_layout = new QGridLayout;
    slice_layout->addWidget(checkbox_Xrev_, 0, 0);
    slice_layout->addWidget(checkbox_Yrev_, 1, 0);
    slice_layout->addWidget(checkbox_Zrev_, 2, 0);
    slice_layout->addWidget(slider_X, 0, 1);
    slice_layout->addWidget(slider_Y, 1, 1);
    slice_layout->addWidget(slider_Z, 2, 1);
    groupbox_slice_->setLayout(slice_layout);
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

void MeshProgram::SliceCheckboxEvent(bool)
{
    slice_config_.revX = this->checkbox_Xrev_->isChecked();
    slice_config_.revY = this->checkbox_Yrev_->isChecked();
    slice_config_.revZ = this->checkbox_Zrev_->isChecked();
    SendSliceConfig(slice_config_);
}

void MeshProgram::SliceSliderEvent(int)
{
    slice_config_.sliceX = static_cast<float>(this->slider_X->value()) /
        (slider_X->maximum() - slider_X->minimum());
    slice_config_.sliceY = static_cast<float>(this->slider_Y->value()) /
        (slider_Y->maximum() - slider_Y->minimum());
    slice_config_.sliceZ = static_cast<float>(this->slider_Z->value()) /
        (slider_Z->maximum() - slider_Z->minimum());
    SendSliceConfig(slice_config_);
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