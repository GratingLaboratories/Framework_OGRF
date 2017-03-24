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
#include "CompressionSolution.h"

#include "GlobalConfig.h"
//#include "PsudoColorRGB.h"

#define updateGL update

#define DEBUG_BIG_POINT         false
#define DEBUG_COLOR_POINT       false

#define ITERATION_MINIMUM       50
#define ITERATION_LIMIT         1000
#define ITERATION_EPSILON       1e-4

#define INF                     9.9e9f

typedef trimesh::vec3  Vec3f;
using Vec3f_om = OpenMesh::Vec3f;

RenderingWidget::RenderingWidget(QWidget *parent, MainWindow* mainwindow) :
    QOpenGLWidget(parent),
    ptr_mainwindow_(mainwindow),
    is_draw_point_(true),
    is_draw_edge_(true),
    is_draw_face_(true),
    is_draw_texture_(false),
    has_lighting_(false),
    is_low_poly_(false),
    is_show_result_(false),
    is_show_diff_(false),
    background_color_(32, 64, 128),
    precision_(100),
    max_difference_(0),
    compress_ok_(false),
    msg(std::cout),
    frame_rate_limit(FPS_LIMIT),
    fps(0),
    scene(msg),
    light_dir_fix_(false),
    sim(nullptr)
{
    // Set the focus policy to Strong, 
    // then the renderingWidget can accept keyboard input event and response.
    setFocusPolicy(Qt::StrongFocus);

	is_load_texture_ = false;
	is_draw_axes_ = false;

	eye_goal_[0] = eye_goal_[1] = eye_goal_[2] = 0.0;
	eye_direction_[0] = eye_direction_[1] = 0.0;
	eye_direction_[2] = 1.0;

    //msg.enable(TRIVIAL_MSG);
    msg.enable(BUFFER_INFO_MSG);

    init_time.start();

    last_time = QTime::currentTime();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));

    timer->setSingleShot(true);
    timer->start();
}

RenderingWidget::~RenderingWidget()
{
    SafeDelete(timer);
    makeCurrent();
    vbo->destroy();
    doneCurrent();
}

void RenderingWidget::initializeGL()
{
    msg.log("initializeGL()", TRIVIAL_MSG);

    initializeOpenGLFunctions();

    QString vertexShaderFileName{ "shader/BasicPhongVertexShader.vertexshader" };
    QString fragmentShaderFileName{ "shader/BasicPhongFragmentShader.fragmentshader" };

    // Read shader source code from files.
    QFile vertexShaderFile{ vertexShaderFileName };
    vertexShaderFile.open(QFile::ReadOnly | QFile::Text);
    QTextStream tsv{ &vertexShaderFile };
    QString vertexShaderSource{ tsv.readAll() };
    vertexShaderFile.close();

    QFile fragmentShaderFile{ fragmentShaderFileName };
    fragmentShaderFile.open(QFile::ReadOnly | QFile::Text);
    QTextStream tsf{ &fragmentShaderFile };
    QString fragmentShaderSource{ tsf.readAll() };
    fragmentShaderFile.close();

    shader_program_basic_phong_ = new QOpenGLShaderProgram(this);
    shader_program_basic_phong_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shader_program_basic_phong_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shader_program_basic_phong_->link();

    vao = new QOpenGLVertexArrayObject();
    vao->create();

    vao->bind();
    {
        // vertex buffer.
        vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        vbo->create();
        vbo->bind();

        // element index buffer
        veo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        veo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        veo->create();
        veo->bind();

        shader_program_basic_phong_->enableAttributeArray(0);
        shader_program_basic_phong_->setAttributeBuffer(0, GL_FLOAT, 0, 3, 9 * sizeof(GLfloat));
        shader_program_basic_phong_->enableAttributeArray(1);
        shader_program_basic_phong_->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 9 * sizeof(GLfloat));
        shader_program_basic_phong_->enableAttributeArray(2);
        shader_program_basic_phong_->setAttributeBuffer(2, GL_FLOAT, 6 * sizeof(GLfloat), 3, 9 * sizeof(GLfloat));
    }
    vao->release();

    camera_ = OpenGLCamera(DEFAULT_CAMERA_POSITION, { 0.0f, 0.0f, 0.0f });
}

