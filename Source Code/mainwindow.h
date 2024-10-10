#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QEvent>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_AnalyzeShape_clicked();

    void on_pushButton_compare_shapes_clicked();

    void on_pushButton_clicked();

    void on_pushButton_AnalyzeSites_clicked();

    void animateButtons(QPushButton*, bool);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void on_pushButton_sizeactivity_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_activityanalysis_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
