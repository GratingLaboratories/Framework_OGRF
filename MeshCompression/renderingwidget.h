#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QOpenGLWidget>

#include "HE_mesh/Vec.h"
#include "CompressionSolution.h"
#include "ConsoleMessageManager.h"
#include "OpenGLCamera.h"
#include <memory>
#include "OpenGLMesh.h"
#include "OpenGLScene.h"

using trimesh::vec;
using trimesh::point;

typedef OpenMesh::TriMesh_ArrayKernelT<>  TriMesh;

class MainWindow;
class CArcBall;
class Mesh3D;

class RenderingWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	RenderingWidget(QWidget *parent, MainWindow* mainwindow=0);
	~RenderingWidget();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	// mouse events
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

public:
	void keyPressEvent(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);

signals:
	void meshInfo(int, int, int);
	void operatorInfo(QString);

private:
	void Render();
	void SetLight();

public slots:
	void SetBackground();
	void ReadMesh();
	void WriteMesh();
	void LoadTexture();
    void Compress();
    void ChangePrecision(const QString &text);

	void CheckDrawPoint(bool bv);
	void CheckDrawEdge(bool bv);
	void CheckDrawFace(bool bv);
	void CheckLight(bool bv);
	void CheckDrawTexture(bool bv);
	void CheckDrawAxes(bool bv);
    void CheckLowPoly(bool bv);
    void CheckShowResult(bool bv);
    void CheckShowDiff(bool bv);
    
private:
	void DrawAxes(bool bv);
	void DrawPoints(bool);
	void DrawEdge(bool);
	void DrawFace(bool);
	void DrawTexture(bool);

private slots:
    void timerEvent();

public:
	MainWindow					*ptr_mainwindow_;
	CArcBall					*ptr_arcball_;
    TriMesh                      mesh_;

	// Texture
	GLuint						texture_[1];
	bool						is_load_texture_;

	// eye
	GLfloat						eye_distance_;
	point						eye_goal_;
	vec							eye_direction_;
	QPoint						current_position_;

	// Render information
	bool						is_draw_point_;
	bool						is_draw_edge_;
	bool						is_draw_face_;
	bool						is_draw_texture_;
	bool						has_lighting_;
	bool						is_draw_axes_;
    bool                        is_low_poly_;
    bool                        is_show_result_;
    bool                        is_show_diff_;

private:
    QColor                      background_color_;
    int                         precision_;
    PositionMap                 position_map_;
    DifferenceMap               difference_map_;
    float                       max_difference_;
    bool                        compress_ok_;
    ConsoleMessageManager       msg;

    int                         frame_rate_limit;
    float                       fps;
    QTime                       last_time;
    QTime                       init_time;
    QTimer                     *timer;
    QOpenGLShaderProgram       *shader_program_basic_phong_;
    QOpenGLBuffer              *vbo, *veo;
    QOpenGLVertexArrayObject   *vao;
    OpenGLCamera                camera_;
    OpenGLMesh                  test;
    OpenGLScene                 scene;
    bool                        light_dir_fix_;
};

#endif // RENDERINGWIDGET_H