void RenderingWidget::resizeGL(int w, int h)
{
    msg.log(QString("resizeGL() with size w=%0, h=%1").arg(w).arg(h), TRIVIAL_MSG);
}

void RenderingWidget::paintGL()
{
    msg.log(QString("printGL()"), TRIVIAL_MSG);

    // OpenGL work.
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    if (is_draw_point_)
    {
        glEnable(GL_DEPTH_TEST);
    }
    if (is_draw_edge_)
    {
        glEnable(GL_CULL_FACE);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    glClearColor(background_color_.redF(),
        background_color_.greenF(),
        background_color_.blueF(),
        1.0f);

    // Wire-frame mode.
    // Any subsequent drawing calls will render the triangles in 
    // wire-frame mode until we set it back to its default using 
    // `glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)`.
    if (!is_draw_face_)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    if (scene.changed())
    {
        vao->bind();
            vbo->bind();
                vbo->allocate(scene.vbuffer.data(), scene.vbuffer.size() * sizeof(GLfloat));
                veo->allocate(scene.ebuffer.data(), scene.ebuffer.size() * sizeof(GLuint));
            vbo->release();
        vao->release();        
    }

    shader_program_basic_phong_->bind();
    {
        vao->bind();
        {
            QMatrix4x4 mat_model; 
            
            QMatrix4x4 mat_projection;
            mat_projection.perspective(45.0f,
                float(this->width()) / float(this->height()),
                0.1f, 100.f);

            shader_program_basic_phong_->setUniformValue("model", mat_model);
            shader_program_basic_phong_->setUniformValue("view", camera_.view_mat());
            shader_program_basic_phong_->setUniformValue("projection", mat_projection);
            if (light_dir_fix_)
                shader_program_basic_phong_->setUniformValue("lightDirFrom", 1.0f, 1.0f, 1.0f);
            else
                shader_program_basic_phong_->setUniformValue("lightDirFrom", camera_.direction());
            shader_program_basic_phong_->setUniformValue("viewPos", camera_.position());

            glDrawElements(GL_TRIANGLES, scene.ebuffer.size(), GL_UNSIGNED_INT, (GLvoid *)0);
        }
        vao->release();
    }

    // TODO
    //glLineWidth(2.5);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(10.0, 0.0, 0.0);
    glEnd();
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 10.0, 0.0);
    glEnd();

    shader_program_basic_phong_->release();

    // Restore Polygon Mode to ensure the correctness of native painter
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE | GL_DEPTH_TEST);
    
    // Native Painter work.
    QPainter painter(this);
    // Show FPS.
    painter.setPen(Qt::white);
    painter.setFont(QFont{ "PT Mono", 12 });
    painter.drawText(this->width() / 15, this->height() / 15,
        QString("FPS: %0").arg(fps, 0, 'f', 1));
    if (sim != nullptr)
    {
        painter.drawText(this->width() / 15, this->height() / 15 * 2,
        QString("t=%0s").arg(sim->get_time(), 0, 'f', 3));
    }
    painter.end();
}

// calculate FPS,
// take a current fps value to update
// return the average of previous values
float update_fps(float this_fps)
{
    static const int fps_stat_cnt = FPS_UPDATE_STEP;
    static float fps_vec[fps_stat_cnt] = { 0 };
    static float old_fps = 0.0f;
    static int count = 0;
    for (int i = 0; i < fps_stat_cnt - 1; i++)
        fps_vec[i] = fps_vec[i + 1];
    fps_vec[fps_stat_cnt - 1] = this_fps;
    float sum = 0;
    for (int i = 0; i < fps_stat_cnt; i++)
        sum += fps_vec[i];
    if (++count == fps_stat_cnt)
    {
        old_fps = sum / fps_stat_cnt;
        count = 0;
    }
    return old_fps;
}

