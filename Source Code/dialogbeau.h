#ifndef DIALOGBEAU_H
#define DIALOGBEAU_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QHBoxLayout>
#include "qparticle.h"


namespace Ui {
class DialogBeau;
}

class DialogBeau : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBeau(QWidget *parent = nullptr);
    ~DialogBeau();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_start_clicked();

    void on_comboBox_reaction_currentTextChanged(const QString &arg1);

    void on_dial_temp_valueChanged(int value);

    void on_horizontalSlider_minpot_valueChanged(int value);

    void on_horizontalSlider_maxpot_valueChanged(int value);

    void on_radioButtonActVsSize_toggled(bool checked);

    void on_checkBox_corrections_toggled(bool checked);

    void on_pushButton_fileout_clicked();

private:
    Ui::DialogBeau *ui;
    QChartView *chartView;
    QChartView *chartView2;
    QHBoxLayout *layout;
    QVector<QParticle> particles;
    QString folder_out;
};

#endif // DIALOGBEAU_H
