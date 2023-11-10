#ifndef VIEWOPENGL_H
#define VIEWOPENGL_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QFuture>

#include <opencv2/opencv.hpp>

using OpenGLFunctions = QOpenGLFunctions_3_3_Core;

class InternalThread;

class ViewOpenGL : public QOpenGLWindow, OpenGLFunctions
{
    Q_OBJECT
public:
    ViewOpenGL();
    ~ViewOpenGL();

    void setImg(const cv::Mat& mat);
    void setImg(const QString& file);

    void setDistortions(const std::vector<float> &d);
    std::vector<float> distortions() const;

    void setDistortion(int idx, float val);
    float distortion(int idx) const;

    void setCamParam(int idx, float val);
    float camParam(int idx) const;

    float Fx() const { return mFx; }
    float Fy() const { return mFy; }
    float Cx() const { return mCx; }
    float Cy() const { return mCy; }

    int wI() const { return mImg.cols; }
    int hI() const { return mImg.rows; }

signals:
    void updateImage();

protected:
    void initializeGL();
    void paintGL();

private slots:
    void onTimeout();

private:
    QOpenGLShaderProgram mProg;
    QTimer mTimer;
    QMutex mMut;
    QScopedPointer<InternalThread> mIThr;


    std::vector<float> mVecRect;
    std::vector<float> mTexRect;

    bool mInit = false;
    bool mUpdate = false;
    bool mTexUpdate = false;
    cv::Mat mImg;
    cv::Mat mOrg;

    int mUMvp = 0;
    int mUTex = 0;
    int mAVec = 0;
    int mATex = 0;

    uint mBTex = 0;

    float mFx = 1;
    float mFy = 1;
    float mCx = 0;
    float mCy = 0;

    std::vector<float> mDistortions;

    void genTexture();
    void _genTexture();

    void undistort();

    friend class InternalThread;
};

#endif // VIEWOPENGL_H
