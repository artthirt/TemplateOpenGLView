#include "ViewOpenGL.h"

#include <QtConcurrent/QtConcurrent>

QString c_Vertex =
    R"(
#version 330
uniform mat4 uMvp;
in vec4 aPos;
in vec2 aTex;
out vec2 vTex;
void main()
{
    vTex = aTex;
    gl_Position = uMvp * aPos;
}
)";

QString c_Fragment =
    R"(
#version 330
in vec2 vTex;
uniform sampler2D uTex;
out vec4 FragColor;
void main()
{
    vec4 rgb = texture2D(uTex, vTex);
    FragColor = rgb.bgra;
}
)";

///////////////////////

class InternalThread: public QThread{
public:
    InternalThread(ViewOpenGL* that){
        mOwner = that;
        moveToThread(this);
        start();
    }
    ~InternalThread(){
        mDone = true;
        quit();
        wait();
    }
    void needCompute(){
        mCompute = true;
    }
protected:
    virtual void run(){
        while(!mDone){
            if(mCompute && mOwner){
                mCompute = false;
                mOwner->undistort();
            }else{
                usleep(2);
            }
        }
    }
private:
    bool mCompute = false;
    bool mDone = false;
    ViewOpenGL *mOwner = nullptr;
};

///////////////////////

ViewOpenGL::ViewOpenGL()
    : QOpenGLWindow()
    , OpenGLFunctions()
{
    mDistortions.resize(14, 0);

    cv::Mat test = cv::Mat::zeros(480, 640, CV_8UC3);
    cv::rectangle(test, {0, 0, 160, 480}, {255, 0, 0}, -1);
    cv::rectangle(test, {160, 0, 160, 480}, {0, 255, 0}, -1);
    cv::rectangle(test, {320, 0, 160, 480}, {0, 0, 255}, -1);
    cv::rectangle(test, {480, 0, 160, 480}, {255, 255, 255}, -1);
    setImg(test);

    mIThr.reset(new InternalThread(this));

    connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    mTimer.start(2);
}

ViewOpenGL::~ViewOpenGL()
{
    if(mIThr){
        mIThr.reset();
    }
}

void ViewOpenGL::setImg(const cv::Mat &mat)
{
    QMutexLocker _(&mMut);

    cv::Size sz = mImg.size();

    mImg = mat;
    mOrg = mat.clone();

    if(mImg.size() != sz){
        mFx = mImg.cols/2;
        mFy = mImg.rows/2;
        mCx = mFx;
        mCy = mFy;
    }

    emit updateImage();

    mTexUpdate = true;
}

void ViewOpenGL::setImg(const QString &file)
{
    auto tmp = cv::imread(file.toStdString());
    if(tmp.empty()){
        return;
    }
    setImg(tmp);
}

void ViewOpenGL::setDistortions(const std::vector<float> &d)
{
    mDistortions = d;
    mIThr->needCompute();
}

std::vector<float> ViewOpenGL::distortions() const
{
    return mDistortions;
}

void ViewOpenGL::setDistortion(int idx, float val)
{
    if(idx >= mDistortions.size()){
        mDistortions.resize(idx + 1, 0);
    }
    mDistortions[idx] = val;
    setDistortions(mDistortions);
}

float ViewOpenGL::distortion(int idx) const
{
    if(idx < mDistortions.size()){
        return mDistortions[idx];
    }
    return 0;
}

void ViewOpenGL::setCamParam(int idx, float val)
{
    switch (idx) {
    case 0:
        mFx = val; break;
    case 1:
        mFy = val; break;
    case 2:
        mCx = val; break;
    case 3:
        mCy = val; break;
    default:
        break;
    }
    mIThr->needCompute();
}

float ViewOpenGL::camParam(int idx) const
{
    switch (idx) {
    case 0:
        return mFx;
    case 1:
        return mFy;
    case 2:
        return mCx;
    case 3:
        return mCy;
    default:
        break;
    }
    return 0;
}

