#include "sitesdialog.h"
#include "ui_sitesdialog.h"
#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QSettings>
#include <QStackedBarSeries>
#include <QBarSet>
#include <QColor>
#include <QBarCategoryAxis>
#include "Nanoparticle.h"   // this already contains the other nanocode .h files
#include "SharedFunctions.h"
#include <chrono>


SitesDialog::SitesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SitesDialog)
{

    ui->setupUi(this);
    QWidget::setWindowTitle("Site analyzer");

    // shape choices here
    ui->comboBox_shape->addItem("Icosahedron");
    ui->comboBox_shape->addItem("Decahedron");
    ui->comboBox_shape->addItem("Octahedron");
    ui->comboBox_shape->addItem("Truncated Octahedron");
    ui->comboBox_shape->addItem("Cuboctahedron");
    ui->comboBox_shape->addItem("FCC111");
    ui->comboBox_shape->addItem("FCC100");
    ui->comboBox_shape->addItem("FCC-Cube");
    ui->comboBox_shape->addItem("Tetrahedron");
    ui->comboBox_shape->addItem("BCC");
    ui->comboBox_shape->addItem("HCP");
    ui->comboBox_shape->addItem("SC");
    ui->comboBox_shape->addItem("Generic/Amorphous");

    // Default site option
    ui->radioButton_all->setChecked(true);
}

SitesDialog::~SitesDialog()
{
    delete ui;
}

void SitesDialog::on_pushButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose .xyz file", QDir::homePath());
    ui->lineEdit_filename->setText(filename);
    file_in = filename;
}

