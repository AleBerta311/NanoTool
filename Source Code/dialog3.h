#ifndef DIALOG3_H
#define DIALOG3_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QScatterSeries>
#include <QSplineSeries>
#include <QLineSeries>
#include <QHBoxLayout>
#include "qparticle.h"

namespace Ui {
class Dialog3;
}

class Dialog3 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog3(QWidget *parent = nullptr);
    ~Dialog3();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_start_clicked();

    void on_horizontalSlider_minpot_valueChanged(int value);

    void on_dial_temp_valueChanged(int value);

    void on_comboBox_reaction_currentTextChanged(const QString &arg1);

private:
    Ui::Dialog3 *ui;
    QChartView *chartView;
    QHBoxLayout *layout;
    QVector<QParticle> particles;
};

#endif // DIALOG3_H
