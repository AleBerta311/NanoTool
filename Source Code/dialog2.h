#ifndef DIALOG2_H
#define DIALOG2_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QHBoxLayout>
#include "qparticle.h"

namespace Ui {
class Dialog2;
}

class Dialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog2(QWidget *parent = nullptr);
    ~Dialog2();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_start_clicked();

    void on_horizontalSlider_minpot_valueChanged(int value);

    void on_horizontalSlider_maxpot_valueChanged(int value);

    void on_dial_temp_valueChanged(int value);

    void on_comboBox_reaction_currentTextChanged(const QString &arg1);


private:
    Ui::Dialog2 *ui;
    QChartView *chartView;
    QChartView *chartView2;
    QHBoxLayout *layout;
    QVector<QParticle> particles;
};

#endif // DIALOG2_H
