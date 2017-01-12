#include "stdafx.h"
#include "renderingwidget.h"
#include <QKeyEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <iostream>
#include <QTextCodec>
#include "ArcBall.h"
#include "globalFunctions.h"
#include "HE_mesh/Vec.h"
#include <SOIL/SOIL.h>

#define updateGL update

#define DEBUG_BIG_POINT     0
#define DEBUG_COLOR_POINT   0

#define ITERATION_MINIMUM   50
#define ITERATION_LIMIT     1000
#define ITERATION_EPSILON   1e-4

#define INF                 9.9e9f

typedef trimesh::vec3  Vec3f;

RenderingWidget::RenderingWidget(QWidget *parent, MainWindow* mainwindow) :
    QOpenGLWidget(parent),
    ptr_mainwindow_(mainwindow),
    eye_distance_(5.0),
    is_draw_point_(true),
    is_draw_edge_(true),
    is_draw_face_(false),
    is_draw_texture_(false),
    is_low_poly(false),
    has_lighting_(false)
{
	ptr_arcball_ = new CArcBall(width(), height());

	is_load_texture_ = false;
	is_draw_axes_ = false;

	eye_goal_[0] = eye_goal_[1] = eye_goal_[2] = 0.0;
	eye_direction_[0] = eye_direction_[1] = 0.0;
	eye_direction_[2] = 1.0;
}

RenderingWidget::~RenderingWidget()
{
	SafeDelete(ptr_arcball_);
}

void RenderingWidget::initializeGL()
{
	glClearColor(0.15, 0.15, 0.3, 0.0);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1);

	SetLight();

}

void RenderingWidget::resizeGL(int w, int h)
{
	h = (h == 0) ? 1 : h;

	ptr_arcball_->reSetBound(w, h);

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0, GLdouble(w) / GLdouble(h), 0.001, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderingWidget::paintGL()
{
	glShadeModel(GL_SMOOTH);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (has_lighting_)
	{
		SetLight();
	}
	else
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	register vec eyepos = eye_distance_*eye_direction_;
	gluLookAt(eyepos[0], eyepos[1], eyepos[2],
		eye_goal_[0], eye_goal_[1], eye_goal_[2],
		0.0, 1.0, 0.0);
	glPushMatrix();

	glMultMatrixf(ptr_arcball_->GetBallMatrix());

	Render();
	glPopMatrix();
}

void RenderingWidget::timerEvent(QTimerEvent * e)
{
	updateGL();
}

void RenderingWidget::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		ptr_arcball_->MouseDown(e->pos());
		break;
	case Qt::MidButton:
		current_position_ = e->pos();
		break;
	default:
		break;
	}

	updateGL();
}
void RenderingWidget::mouseMoveEvent(QMouseEvent *e)
{
	switch (e->buttons())
	{
	case Qt::LeftButton:
		ptr_arcball_->MouseMove(e->pos());
		setCursor(Qt::ClosedHandCursor);
		break;
	case Qt::MidButton:
		eye_goal_[0] -= 4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width());
		eye_goal_[1] += 4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height());
		current_position_ = e->pos();
		break;
	default:
		break;
	}

	updateGL();
}
void RenderingWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		break;
	default:
		break;
	}
	updateGL();
}
void RenderingWidget::mouseReleaseEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		ptr_arcball_->MouseUp(e->pos());
		setCursor(Qt::ArrowCursor);
		break;

	case Qt::RightButton:
		break;
	default:
		break;
	}
}

void RenderingWidget::wheelEvent(QWheelEvent *e)
{
	eye_distance_ += e->delta()*0.001;
	eye_distance_ = eye_distance_ < 0 ? 0 : eye_distance_;

	updateGL();
}

void RenderingWidget::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
    case Qt::Key_Enter:
        // Fail to catch Enter use these code.
        emit(operatorInfo(QString("pressed Enter")));
        break;
    case Qt::Key_Space:
        emit(operatorInfo(QString("pressed Space")));
        break;
	default:
        emit(operatorInfo(QString("pressed ") + e->text() +
            QString("(%0)").arg(e->key())));
		break;
	}
}

void RenderingWidget::keyReleaseEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}

void RenderingWidget::Render()
{
	DrawPoints(is_draw_point_);
	DrawEdge(is_draw_edge_);
	DrawFace(is_draw_face_);
	DrawTexture(is_draw_texture_);

    DrawAxes(is_draw_axes_);
}

