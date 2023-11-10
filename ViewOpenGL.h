#ifndef VIEWOPENGL_H
#define VIEWOPENGL_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

#include <opencv2/opencv.hpp>

using OpenGLFunctions = QOpenGLFunctions_3_3_Core;

class ViewOpenGL : public QOpenGLWindow, OpenGLFunctions
{
    Q_OBJECT
public:
    ViewOpenGL();
    ~ViewOpenGL();

    void setImg(const cv::Mat& mat);
    void setImg(const QString& file);

protected:
    void initializeGL();
    void paintGL();

private slots:
    void onTimeout();

private:
    QOpenGLShaderProgram mProg;
    QTimer mTimer;
    QMutex mMut;

    std::vector<float> mVecRect;
    std::vector<float> mTexRect;

    bool mInit = false;
    bool mUpdate = false;
    bool mTexUpdate = false;
    cv::Mat mImg;

    int mUMvp = 0;
    int mUTex = 0;
    int mAVec = 0;
    int mATex = 0;

    uint mBTex = 0;

    void genTexture();

};

#endif // VIEWOPENGL_H
