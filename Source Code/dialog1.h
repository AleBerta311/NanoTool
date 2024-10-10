#ifndef DIALOG1_H
#define DIALOG1_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QHBoxLayout>

namespace Ui {
class Dialog1;
}

class Dialog1 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog1(QWidget *parent = nullptr);
    ~Dialog1();

private slots:

    void on_pushButton_start_clicked();

    void on_pushButton_filein_clicked();

    void on_pushButton_fileout_clicked();


    void on_horizontalSlider_minpot_valueChanged(int value);

    void on_horizontalSlider_maxpot_valueChanged(int value);

    void on_dial_temp_valueChanged(int value);


    void on_comboBox_reaction_currentTextChanged(const QString &arg1);

private:
    Ui::Dialog1 *ui;
    QChartView *chartView;
    QChartView *chartView2;
    QHBoxLayout *layout;
    QString filename_in;
    QString folder_out;
};

#endif // DIALOG1_H