void RenderingWidget::SetLight()
{
	static GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat mat_shininess[] = { 50.0f };
	static GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	static GLfloat white_light[] = { 0.8f, 0.8f, 0.9f, 1.0f };
	static GLfloat lmodel_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void RenderingWidget::SetBackground()
{
	QColor color = QColorDialog::getColor(Qt::white, this, tr("background color"));
	GLfloat r = (color.red()) / 255.0f;
	GLfloat g = (color.green()) / 255.0f;
	GLfloat b = (color.blue()) / 255.0f;
	GLfloat alpha = color.alpha() / 255.0f;
	glClearColor(r, g, b, alpha);
	updateGL();
}


void mesh_unify(MyMesh &mesh, float scale = 1.0)
{
    Vec3f max_pos(-INF, -INF, -INF);
    Vec3f min_pos(+INF, +INF, +INF);

    for (auto v : mesh.vertices())
    {
        auto point = mesh.point(v);
        for (int i = 0; i < 3; i++)
        {
            float t = point[i];
            if (t > max_pos[i])
                max_pos[i] = t;
            if (t < min_pos[i])
                min_pos[i] = t;
        }
    }

    float xmax = max_pos[0], ymax = max_pos[1], zmax = max_pos[2];
    float xmin = min_pos[0], ymin = min_pos[1], zmin = min_pos[2];

    float scaleX = xmax - xmin;
    float scaleY = ymax - ymin;
    float scaleZ = zmax - zmin;
    float scaleMax;

    if (scaleX < scaleY)
    {
        scaleMax = scaleY;
    }
    else
    {
        scaleMax = scaleX;
    }
    if (scaleMax < scaleZ)
    {
        scaleMax = scaleZ;
    }

    float scaleV = scale / scaleMax;
    Vec3f center((xmin + xmax) / 2.f, (ymin + ymax) / 2.f, (zmin + zmax) / 2.f);
    for (auto v : mesh.vertices())
    {
        OpenMesh::Vec3f pt_om = mesh.point(v);
        Vec3f pt{ pt_om[0], pt_om[1], pt_om[2] };
        Vec3f res = (pt - center) * scaleV;
        OpenMesh::Vec3f res_om{ res[0], res[1], res[2] };
        mesh.set_point(v, res_om);
    }
    // REMARK: OpenMesh::Vec3f has conflict with Vec3f;
}

void RenderingWidget::ReadMesh()
{
	QString filename = QFileDialog::
		getOpenFileName(this, tr("Read Mesh"),
		"..", tr("Meshes (*.obj)"));

	if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Read Mesh Failed!")));
		return;
	}

	// 中文路径支持
	QTextCodec *code = QTextCodec::codecForName("gd18030");
    QTextCodec::setCodecForLocale(code);
    QByteArray byfilename = filename.toLocal8Bit();

    //Read Mesh
    mesh_.request_vertex_normals();
    OpenMesh::IO::Options opt;
    if (!OpenMesh::IO::read_mesh(mesh_, byfilename.data(), opt))
    {
        std::cerr << "Cannot Open mesh file." << std::endl;
        exit(0xA1);
    }

    // If the file did not provide vertex normals, then calculate them
    if (!opt.check(OpenMesh::IO::Options::VertexNormal))
    {
        // we need face normals to update the vertex normals
        mesh_.request_face_normals();
        // let the mesh update the normals
        mesh_.update_normals();
        // dispose the face normals, as we don't need them anymore
        mesh_.release_face_normals();
    }

    //Get Point,Face and edge numbers
    std::cout << "PointSize:" << mesh_.n_vertices() << std::endl;
    std::cout << "FaceSize:" << mesh_.n_faces() << std::endl;
    std::cout << "EdgeSize:" << mesh_.n_edges() << std::endl;

    mesh_unify(mesh_, 2.0f);
    mesh_.update_normals();

	//	m_pMesh->LoadFromOBJFile(filename.toLatin1().data());
	emit(operatorInfo(QString("Read Mesh from") + filename + QString(" Done")));
	emit(meshInfo(mesh_.n_vertices(), mesh_.n_edges(), mesh_.n_faces()));

	updateGL();
}

void RenderingWidget::WriteMesh()
{
	if (mesh_.n_vertices() == 0)
	{
		emit(QString("The Mesh is Empty !"));
		return;
	}
	QString filename = QFileDialog::
		getSaveFileName(this, tr("Write Mesh"),
		"..", tr("Meshes (*.obj)"));

	if (filename.isEmpty())
		return;

    // 中文路径支持
    QTextCodec *code = QTextCodec::codecForName("gd18030");
    QTextCodec::setCodecForLocale(code);
    QByteArray byfilename = filename.toLocal8Bit();
    if (!OpenMesh::IO::write_mesh(mesh_, byfilename.data()))
    {
        std::cerr << "Cannot write mesh file." << std::endl;
        exit(0xA1);
    }

	emit(operatorInfo(QString("Write Mesh to ") + filename + QString(" Done")));
}

