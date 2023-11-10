#include "ViewOpenGL.h"

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
    FragColor = rgb;
}
)";

///////////////////////

ViewOpenGL::ViewOpenGL()
    : QOpenGLWindow()
    , OpenGLFunctions()
{
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    mTimer.start(2);
}

ViewOpenGL::~ViewOpenGL()
{

}

void ViewOpenGL::setImg(const cv::Mat &mat)
{
    QMutexLocker _(&mMut);
    mImg = mat;
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
        0, 0,
        0, 1,
        1, 0,
        1, 1,
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

    cv::Mat test = cv::Mat::zeros(480, 640, CV_8UC3);
    cv::rectangle(test, {0, 0, 160, 480}, {255, 0, 0}, -1);
    cv::rectangle(test, {160, 0, 160, 480}, {0, 255, 0}, -1);
    cv::rectangle(test, {320, 0, 160, 480}, {0, 0, 255}, -1);
    cv::rectangle(test, {480, 0, 160, 480}, {255, 255, 255}, -1);
    setImg(test);

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
        mTexUpdate = false;
        genTexture();
    }
    if(mUpdate){
        mUpdate = false;
        update();
    }

}

void ViewOpenGL::genTexture()
{
    if(mImg.empty() || !mInit){
        return;
    }
    QMutexLocker _(&mMut);

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
    qDebug() << glGetError();

    doneCurrent();

    mUpdate = true;
}
