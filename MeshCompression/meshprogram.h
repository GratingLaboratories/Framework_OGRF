#pragma once

#include <QtWidgets/QMainWindow>

class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;
class RenderingWidget;

class MeshCompression : public QMainWindow
{
    Q_OBJECT

public:
    MeshCompression(QWidget *parent = Q_NULLPTR);

private:
    void CreateActions();
    void CreateMenus();
    void CreateToolBars();
    void CreateStatusBar();
    void CreateRenderGroup();

//protected:
//    void keyPressEvent(QKeyEvent *e);
//    void keyReleaseEvent(QKeyEvent *e);

signals:
    void SendCmdText(const QString &);

public slots:
    void ShowMeshInfo(int npoint, int nedge, int nface) const;
    void ControlLineEvent();
    void OpenFile() const;
    void ShowAbout();

private:

    // Basic
    QMenu							*menu_file_;
    QMenu							*menu_edit_;
    QMenu							*menu_help_;
    QToolBar						*toolbar_file_;
    QToolBar						*toolbar_edit_;
    QToolBar						*toolbar_basic_;
    QAction							*action_new_;
    QAction							*action_open_;
    QAction							*action_save_;
    QAction							*action_saveas_;

    QAction							*action_aboutqt_;
    QAction							*action_about_;

    // Basic Operator Tool
    QAction							*action_loadmesh_;
    QAction							*action_loadtexture_;
    QAction							*action_background_;

    QAction                         *action_convert_;
    QAction                         *action_param_;

    // Render CheckBoxs
    QCheckBox						*checkbox_dept_;
    QCheckBox						*checkbox_cull_;
    QCheckBox						*checkbox_face_;
    QCheckBox						*checkbox_light_;
    QCheckBox						*checkbox_texture_;
    QCheckBox						*checkbox_axes_;

    // Option CheckBoxs
    QCheckBox                       *checkbox_lowpoly_;
    QCheckBox                       *checkbox_show_result_;  
    QCheckBox                       *checkbox_show_diff_;    

    // Control
    QPushButton                     *pushbutton_compress_;
    QLineEdit                       *lineedit_control_; 

    QGroupBox						*groupbox_render_;
    QGroupBox						*groupbox_option_;
    QGroupBox                       *groupbox_control_;

    // Information
    QLabel							*label_meshinfo_;
    QLabel							*label_operatorinfo_;

    RenderingWidget					*renderingwidget_;
};