void RenderingWidget::LoadTexture()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Load Texture"),
		"..", tr("Images(*.bmp *.jpg *.png *.jpeg)"));
	if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Load Texture Failed!")));
		return;
	}
    
    QByteArray filename_ba = filename.toLocal8Bit();
    int width, height;
    unsigned char* image = SOIL_load_image(filename_ba.data(), &width, &height, 0, SOIL_LOAD_RGB);

	glGenTextures(1, &texture_[0]);
    glBindTexture(GL_TEXTURE_2D, texture_[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height,
        GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
        height, 0, GL_RGB, GL_UNSIGNED_BYTE,
        image);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    /* original code.
	QImage tex1, buf;
	if (!buf.load(filename))
	{
		//        QMessageBox::warning(this, tr("Load Fialed!"), tr("Cannot Load Image %1").arg(filenames.at(0)));
		emit(operatorInfo(QString("Load Texture Failed!")));
		return;

	}
    tex1 = buf;
	glBindTexture(GL_TEXTURE_2D, texture_[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
		GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());
    */
	is_load_texture_ = true;
	emit(operatorInfo(QString("Load Texture from ") + filename + QString(" Done")));
}

void RenderingWidget::Convert()
{
    if (mesh_.vertices_empty())
        return;

    //GlobalMiniSurf gms;
    //gms.minimize(mesh_);
    updateGL();
    return;

}

// For reference.
//void param_show(MyMesh &mesh, ParamSurf &param, int size = 512, int bdr = 10)
//{
//    const int img_size = size;
//    const int bdr_size = bdr;
//    QImage img(img_size + 2 * bdr_size, img_size + 2 * bdr_size, QImage::Format_ARGB32);
//    QPainter ptr;
//    ptr.begin(&img);
//    ptr.setPen(Qt::white);
//    ptr.setBrush(Qt::white);
//    ptr.drawRect(0, 0, img_size + 2 * bdr_size, img_size + 2 * bdr_size);
//    QPen pt(Qt::darkBlue, 4);
//    ptr.setPen(pt);
//    for (auto v : mesh.vertices())
//    {
//        auto pos = param.query(v);
//        int x = (int)(pos[0] * img_size) + bdr_size;
//        int y = (int)(pos[1] * img_size) + bdr_size;
//        ptr.drawPoint(x, y);
//    }
//    QPen ed(Qt::blue, 1);
//    ptr.setPen(ed);
//    for (auto he : mesh.halfedges())
//    {
//        auto from = mesh.from_vertex_handle(he);
//        auto to = mesh.to_vertex_handle(he);
//        auto pf = param.query(from);
//        auto pt = param.query(to);
//        ptr.drawLine(pf[0] * img_size + bdr_size
//            , pf[1] * img_size + bdr_size
//            , pt[0] * img_size + bdr_size
//            , pt[1] * img_size + bdr_size);
//    }
//    ptr.end();
//
//    QMessageBox box;
//    box.setIconPixmap(QPixmap::fromImage(img));
//    box.show();
//    box.exec();
//}

//void RenderingWidget::Param()
//{
//    if (mesh_.vertices_empty())
//        return;
//
//    bool ok;
//    QString ori;
//    QTextStream ori_s(&ori);
//    ori_s << _method << " "
//        << _boundary;
//    QString s = QInputDialog::getText(this, tr("Settings"), tr("PARAM(1..3), BOUNDARY(4..5)"),
//        QLineEdit::Normal, ori, &ok);
//    if (ok)
//    {
//        QTextStream ss(&s);
//        ss >> _method >> _boundary;
//    }
//
//    param.setting(_method, _boundary);
//    param.init();
//    param_show(mesh_, param, 500, 10);
//
//    updateGL();
//    return;
//}

void RenderingWidget::CheckDrawPoint(bool bv)
{
	is_draw_point_ = bv;
	updateGL();
}
void RenderingWidget::CheckDrawEdge(bool bv)
{
	is_draw_edge_ = bv;
	updateGL();
}
void RenderingWidget::CheckDrawFace(bool bv)
{
	is_draw_face_ = bv;
	updateGL();
}
void RenderingWidget::CheckLight(bool bv)
{
	has_lighting_ = bv;
	updateGL();
}
void RenderingWidget::CheckDrawTexture(bool bv)
{
	is_draw_texture_ = bv;
	if (is_draw_texture_)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	updateGL();
}
void RenderingWidget::CheckDrawAxes(bool bV)
{
	is_draw_axes_ = bV;
	updateGL();
}

void RenderingWidget::CheckLowPoly(bool bv)
{
    is_low_poly = bv;
    updateGL();
}

