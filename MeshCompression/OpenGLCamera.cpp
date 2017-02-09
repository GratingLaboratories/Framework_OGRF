#include "stdafx.h"
#include "OpenGLCamera.h"

#define PI 3.14159f

OpenGLCamera::~OpenGLCamera()
{
}

void OpenGLCamera::update()
{
    direction_ = (position_ - target_).normalized();
    QVector3D up{ 0.0f, 0.0f, 1.0f };
    right_ = QVector3D::crossProduct(up, direction_);
    up_ = QVector3D::crossProduct(direction_, right_);
}

void OpenGLCamera::move_right(float dis)
{
    position_ += dis * right_;
    update();
}

void OpenGLCamera::move_up(float dis)
{
    position_ += dis * up_;
    update();
}

void OpenGLCamera::move_back(float dis)
{
    float radius = (position_ - target_).length();
    if (radius < -dis)
        return;
    position_ += dis * direction_;
    update();
}

void OpenGLCamera::move_right_target(float dis)
{
    target_ += dis * right_;
    update();
}

void OpenGLCamera::move_up_target(float dis)
{
    target_ += dis * up_;
    update();
}

void OpenGLCamera::move_back_target(float dis)
{
    target_ += dis * direction_;
    update();
}

void OpenGLCamera::move_around_right(float angle)
{
    float radius = (position_ - target_).length();
    float dis = 2 * radius * sinf(angle * PI / 180.0f / 2.0f);           // d = 2 r sin(alpha * PI / 2)
    position_ += dis * cosf(angle * PI / 180.0f / 2.0f) * right_;        // x += d cos(alpha * PI / 2)
    position_ -= dis * sinf(angle * PI / 180.0f / 2.0f) * direction_;    // z -= d sin(alpha * PI / 2)
    update();
}

void OpenGLCamera::move_around_up(float angle)
{
    float radius = (position_ - target_).length();
    float dis = 2 * radius * sinf(angle * PI / 180.0f / 2.0f);           // d = 2 r sin(alpha * PI / 2)
    position_ += dis * cosf(angle * PI / 180.0f / 2.0f) * up_;           // y += d cos(alpha * PI / 2)
    position_ -= dis * sinf(angle * PI / 180.0f / 2.0f) * direction_;    // z -= d sin(alpha * PI / 2)
    update();
}

void OpenGLCamera::move_around_right_target(float angle)
{
    float radius = (position_ - target_).length();
    float dis = 2 * radius * sinf(angle * PI / 180.0f / 2.0f);           // d = 2 r sin(alpha * PI / 2)
    target_ += dis * cosf(angle * PI / 180.0f / 2.0f) * right_;          // x += d cos(alpha * PI / 2)
    target_ += dis * sinf(angle * PI / 180.0f / 2.0f) * direction_;      // z += d sin(alpha * PI / 2)
    update();
}

void OpenGLCamera::move_around_up_target(float angle)
{
    float radius = (position_ - target_).length();
    float dis = 2 * radius * sinf(angle * PI / 180.0f / 2.0f);           // d = 2 r sin(alpha * PI / 2)
    target_ += dis * cosf(angle * PI / 180.0f / 2.0f) * up_;             // y += d cos(alpha * PI / 2)
    target_ += dis * sinf(angle * PI / 180.0f / 2.0f) * direction_;      // z += d sin(alpha * PI / 2)
    update();
}

QMatrix4x4 OpenGLCamera::view_mat() const
{
    QMatrix4x4 view;
    view.lookAt(position_, target_, up_);
    return view;
}