void RenderingWidget::timerEvent()
{
    // Calculate delta time.
    auto delta_time = last_time.msecsTo(QTime::currentTime());

    // Update FPS
    fps = update_fps(1000.0 / delta_time);
    // Emit msg of delta time (TRIVIAL level)
    msg.log(QString("\rdelta time = %0 ms")
        .arg(last_time.msecsTo(QTime::currentTime()))
        , TRIVIAL_MSG);
    // State current time as `last_time`
    last_time = QTime::currentTime();
    
    // Screen-Shot
    if (sim != nullptr)
    {
        auto t = sim->get_time();
        if (SCREEN_SHOT_ENABLE &&
            frame >= SCREEN_SHOT_FRAME_BEGIN && 
            frame <= SCREEN_SHOT_FRAME_END && 
            frame % SCREEN_SHOT_FRAME_STEP == 0)
        {
            QImage image = grabFramebuffer();
            image.save(QString("images/%0/%1.png")
                .arg("spring").arg(frame / SCREEN_SHOT_FRAME_STEP), "PNG");
            emit(operatorInfo(QString("Screen-Shot at frame %0").arg(frame / SCREEN_SHOT_FRAME_STEP)));
        }
        sim->simulate(t + 0.0001f); // current time, in fact.
        frame++;
    }

	updateGL();

    // re-start timer for next frame event
    timer->start(1000 / frame_rate_limit + MINIMUN_FRAME_STEP_MS);
}

void RenderingWidget::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		//ptr_arcball_->MouseDown(e->pos());
        current_position_ = e->pos();
        setCursor(Qt::ClosedHandCursor);
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
    bool has_shift = e->modifiers() & Qt::ShiftModifier;
    bool has_ctrl = e->modifiers() & Qt::ControlModifier;
    bool has_alt = e->modifiers() & Qt::AltModifier;
    float multiplier = 1.0;
    multiplier *= has_shift ? 0.25f : 1.0f;
    multiplier *= has_ctrl ?  3.0f : 1.0f;
	switch (e->buttons())
	{
	case Qt::LeftButton:
		//ptr_arcball_->MouseMove(e->pos());
        if (has_alt)
        {
            camera_.move_right_target(-4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width()) * multiplier);
            camera_.move_up_target(+4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height()) * multiplier);
        }
        else
        {
            camera_.move_around_right(-180.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width()) * multiplier);
            camera_.move_around_up(+180.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height()) * multiplier);
        }
            current_position_ = e->pos();
		break;
	case Qt::MidButton:
		camera_.move_right_target(-4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width()) * multiplier);
        camera_.move_up_target(+4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height()) * multiplier);
		current_position_ = e->pos();
		break;
	default:
		break;
	}
	updateGL();
}
void RenderingWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	//switch (e->button())
	//{
	//case Qt::LeftButton:
	//	break;
	//default:
	//	break;
	//}
	//updateGL();
}
void RenderingWidget::mouseReleaseEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
	    //ptr_arcball_->MouseUp(e->pos());
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
    bool has_shift = e->modifiers() & Qt::ShiftModifier;
    bool has_ctrl = e->modifiers() & Qt::ControlModifier;
    float multiplier = 1.0;
    multiplier *= has_shift ? 0.25f : 1.0f;
    multiplier *= has_ctrl ? 3.0f : 1.0f;
    camera_.move_back(- e->delta()*0.0015 * multiplier);

	updateGL();
}