void RenderingWidget::DrawAxes(bool bV)
{
	if (!bV)
		return;
	//x axis
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.7, 0.0, 0.0);
	glEnd();
	glPushMatrix();
	glTranslatef(0.7, 0, 0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	//y axis
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.0, 0.7, 0.0);
	glEnd();

	glPushMatrix();
	glTranslatef(0.0, 0.7, 0);
	glRotatef(90, -1.0, 0.0, 0.0);
	glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	//z axis
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.0, 0.0, 0.7);
	glEnd();
	glPushMatrix();
	glTranslatef(0.0, 0, 0.7);
	glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	glColor3f(1.0, 1.0, 1.0);
}

void RenderingWidget::DrawPoints(bool bv)
{
    if (!bv || mesh_.vertices_empty())
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (DEBUG_BIG_POINT)
    {
        glPointSize(5.0);
    }

    glBegin(GL_POINTS);

    for (auto v : mesh_.vertices())
    {
        auto pos = mesh_.point(v);
        auto nor = mesh_.normal(v);
        glNormal3fv(nor.data());
        glVertex3fv(pos.data());
    }

    glEnd();
}

void RenderingWidget::DrawEdge(bool bv)
{
    if (!bv || mesh_.vertices_empty())
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto f : mesh_.faces())
    {
        glBegin(GL_LINE_LOOP);

        auto he = mesh_.halfedge_handle(f);
        do
        {
            auto v = mesh_.to_vertex_handle(he);
            glNormal3fv(mesh_.normal(v).data());
            glVertex3fv(mesh_.point(v).data());

            he = mesh_.next_halfedge_handle(he);
        } while (he != mesh_.halfedge_handle(f));

        glEnd();
    }

    // different method [WRONG]
/*
    for (auto he : mesh_.halfedges())
    {
        glBegin(GL_LINE_LOOP);
        auto v_from = mesh_.from_vertex_handle(he);
        glVertex3fv(mesh_.point(v_from).data());
        glNormal3fv(mesh_.normal(v_from).data());

        auto v_to = mesh_.to_vertex_handle(he);
        glVertex3fv(mesh_.point(v_to).data());
        glNormal3fv(mesh_.normal(v_to).data());

        glEnd();
    }
*/
}

void RenderingWidget::DrawFace(bool bv)
{
    if (!bv || mesh_.vertices_empty())
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_TRIANGLES);

    if (!is_low_poly)
    {   // Normal
        for (auto f : mesh_.faces())
        {
            auto he = mesh_.halfedge_handle(f);
            do
            {
                auto v = mesh_.to_vertex_handle(he);
                glNormal3fv(mesh_.normal(v).data());
                glVertex3fv(mesh_.point(v).data());

                he = mesh_.next_halfedge_handle(he);
            } while (he != mesh_.halfedge_handle(f));
        }
    }
    else 
    {   // Low Poly Mode
        for (auto f : mesh_.faces())
        {
            auto he = mesh_.halfedge_handle(f);
            // Calculate the average normal of vertices of face.
            Vec3f average_normal(0, 0, 0);
            do
            {
                auto v = mesh_.to_vertex_handle(he);
                auto norm_om = mesh_.normal(v).data();
                Vec3f norm{ norm_om[0], norm_om[1], norm_om[2] };
                average_normal += norm;
                he = mesh_.next_halfedge_handle(he);
            } while (he != mesh_.halfedge_handle(f));
            // Use the average normal as the whole face.
            average_normal /= 3.0f;
            he = mesh_.halfedge_handle(f);
            do
            {
                auto v = mesh_.to_vertex_handle(he);
                glNormal3fv(average_normal.data());
                glVertex3fv(mesh_.point(v).data());

                he = mesh_.next_halfedge_handle(he);
            } while (he != mesh_.halfedge_handle(f));
        }
    }

    glEnd();
}

// No Usage in this project.
void RenderingWidget::DrawTexture(bool bv)
{
	//if (!bv)
	//	return;
	//if (mesh_.n_vertices() == 0 || !is_load_texture_)
	//	return;
 //   if (!param.has_init())
 //   {
 //       param.setting(_method, _boundary);
 //       param.init();
 //   }

	//glBindTexture(GL_TEXTURE_2D, texture_[0]);

 //   glEnable(GL_TEXTURE_2D);

	//glBegin(GL_TRIANGLES);
 //   for (auto f : mesh_.faces())
 //   {
 //       auto he = mesh_.halfedge_handle(f);
 //       do
 //       {
 //           auto v = mesh_.to_vertex_handle(he);
 //           glTexCoord2fv(param.query(v).data());
 //           glNormal3fv(mesh_.normal(v).data());
 //           glVertex3fv(mesh_.point(v).data());

 //           he = mesh_.next_halfedge_handle(he);
 //       } while (he != mesh_.halfedge_handle(f));
 //   }

	//glEnd();
}
