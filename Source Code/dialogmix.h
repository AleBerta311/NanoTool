#ifndef DIALOGMIX_H
#define DIALOGMIX_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QScatterSeries>
#include <QSplineSeries>
#include <QLineSeries>
#include <QHBoxLayout>
#include "qparticle.h"

namespace Ui {
class DialogMix;
}

class DialogMix : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMix(QWidget *parent = nullptr);
    ~DialogMix();

private slots:
    void on_horizontalSlider_maxpot_valueChanged(int value);

    void on_horizontalSlider_minpot_valueChanged(int value);

    void on_dial_temp_valueChanged(int value);

    void on_pushButton_start_clicked();

    void on_pushButton_add_sample_clicked();

    void on_pushButton_fileout_clicked();

private:
    Ui::DialogMix *ui;
    QChartView *chartView;
    QChartView *chartView2;
    QHBoxLayout *layout;
    QVector<QParticle> particles;
    QString folder_out;
};

#endif // DIALOGMIX_H
