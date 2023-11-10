#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDir dir;
    mDefaultDir = dir.homePath();

    auto w = QWidget::createWindowContainer(&mView);
    ui->vlCntr->addWidget(w);

    initDistortions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    auto str = QFileDialog::getOpenFileName(nullptr, "Open Image...", mDefaultDir,
                                            "Images (*.bmp *.png *.jpg *.jpeg *.tif *.tiff *.pgm *.ppm)");
    if(!str.isEmpty()){
        mView.setImg(str);
    }
}

using tsetdist = std::function<void(int, float)>;

const float maxSld = 1000;

struct ElControls{
    QHBoxLayout *hb = nullptr;
    QDoubleSpinBox *db = nullptr;
    QSlider *sl = nullptr;

    void setMinMaxVal(float _min, float _max, float val) const{
        if(db){
            db->setMinimum(_min);
            db->setMaximum(_max);
            db->setValue(val);
        }
//        if(sl){
//            sl->setMinimum(_min);
//            sl->setMaximum(_max);
//            sl->setValue(val);
//        }
    }
};

ElControls createControl(tsetdist fun, const QString& name, float val, int idx, float _min, float _max,
                           float singleStep, int prec = 2)
{
    auto hb = new QHBoxLayout();
    auto lb = new QLabel(name);
    auto sl = new QSlider(Qt::Horizontal);
    auto db = new QDoubleSpinBox();
    hb->addWidget(lb);
    hb->addWidget(sl);
    hb->addWidget(db);

    float sliderKoeff = 2 * maxSld / (_max - _min);
    //        sl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sl->setMinimum(-maxSld);
    sl->setMaximum(maxSld);
    db->setMinimum(_min);
    db->setMaximum(_max);
    db->setSingleStep(singleStep);
    db->setDecimals(prec);

    QObject::connect(sl, &QSlider::valueChanged, [ db, idx, sliderKoeff, fun](int val){
        QSignalBlocker _(db);
        float dval = 1./sliderKoeff * val;
        db->setValue(dval);
        fun(idx, dval);
    });
    QObject::connect(db, qOverload<double>(&QDoubleSpinBox::valueChanged), [ sl, idx, sliderKoeff, fun](double dval){
        QSignalBlocker _(sl);
        int ival = sliderKoeff * dval;
        sl->setValue(ival);
        fun(idx, dval);
    });
    {
        QSignalBlocker _(db);
        db->setValue(val);
    }
    {
        QSignalBlocker _(sl);
        sl->setValue(sliderKoeff * val);
    }

    return {hb, db, sl};
}

void MainWindow::initDistortions()
{
//    auto funcam = [this](int idx, float val){
//        mView.setCamParam(idx, val);
//    };
    auto fundist = [this](int idx, float val){
        mView.setDistortion(idx, val);
    };

//    auto fx = createControl(funcam, "Fx", 0, 0, 0, 60000, 1, 2);
//    auto fy = createControl(funcam, "Fy", 0, 1, 0, 60000, 1, 2);
//    auto cx = createControl(funcam, "Cx", 0, 2, 0, 60000, 1, 2);
//    auto cy = createControl(funcam, "Cy", 0, 3, 0, 60000, 1, 2);

//    QObject::connect(&mView, &ViewOpenGL::updateImage, [fx, fy, cx, cy, this](){
//        fx.setMinMaxVal(0, mView.wI(), mView.camParam(0));
//        fy.setMinMaxVal(0, mView.hI(), mView.camParam(1));
//        cx.setMinMaxVal(0, mView.wI(), mView.camParam(2));
//        cy.setMinMaxVal(0, mView.hI(), mView.camParam(3));
//    });

//    ui->vlDist->addLayout(fx.hb);
//    ui->vlDist->addLayout(fy.hb);
//    ui->vlDist->addLayout(cx.hb);
//    ui->vlDist->addLayout(cy.hb);

    QStringList names = {
        "k1",
        "k2",
        "p1",
        "p2",
        "k3",
        "k4",
        "k5",
        "k6",
        "s1",
        "s2",
        "s3",
        "s4",
        "t1",
        "t2",
    };
    int idx = 0;
    for(auto n: names){
        auto hb = createControl(fundist, n, 0, idx++, -1, 1, 0.0001, 5);
        ui->vlDist->addLayout(hb.hb);
    }
}