void SitesDialog::on_pushButton_analyze_clicked()
{
    RedirectOutput();
    // Record start time
    auto start = std::chrono::high_resolution_clock::now();
    //////////////////////////// Basically all the Nanocode goes here ////////////////////////////
    try
    {
        // Example user input
        int atomic_number = ui->lineEdit_atomicZ->text().toInt();

        // Initialize maps
        map<int, double> Z_to_radius_Map;
        DefineAtomicRadiusMap(Z_to_radius_Map);

        map<string, int> Geom_to_SurfaceCN_thresholds;
        DefineSurfaceCNValues(Geom_to_SurfaceCN_thresholds);

        map<string, int> Geom_to_BulkCN;
        DefineBulkCN(Geom_to_BulkCN);

        // File paths
        string str_coordinates = file_in.toStdString();

        Nanoparticle myNano;

        // Load the cluster geometric data
        myNano.Load_cluster_from_file(str_coordinates);

        // Set bulk radius and perform calculations
        myNano.SetBulkRadius(Z_to_radius_Map[atomic_number]);

        double cutoff_r = CalculateCutoffRadius(atomic_number, Z_to_radius_Map);
        myNano.Find_nearest_neighbors(cutoff_r);
        myNano.Count_CN();

        // Set the shape and identify surface atoms
        string shape = ui->comboBox_shape->currentText().toStdString();
        myNano.Set_shape(shape);
        myNano.Identify_Surface_Atoms(Geom_to_SurfaceCN_thresholds);

        // Perform further calculations
        myNano.CalculateAllStrains();

        int ads_type;
        if (ui->radioButton_atop->isChecked())
            ads_type = 0;
        else if (ui->radioButton_bridge->isChecked())
            ads_type = 1;
        else if (ui->radioButton_3hollow->isChecked())
            ads_type = 2;
        else if (ui->radioButton_4hollow->isChecked())
            ads_type = 3;
        else // if "all" is checked
            ads_type = 4;

        // Prepare for charting
        QStackedBarSeries *series = new QStackedBarSeries();
        QMap<double, QMap<QString, int>> allOccurrences; // {GCN Value -> {Color Label -> Occurrence}}

        // Helper function to process and aggregate GCN calculations
        auto processGCN = [&](std::function<void()> calcFunction, const QString& label, QColor color)
        {
            myNano.Clear_gcn_occurences(); // Reset occurrences
            if (label != "aGCN")
            {
                myNano.Calculate_aGCN(Geom_to_BulkCN);
                myNano.Clear_gcn_soft();
            }


            calcFunction();
            myNano.Count_gcn_occurences();
            auto gcnOccurrences = myNano.Get_gcn_occurences();

            // Collect and aggregate values
            for (const auto& item : gcnOccurrences) {
                double key = item.first;
                int occurrence = item.second;
                if (!allOccurrences.contains(key)) {
                    allOccurrences[key] = QMap<QString, int>();
                }
                allOccurrences[key][label] = occurrence;
            }
            string outsites = "C:/Users/aless/OneDrive/Desktop/QtProject/NanoTool/build/Desktop_Qt_6_7_2_MinGW_64_bit-Debug/debug/" + label.toStdString() + "Sites.xyz";
            string outsites_and_surf = "C:/Users/aless/OneDrive/Desktop/QtProject/NanoTool/build/Desktop_Qt_6_7_2_MinGW_64_bit-Debug/debug/" + label.toStdString() + "SitesAndSurf.xyz";
            myNano.Write_site_coords(outsites);
            myNano.Write_sites_plus_surface(outsites_and_surf);

        };

        if (ads_type == 4)
        {
            // Handle "all" case
            processGCN([&]() { myNano.Calculate_aGCN(Geom_to_BulkCN); }, "aGCN", Qt::blue);
            processGCN([&]() { myNano.Calculate_bGCN(Geom_to_BulkCN); }, "bGCN", Qt::red);
            processGCN([&]() { myNano.Calculate_3hGCN(Geom_to_BulkCN); }, "3hGCN", Qt::green);
            processGCN([&]() { myNano.Calculate_4hGCN(Geom_to_BulkCN); }, "4hGCN", Qt::yellow);
        }
        else
        {
            // Handle individual cases

            switch (ads_type) {
            case 0:
                processGCN([&]() { myNano.Calculate_aGCN(Geom_to_BulkCN); }, "aGCN", Qt::blue);
                break;
            case 1:
                processGCN([&]() { myNano.Calculate_bGCN(Geom_to_BulkCN); }, "bGCN", Qt::red);
                break;
            case 2:
                processGCN([&]() { myNano.Calculate_3hGCN(Geom_to_BulkCN); }, "3hGCN", Qt::green);
                break;
            case 3:
                processGCN([&]() { myNano.Calculate_4hGCN(Geom_to_BulkCN); }, "4hGCN", Qt::yellow);
                break;
            }
        }


        // Sort categories and create bar sets
        QList<double> sortedKeys = allOccurrences.keys();
        std::sort(sortedKeys.begin(), sortedKeys.end());

        // Create a QMap for bar sets
        QMap<QString, QBarSet*> barSetsMap;
        QStringList categories;
        for (const auto& key : sortedKeys) {
            QString categoryLabel = QString("%1").arg(key, 0, 'f', 3);
            categories << categoryLabel;

            // Create bar sets if they do not already exist
            for (auto it = allOccurrences[key].begin(); it != allOccurrences[key].end(); ++it)
            {
                QString label = it.key();
                int occurrence = it.value();

                if (!barSetsMap.contains(label))
                {
                    QBarSet *barSet = new QBarSet(label);
                    barSet->setColor(label == "aGCN" ? Qt::blue :
                                         label == "bGCN" ? Qt::red :
                                         label == "3hGCN" ? Qt::green :
                                         Qt::yellow);
                    series->append(barSet);
                    barSetsMap[label] = barSet;
                }

                QBarSet *barSet = barSetsMap[label];
                int index = categories.indexOf(categoryLabel);
                while (barSet->count() <= index) {
                    barSet->append(0); // Append zeros to match the index
                }
                barSet->replace(index, occurrence);

            }
        }

        // Enable labels on the bars (for all the bar sets)
        series->setLabelsVisible(true);  // Show the value labels on bars
        series->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);  // Position labels inside bars
        series->setLabelsFormat("<span style='color:black; font-weight: bold;'>@value</span>"); // Bold labels

        // Create a chart
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(ui->buttonGroup_site_type->checkedButton()->text() + " GCN occurrences for " +
                        QString::number(myNano.Get_atoms().size()) + QString::fromStdString(shape) + " (Z = " +QString::number(atomic_number) + " )");
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumSize(1280, 640);
        chart->setAnimationOptions(QChart::SeriesAnimations);

        // Create a category axis for the x-axis
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->setAxisX(axisX, series);
        axisX->setTitleText("GCN values");

        // Set font for x-axis labels
        QFont xAxisFont = axisX->labelsFont();
        xAxisFont.setPointSize(14); // Increase font size
        xAxisFont.setBold(true);     // Make font bold
        axisX->setLabelsFont(xAxisFont);

        axisX->setLabelsAngle(-90); // Rotate labels by angle

        // Create a value axis for the y-axis
        QValueAxis *axisY = new QValueAxis();
        chart->setAxisY(axisY, series);
        axisY->setTitleText("Occurrence");

        // Set font for y-axis labels (optional)
        QFont yAxisFont = axisY->labelsFont();
        yAxisFont.setPointSize(14); // Increase font size
        yAxisFont.setBold(true);     // Make font bold
        axisY->setLabelsFont(yAxisFont);

        QFont legendFont = chart->legend()->font();
        legendFont.setPointSize(legendFont.pointSize() + 6);  // Increase legend font size
        chart->legend()->setFont(legendFont);

        QHBoxLayout* layout = new QHBoxLayout();
        layout->addWidget(chartView);
        setLayout(layout);
        chartView->setChart(chart);

        map<int, double> Z_to_mass_map;
        DefineMassMap(Z_to_mass_map);

        myNano.Calculate_surface_area_void(Geom_to_BulkCN);
        myNano.Calculate_total_mass(Z_to_mass_map, atomic_number);

        QString out_dir = ui->lineEdit_filename_out->text();
        QString out_file1 = out_dir + "/Surface.txt";
        QString out_file2 = out_dir + "/AllCN.txt";
        QString out_file3 = out_dir + "/DiamSurf.txt";
        string out1 = out_file1.toStdString();
        string out2 = out_file2.toStdString();
        string out3 = out_file3.toStdString();

        myNano.Write_surface_atoms(out1);
        myNano.Write_atom_w_cn(out2);
        myNano.Write_diam_and_surface(out3);
    }
    catch(const std::exception& e)
    {
        QMessageBox::critical(this, "Error: ", e.what());
        qDebug() << "Exception caught";
    }

    // Record start time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration (in microseconds)
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Need for Speed
    cout << "Analysis completed in: " << duration << " microseconds" << endl;
}

void SitesDialog::on_pushButton_fileout_clicked()
{
    QSettings settings("NanoGroup", "NanoTool");
    QString lastFolder = settings.value("lastSelectedFolder", QDir::homePath()).toString();

    QString f_out = QFileDialog::getExistingDirectory(this, tr("Select Folder"), lastFolder);
    folder_out = f_out;
    ui->lineEdit_filename_out->setText(folder_out);

    if (!f_out.isEmpty())
    {
        // Save the selected folder using QSettings
        QSettings settings("NanoGroup", "NanoTool");
        settings.setValue("lastSelectedFolder", f_out);
    }
}