void RenderingWidget::keyPressEvent(QKeyEvent *e)
{
    bool has_shift = e->modifiers() & Qt::ShiftModifier;
    bool has_ctrl = e->modifiers() & Qt::ControlModifier;
    float multiplier = 1.0;
    multiplier *= has_shift ? 0.25f : 1.0f;
    multiplier *= has_ctrl ? 3.0f : 1.0f;

    float angle = 10.0f * multiplier;
    float dis = 0.2f * multiplier;

    switch (e->key())
    {
    case Qt::Key_Enter:
        // Fail to catch Enter use these code.
        emit(operatorInfo(QString("pressed Enter")));
        break;
    case Qt::Key_Space:
        emit(operatorInfo(QString("pressed Space")));
        break;

    case Qt::Key_Left:
        emit(operatorInfo(QString("camera move_around_left  %0 degrees").arg(angle)));
        camera_.move_around_right(-angle);
        break;
    case Qt::Key_Right:
        emit(operatorInfo(QString("camera move_around_right %0 degrees").arg(angle)));
        camera_.move_around_right(+angle);
        break;
    case Qt::Key_Up:
        emit(operatorInfo(QString("camera move_around_up    %0 degrees").arg(angle)));
        camera_.move_around_up(+angle);
        break;
    case Qt::Key_Down:
        emit(operatorInfo(QString("camera move_around_down  %0 degrees").arg(angle)));
        camera_.move_around_up(-angle);
        break;

    case Qt::Key_W:
        emit(operatorInfo(QString("target move_around_up    %0 degrees").arg(angle)));
        camera_.move_around_up_target(+angle);
        break;
    case Qt::Key_A:
        emit(operatorInfo(QString("target move_around_left  %0 degrees").arg(angle)));
        camera_.move_around_right_target(-angle);
        break;
    case Qt::Key_S:
        emit(operatorInfo(QString("target move_around_down  %0 degrees").arg(angle)));
        camera_.move_around_up_target(-angle);
        break;
    case Qt::Key_D:
        emit(operatorInfo(QString("target move_around_right %0 degrees").arg(angle)));
        camera_.move_around_right_target(+angle);
        break;
    case Qt::Key_Equal:
    case Qt::Key_Plus:
        emit(operatorInfo(QString("move front")));
        camera_.move_back(-dis);
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        emit(operatorInfo(QString("move back ")));
        camera_.move_back(+dis);
        break;
    case Qt::Key_R:
        emit(operatorInfo(QString("reset camera")));
        camera_ = OpenGLCamera(DEFAULT_CAMERA_POSITION, { 0, 0, 0 });
        break;
    case Qt::Key_L:
        emit(operatorInfo(QString("light fix switch")));
        light_dir_fix_ = !light_dir_fix_;
        break;

    default:
        emit(operatorInfo(QString("pressed ") + e->text() +
            QString("(%0)").arg(e->key())));
        break;
    }
}

void RenderingWidget::keyReleaseEvent(QKeyEvent *e)
{
	//switch (e->key())
	//{
	//case Qt::Key_A:
	//	break;
	//default:
	//	break;
	//}
}

void RenderingWidget::Render()
{
    /* TRAIL branch */
    /*
	DrawPoints(is_draw_point_);
	DrawEdge(is_draw_edge_);
	DrawFace(is_draw_face_);
	DrawTexture(is_draw_texture_);

    DrawAxes(is_draw_axes_);
    */
}

void RenderingWidget::SetLight()
{
	//static GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//static GLfloat mat_shininess[] = { 50.0f };
	//static GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	//static GLfloat white_light[] = { 0.8f, 0.8f, 0.9f, 1.0f };
	//static GLfloat lmodel_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };

	//glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	//glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
}

void RenderingWidget::SetBackground()
{
	QColor color = QColorDialog::getColor(Qt::white, this, tr("background color"));
	GLfloat r = (color.red()) / 255.0f;
	GLfloat g = (color.green()) / 255.0f;
	GLfloat b = (color.blue()) / 255.0f;
	GLfloat alpha = color.alpha() / 255.0f;
	glClearColor(r, g, b, alpha);
    background_color_ = color;

	updateGL();
}

void mesh_unify(TriMesh &mesh, float scale = 1.0)
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

void RenderingWidget::ReadScene()
{
	QString filename = QFileDialog::
		getOpenFileName(this, tr("Read Scene"),
		"./scene", tr("Scene Files (*.scene)"));

	if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Read Scene Failed!")));
		return;
	}

	// 中文路径支持
	QTextCodec *code = QTextCodec::codecForName("gd18030");
    QTextCodec::setCodecForLocale(code);
    QByteArray byfilename = filename.toLocal8Bit();

    scene.open(filename);

    //sim = new SimulatorSimpleSpring(scene);
    //sim = new SimulatorSimpleFED(scene);
    //sim->init(0.0f);

    //test = OpenGLMesh();
    //test.file_name_ = filename;
    //test.need_scale_ = true;
    //test.need_centralize_ = true;
    //test.scale_ = 1.0f;
    //test.init();

    frame = 0;
	updateGL();
}

