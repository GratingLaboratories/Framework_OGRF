#pragma once

#include <QtWidgets/QMainWindow>
#include "SliceConfig.h"
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;
class RenderingWidget;

class MeshProgram : public QMainWindow
{
    Q_OBJECT

public:
    MeshProgram(QWidget *parent = Q_NULLPTR);

private:
    void CreateActions();
    void CreateMenus();
    void CreateToolBars();
    void CreateStatusBar();
    void CreateRenderGroup();

signals:
    void SendCmdText(const QString &);
    void SendSliceConfig(const SliceConfig &config);

public slots:
    void ShowMeshInfo(int npoint, int nedge, int nface) const;
    void ControlLineEvent();
    void SliceCheckboxEvent(bool);
    void SliceSliderEvent(int);
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

    /// SKELETON BRANCH
    /// NO NEED TO MERGE THESE CHANGE
    QAction                         *action_open_mesh;
    QAction                         *action_get_skeleton;
    QAction                         *action_open_clear;

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

    // Slice Group
    QGroupBox                       *groupbox_slice_;
    QCheckBox						*checkbox_Xrev_;
    QCheckBox						*checkbox_Yrev_;
    QCheckBox						*checkbox_Zrev_;
    QSlider                         *slider_X;
    QSlider                         *slider_Y;
    QSlider                         *slider_Z;
    SliceConfig                     slice_config_;

    // Information
    QLabel							*label_meshinfo_;
    QLabel							*label_operatorinfo_;

    RenderingWidget					*renderingwidget_;
};