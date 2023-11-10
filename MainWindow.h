#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ViewOpenGL.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;
    QString mDefaultDir;
    QString mLastFile;

    ViewOpenGL mView;

    void initDistortions();

    void loadSettings();
    void writeSettings();
};

#endif // MAINWINDOW_H
