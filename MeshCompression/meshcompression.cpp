#include "stdafx.h"

#include "renderingwidget.h"
#include "meshcompression.h"

MeshCompression::MeshCompression(QWidget *parent)
    : QMainWindow(parent)
{
    renderingwidget_ = new RenderingWidget(this);
    renderingwidget_->grabKeyboard();
    setCentralWidget(new QWidget);

    //setGeometry(300, 150, 800, 600);
    resize(1080, 960);

    CreateActions();
    CreateMenus();
    CreateToolBars();
    CreateStatusBar();
    CreateRenderGroup();

    QVBoxLayout *layout_left = new QVBoxLayout;
    layout_left->addWidget(groupbox_render_);
    layout_left->addStretch(2);
    layout_left->addWidget(groupbox_option_);
    layout_left->addStretch(4);

    QHBoxLayout *layout_main = new QHBoxLayout;

    layout_main->addLayout(layout_left);
    layout_main->addWidget(renderingwidget_);
    layout_main->setStretch(1, 1);

    centralWidget()->setLayout(layout_main);

    toolbar_file_->setVisible(false);
}

void MeshCompression::CreateActions()
{
    action_new_ = new QAction(QIcon(":/MeshCompression/Resources/images/new.png"), tr("&New"), this);
    action_new_->setShortcut(QKeySequence::New);
    action_new_->setStatusTip(tr("Create a new file"));

    action_open_ = new QAction(QIcon(":/MeshCompression/Resources/images/open.png"), tr("&Open..."), this);
    action_open_->setShortcuts(QKeySequence::Open);
    action_open_->setStatusTip(tr("Open an existing file"));
    connect(action_open_, SIGNAL(triggered()), renderingwidget_, SLOT(ReadMesh()));

    action_save_ = new QAction(QIcon(":/MeshCompression/Resources/images/save.png"), tr("&Save"), this);
    action_save_->setShortcuts(QKeySequence::Save);
    action_save_->setStatusTip(tr("Save the document to disk"));
    connect(action_save_, SIGNAL(triggered()), renderingwidget_, SLOT(WriteMesh()));

    action_saveas_ = new QAction(tr("Save &As..."), this);
    action_saveas_->setShortcuts(QKeySequence::SaveAs);
    action_saveas_->setStatusTip(tr("Save the document under a new name"));
    //  connect(action_saveas_, SIGNAL(triggered()), imagewidget_, SLOT(SaveAs()));

    action_loadmesh_ = new QAction(tr("readOBJ"), this);
    action_loadtexture_ = new QAction(tr("LoadTexture"), this);
    action_background_ = new QAction(tr("ChangeBackground"), this);

    action_convert_ = new QAction(tr("Convert"), this);
    action_param_ = new QAction(tr("Param"), this);

    connect(action_loadmesh_, SIGNAL(triggered()), renderingwidget_, SLOT(ReadMesh()));
    connect(action_loadtexture_, SIGNAL(triggered()), renderingwidget_, SLOT(LoadTexture()));
    connect(action_background_, SIGNAL(triggered()), renderingwidget_, SLOT(SetBackground()));
    connect(action_convert_, SIGNAL(triggered()), renderingwidget_, SLOT(Convert()));
    connect(action_param_, SIGNAL(triggered()), renderingwidget_, SLOT(Param()));
}

void MeshCompression::CreateMenus()
{
    menu_file_ = menuBar()->addMenu(tr("&File"));
    menu_file_->setStatusTip(tr("File menu"));
    menu_file_->addAction(action_new_);
    menu_file_->addAction(action_open_);
    menu_file_->addAction(action_save_);
    menu_file_->addAction(action_saveas_);
}

void MeshCompression::CreateToolBars()
{
    toolbar_file_ = addToolBar(tr("File"));
    toolbar_file_->addAction(action_new_);
    toolbar_file_->addAction(action_open_);
    toolbar_file_->addAction(action_save_);

    toolbar_basic_ = addToolBar(tr("Basic"));
    toolbar_basic_->addAction(action_loadmesh_);
    //toolbar_basic_->addAction(action_loadtexture_);
    toolbar_basic_->addAction(action_background_);
    //toolbar_basic_->addAction(action_convert_);
    //toolbar_basic_->addAction(action_param_);
}

void MeshCompression::CreateStatusBar()
{
    label_meshinfo_ = new QLabel(QString("Mesh: p: %1 e: %2 f: %3\t").arg(0).arg(0).arg(0));
    label_meshinfo_->setAlignment(Qt::AlignCenter);
    label_meshinfo_->setMinimumSize(label_meshinfo_->sizeHint());

    label_operatorinfo_ = new QLabel();
    label_operatorinfo_->setAlignment(Qt::AlignVCenter);


    statusBar()->addWidget(label_meshinfo_);
    connect(renderingwidget_, SIGNAL(meshInfo(int, int, int)), this, SLOT(ShowMeshInfo(int, int, int)));

    statusBar()->addWidget(label_operatorinfo_);
    connect(renderingwidget_, SIGNAL(operatorInfo(QString)), label_operatorinfo_, SLOT(setText(QString)));
}

void MeshCompression::CreateRenderGroup()
{
    checkbox_point_ = new QCheckBox(tr("Point"), this);
    connect(checkbox_point_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawPoint(bool)));
    checkbox_point_->setChecked(true);

    checkbox_edge_ = new QCheckBox(tr("Edge"), this);
    connect(checkbox_edge_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawEdge(bool)));
    checkbox_edge_->setChecked(true);

    checkbox_face_ = new QCheckBox(tr("Face"), this);
    connect(checkbox_face_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawFace(bool)));

    checkbox_light_ = new QCheckBox(tr("Light"), this);
    connect(checkbox_light_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckLight(bool)));

    checkbox_texture_ = new QCheckBox(tr("Texture"), this);
    connect(checkbox_texture_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawTexture(bool)));

    checkbox_axes_ = new QCheckBox(tr("Axes"), this);
    connect(checkbox_axes_, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckDrawAxes(bool)));

    groupbox_render_ = new QGroupBox(tr("Render"), this);

    checkbox_lowpoly = new QCheckBox(tr("Low Poly"), this);
    connect(checkbox_lowpoly, SIGNAL(clicked(bool)), renderingwidget_, SLOT(CheckLowPoly(bool)));

    groupbox_option_ = new QGroupBox(tr("Options"), this);

    QVBoxLayout* render_layout = new QVBoxLayout(groupbox_render_);
    render_layout->addWidget(checkbox_point_);
    render_layout->addWidget(checkbox_edge_);
    render_layout->addWidget(checkbox_face_);
    render_layout->addWidget(checkbox_texture_);
    render_layout->addWidget(checkbox_light_);
    render_layout->addWidget(checkbox_axes_);

    QVBoxLayout* option_layout = new QVBoxLayout(groupbox_option_);
    option_layout->addWidget(checkbox_lowpoly);
}

void MeshCompression::keyPressEvent(QKeyEvent *e)
{

}

void MeshCompression::keyReleaseEvent(QKeyEvent *e)
{

}

void MeshCompression::ShowMeshInfo(int npoint, int nedge, int nface)
{
    label_meshinfo_->setText(QString("Mesh: p: %1 e: %2 f: %3\t").arg(npoint).arg(nedge).arg(nface));
}

void MeshCompression::OpenFile()
{

}

void MeshCompression::ShowAbout()
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