void RenderingWidget::WriteMesh()
{
	//if (mesh_.n_vertices() == 0)
	//{
	//	emit(QString("The Mesh is Empty !"));
	//	return;
	//}
	//QString filename = QFileDialog::
	//	getSaveFileName(this, tr("Write Mesh"),
	//	"..", tr("Meshes (*.obj)"));

	//if (filename.isEmpty())
	//	return;

 //   // 中文路径支持
 //   QTextCodec *code = QTextCodec::codecForName("gd18030");
 //   QTextCodec::setCodecForLocale(code);
 //   QByteArray byfilename = filename.toLocal8Bit();
 //   if (!OpenMesh::IO::write_mesh(mesh_, byfilename.data()))
 //   {
 //       std::cerr << "Cannot write mesh file." << std::endl;
 //       exit(0xA1);
 //   }

	//emit(operatorInfo(QString("Write Mesh to ") + filename + QString(" Done")));
}

void RenderingWidget::LoadTexture()
{
//	QString filename = QFileDialog::getOpenFileName(this, tr("Load Texture"),
//		"..", tr("Images(*.bmp *.jpg *.png *.jpeg)"));
//	if (filename.isEmpty())
//	{
//		emit(operatorInfo(QString("Load Texture Failed!")));
//		return;
//	}
//    
//    QByteArray filename_ba = filename.toLocal8Bit();
//    int width, height;
//    unsigned char* image = SOIL_load_image(filename_ba.data(), &width, &height, 0, SOIL_LOAD_RGB);
//
//	glGenTextures(1, &texture_[0]);
//    glBindTexture(GL_TEXTURE_2D, texture_[0]);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height,
//        GL_RGBA, GL_UNSIGNED_BYTE, image);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
//        height, 0, GL_RGB, GL_UNSIGNED_BYTE,
//        image);
//    SOIL_free_image_data(image);
//    glBindTexture(GL_TEXTURE_2D, 0);
//    
//    /* original code.
//	QImage tex1, buf;
//	if (!buf.load(filename))
//	{
//		//        QMessageBox::warning(this, tr("Load Fialed!"), tr("Cannot Load Image %1").arg(filenames.at(0)));
//		emit(operatorInfo(QString("Load Texture Failed!")));
//		return;
//
//	}
//    tex1 = buf;
//	glBindTexture(GL_TEXTURE_2D, texture_[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
//		GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());
//    */
//	is_load_texture_ = true;
//	emit operatorInfo(QString("Load Texture from ") + filename + QString(" Done"));
}

void RenderingWidget::ControlLineEvent(const QString &cmd_text)
{
    emit operatorInfo(cmd_text);
}
// For reference.
//void param_show(TriMesh &mesh, ParamSurf &param, int size = 512, int bdr = 10)
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
    is_low_poly_ = bv;
    updateGL();
}

void RenderingWidget::CheckShowResult(bool bv)
{
    is_show_result_ = bv;
    updateGL();
}

void RenderingWidget::CheckShowDiff(bool bv)
{
    is_show_diff_ = bv;
    updateGL();
}

