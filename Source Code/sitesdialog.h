#ifndef SITESDIALOG_H
#define SITESDIALOG_H

#include <QDialog>
#include <QString>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QBarSeries>
#include <QVBoxLayout>

namespace Ui {
class SitesDialog;
}

class SitesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SitesDialog(QWidget *parent = nullptr);
    ~SitesDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_analyze_clicked();

    void on_pushButton_fileout_clicked();

private:
    Ui::SitesDialog *ui;
    QString file_in;
    QChartView *chartView;
    QVBoxLayout *layout;
    QString folder_out;
};

#endif // SITESDIALOG_H