void ViewOpenGL::initializeGL()
{
    QOpenGLWindow::initializeGL();
    OpenGLFunctions::initializeOpenGLFunctions();

    mVecRect = {
        -1, -1, 0,
        -1,  1, 0,
         1, -1, 0,
         1,  1, 0,
    };
    mTexRect = {
        0, 1,
        0, 0,
        1, 1,
        1, 0,
    };

    mProg.addShaderFromSourceCode(QOpenGLShader::Vertex, c_Vertex);
    mProg.addShaderFromSourceCode(QOpenGLShader::Fragment, c_Fragment);
    if(!mProg.link()){
        qDebug() << mProg.log();
    }

    mProg.bind();

    mUMvp = mProg.uniformLocation("uMvp");
    mAVec = mProg.attributeLocation("aPos");
    mATex = mProg.attributeLocation("aTex");
    mUTex = mProg.uniformLocation("uTex");

    //cv::imwrite("test.png", test);

    mInit = true;
}

void ViewOpenGL::paintGL()
{
    mProg.bind();
    float ar = 1.f * width() / height();
    QMatrix4x4 mvp, proj, model;
    proj.frustum(-ar/2, ar/2, -1./2, 1./2, 1., 10);
    model.setToIdentity();
    model.translate(0, 0, -2);

    if(!mImg.empty()){
        float arI = 1.f * mImg.cols / mImg.rows;
        if(ar > arI){
            model.scale(arI, 1, 1);
        }else{
            model.scale(ar, ar/arI, 1);
        }
    }

    mvp = proj * model;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1, 0.1, 0.1, 1);

    glUniformMatrix4fv(mUMvp, 1, 0, mvp.constData());
    glUniform1i(mUTex, 0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mBTex);
    glEnableVertexAttribArray(mAVec);
    glVertexAttribPointer(mAVec, 3, GL_FLOAT, false, 0, mVecRect.data());
    glEnableVertexAttribArray(mATex);
    glVertexAttribPointer(mATex, 2, GL_FLOAT, false, 0, mTexRect.data());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, mVecRect.size()/3);
    glDisableVertexAttribArray(mATex);
    glDisableVertexAttribArray(mAVec);
    glDisable(GL_TEXTURE_2D);


    mProg.release();
}

void ViewOpenGL::onTimeout()
{
    if(!mInit){
        return;
    }
    if(mTexUpdate){
        genTexture();
    }
    if(mUpdate){
        mUpdate = false;
        update();
    }

}

void ViewOpenGL::genTexture()
{
    if(!mTexUpdate || mImg.empty() || !mInit){
        return;
    }
    if(!mMut.tryLock()){
        return;
    }
    mTexUpdate = false;

    _genTexture();

    mMut.unlock();
    mUpdate = true;
}

void ViewOpenGL::_genTexture()
{
    if(mBTex == 0){
        glGenTextures(1, &mBTex);
    }

    int fmt = GL_RGB;
    int type = GL_UNSIGNED_BYTE;
    if(mImg.channels() == 1){
        fmt = GL_RED;
    }
    if(mImg.channels() == 4){
        fmt = GL_RGBA;
    }
    if(mImg.depth() == CV_16U){
        type = GL_UNSIGNED_SHORT;
    }

    makeCurrent();

    glBindTexture(GL_TEXTURE_2D, mBTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, mImg.cols, mImg.rows, 0, fmt, type, mImg.data);
    doneCurrent();
}

void ViewOpenGL::undistort()
{
    QMutexLocker _(&mMut);
    float camm[] = {
        mFx, 0, mCx,
        0, mFy, mCy,
        0, 0, 1
    };
    auto cameraMatrix = cv::Mat(3, 3, CV_32F, camm);

    cv::undistort(mOrg, mImg, cameraMatrix, mDistortions);

    mTexUpdate = true;
}