//void RenderingWidget::DrawAxes(bool bV)
//{
////	if (!bV)
////		return;
////	//x axis
////	glColor3f(1.0, 0.0, 0.0);
////	glBegin(GL_LINES);
////	glVertex3f(0, 0, 0);
////	glVertex3f(0.7, 0.0, 0.0);
////	glEnd();
////	glPushMatrix();
////	glTranslatef(0.7, 0, 0);
////	glRotatef(90, 0.0, 1.0, 0.0);
////	glutSolidCone(0.02, 0.06, 20, 10);
////	glPopMatrix();
////
////	//y axis
////	glColor3f(0.0, 1.0, 0.0);
////	glBegin(GL_LINES);
////	glVertex3f(0, 0, 0);
////	glVertex3f(0.0, 0.7, 0.0);
////	glEnd();
////
////	glPushMatrix();
////	glTranslatef(0.0, 0.7, 0);
////	glRotatef(90, -1.0, 0.0, 0.0);
////	glutSolidCone(0.02, 0.06, 20, 10);
////	glPopMatrix();
////
////	//z axis
////	glColor3f(0.0, 0.0, 1.0);
////	glBegin(GL_LINES);
////	glVertex3f(0, 0, 0);
////	glVertex3f(0.0, 0.0, 0.7);
////	glEnd();
////	glPushMatrix();
////	glTranslatef(0.0, 0, 0.7);
////	glutSolidCone(0.02, 0.06, 20, 10);
////	glPopMatrix();
////
////	glColor3f(1.0, 1.0, 1.0);
////}
////
////void RenderingWidget::DrawPoints(bool bv)
////{
////    if (!bv || mesh_.vertices_empty())
////        return;
////
////    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
////
////    if (DEBUG_BIG_POINT)
////    {
////        glPointSize(5.0);
////    }
////
////    glBegin(GL_POINTS);
////
////    for (auto v : mesh_.vertices())
////    {
////        auto pos = mesh_.point(v);
////        if (is_show_result_ && compress_ok_)
////            pos = position_map_[v];
////        auto nor = mesh_.normal(v);
////        glNormal3fv(nor.data());
////        glVertex3fv(pos.data());
////    }
////
////    glEnd();
////}
////
////void RenderingWidget::DrawEdge(bool bv)
////{
////    if (!bv || mesh_.vertices_empty())
////        return;
////
////    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
////
////    for (auto f : mesh_.faces())
////    {
////        glBegin(GL_LINE_LOOP);
////
////        auto he = mesh_.halfedge_handle(f);
////        do
////        {
////            auto v = mesh_.to_vertex_handle(he);
////            glNormal3fv(mesh_.normal(v).data());
////            if (is_show_result_ && compress_ok_)
////                glVertex3fv(position_map_[v].data());
////            else
////                glVertex3fv(mesh_.point(v).data());
////
////            he = mesh_.next_halfedge_handle(he);
////        } while (he != mesh_.halfedge_handle(f));
////
////        glEnd();
////    }
////
////    // different method [WRONG]
/////*
////    for (auto he : mesh_.halfedges())
////    {
////        glBegin(GL_LINE_LOOP);
////        auto v_from = mesh_.from_vertex_handle(he);
////        glVertex3fv(mesh_.point(v_from).data());
////        glNormal3fv(mesh_.normal(v_from).data());
////
////        auto v_to = mesh_.to_vertex_handle(he);
////        glVertex3fv(mesh_.point(v_to).data());
////        glNormal3fv(mesh_.normal(v_to).data());
////
////        glEnd();
////    }
////*/
//}
//
//void RenderingWidget::DrawFace(bool bv)
//{
//    //if (!bv || mesh_.vertices_empty())
//    //    return;
//
//    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    //glBegin(GL_TRIANGLES);
//
//    //if (is_show_result_ && compress_ok_)
//    //{   // Show Result Mode
//    //    for (auto f : mesh_.faces())
//    //    {
//    //        auto he = mesh_.halfedge_handle(f);
//    //        do
//    //        {
//    //            auto v = mesh_.to_vertex_handle(he);
//    //            glNormal3fv(mesh_.normal(v).data());
//    //            glVertex3fv(position_map_[v].data());
//
//    //            he = mesh_.next_halfedge_handle(he);
//    //        } while (he != mesh_.halfedge_handle(f));
//    //    }
//    //}
//    //else if (is_show_diff_ && compress_ok_)
//    //{   // Show diff Mode
//    //    CPseudoColorRGB  Psdc;  // 定义计算colormap对象
//
//    //    Psdc.SetPCRamp(0.0, 1.0);
//    //    Psdc.SetPCType(PCT_JET);    // 变换类型：红绿蓝
//    //    Psdc.SetPCValueRange(0.0, 1.0); // 定义范围
//
//    //    // 示范：如何计算值value所对应的rgb颜色
//    //    double color[3];
//    //    double value;
//    //    //Psdc.GetPC(color, value);  // 返回的color值范围为[0,255]
//
//    //    for (auto f : mesh_.faces())
//    //    {
//    //        auto he = mesh_.halfedge_handle(f);
//    //        do
//    //        {
//    //            auto v = mesh_.to_vertex_handle(he);
//    //            glNormal3fv(mesh_.normal(v).data());
//    //            Psdc.GetPC(color, difference_map_[v] / max_difference_);
//    //            glColor3f(color[0], color[1], color[2]);
//    //            glVertex3fv(mesh_.point(v).data());
//
//    //            he = mesh_.next_halfedge_handle(he);
//    //        } while (he != mesh_.halfedge_handle(f));
//    //    }
//    //}
//    //else if (!is_low_poly_)
//    //{   // Normal Mode
//    //    for (auto f : mesh_.faces())
//    //    {
//    //        auto he = mesh_.halfedge_handle(f);
//    //        do
//    //        {
//    //            auto v = mesh_.to_vertex_handle(he);
//    //            glNormal3fv(mesh_.normal(v).data());
//    //            glColor3f(1.0f, 1.0f, 1.0f);
//    //            glVertex3fv(mesh_.point(v).data());
//
//    //            he = mesh_.next_halfedge_handle(he);
//    //        } while (he != mesh_.halfedge_handle(f));
//    //    }
//    //}
//    //else 
//    //{   // Low Poly Mode
//    //    for (auto f : mesh_.faces())
//    //    {
//    //        auto he = mesh_.halfedge_handle(f);
//    //        // Calculate the average normal of vertices of face.
//    //        Vec3f average_normal(0, 0, 0);
//    //        do
//    //        {
//    //            auto v = mesh_.to_vertex_handle(he);
//    //            auto norm_om = mesh_.normal(v).data();
//    //            Vec3f norm{ norm_om[0], norm_om[1], norm_om[2] };
//    //            average_normal += norm;
//    //            he = mesh_.next_halfedge_handle(he);
//    //        } while (he != mesh_.halfedge_handle(f));
//    //        // Use the average normal as the whole face.
//    //        average_normal /= 3.0f;
//    //        he = mesh_.halfedge_handle(f);
//    //        do
//    //        {
//    //            auto v = mesh_.to_vertex_handle(he);
//    //            glNormal3fv(average_normal.data());
//    //            glColor3f(1.0f, 1.0f, 1.0f);
//    //            glVertex3fv(mesh_.point(v).data());
//
//    //            he = mesh_.next_halfedge_handle(he);
//    //        } while (he != mesh_.halfedge_handle(f));
//    //    }
//    //}
//
//    //glEnd();
//}
//
//// No Usage in this project.
//void RenderingWidget::DrawTexture(bool bv)
//{
//	//if (!bv)
//	//	return;
//	//if (mesh_.n_vertices() == 0 || !is_load_texture_)
//	//	return;
// //   if (!param.has_init())
// //   {
// //       param.setting(_method, _boundary);
// //       param.init();
// //   }
//
//	//glBindTexture(GL_TEXTURE_2D, texture_[0]);
//
// //   glEnable(GL_TEXTURE_2D);
//
//	//glBegin(GL_TRIANGLES);
// //   for (auto f : mesh_.faces())
// //   {
// //       auto he = mesh_.halfedge_handle(f);
// //       do
// //       {
// //           auto v = mesh_.to_vertex_handle(he);
// //           glTexCoord2fv(param.query(v).data());
// //           glNormal3fv(mesh_.normal(v).data());
// //           glVertex3fv(mesh_.point(v).data());
//
// //           he = mesh_.next_halfedge_handle(he);
// //       } while (he != mesh_.halfedge_handle(f));
// //   }
//
//	//glEnd();
//}
