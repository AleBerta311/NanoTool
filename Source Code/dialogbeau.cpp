#include "dialogbeau.h"
#include "ui_dialogbeau.h"
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <QDir>
#include <fstream>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QSettings>
#include <QScatterSeries>

#include "Nanoparticle.h"       // this includes the other nanocode things
#include "SharedFunctions.h"

DialogBeau::DialogBeau(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogBeau)
{
    ui->setupUi(this);

    QWidget::setWindowTitle("BEAUCOOP");
    // reaction choices here
    ui->comboBox_reaction->addItem("HER");
    ui->comboBox_reaction->addItem("CO2RR");
    ui->comboBox_reaction->addItem("ARR IPA");
    ui->comboBox_reaction->addItem("ARR Propane");


    // sliders setup
    ui->horizontalSlider_minpot->setRange(-300, 300);
    ui->horizontalSlider_maxpot->setRange(-300, 300);
    ui->horizontalSlider_minpot->setValue(50);
    ui->horizontalSlider_maxpot->setValue(100);

    // dial setup

    ui->dial_temp->setRange(100, 1500);
    ui->dial_temp->setValue(300);

    //checkboxes

    ui->checkBox_red_curr->setChecked(true);
    ui->radioButton_ActvsPot->setChecked(true);

    ui->lineEdit_ZPE->setHidden(true);
    ui->lineEdit_TDS->setHidden(true);
    ui->lineEdit_coeff->setHidden(true);
    ui->label_coeff->setHidden(true);
    ui->label_TDS->setHidden(true);
    ui->label_ZPE->setHidden(true);

}

DialogBeau::~DialogBeau()
{
    delete ui;
}

void DialogBeau::on_pushButton_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Choose .xyz files", QDir::homePath(), "XYZ Files (*.xyz)");

    if (!fileNames.isEmpty())
    {
        for (auto &fileName : fileNames)
        {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&file);
                QString firstLine = in.readLine();      // number of atoms
                QString element = in.readLine();        // here is the atomic element -> number
                QString shape_line = in.readLine();     // here is the shape
                file.close();

                // Display the lines as needed
                QMessageBox::information(nullptr, "File Content", "Path: " + fileName + "\nFirst Line: " + firstLine +
                                                                      "\nSecond Line: " + element + "\nThird Line: " + shape_line);

                // Add the particle to the QVector
                QParticle new_particle(element.toInt(), fileName, shape_line, firstLine.toInt());
                particles.push_back(new_particle);

                // Update the particle counter
                ui->label_particle_counter->setText(QString::number(particles.size()));
            }
            else
            {
                QMessageBox::warning(nullptr, "Error", "Could not open the file: " + fileName);
            }
        }
    }

}


void DialogBeau::on_pushButton_start_clicked()
{
    if (ui->radioButtonActVsSize->isChecked())
    {
        QChart *chart = new QChart();
        chart->setTitle("Mass Activity");
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumSize(800, 600);

        chart->removeAllSeries();

        QHBoxLayout* layout = new QHBoxLayout();
        layout->addWidget(chartView);

        QValueAxis *axisX = new QValueAxis();
        axisX->setTitleText("Size (nm)");
        axisX->setLabelFormat("%0.3f");
        axisX->setMin(-0.5);
        chart->addAxis(axisX, Qt::AlignBottom);

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Mass Activity (arb. u.)");
        axisY->setLabelFormat("%.1e");
        axisY->setMin(0);
        chart->addAxis(axisY, Qt::AlignLeft);

        bool is_reduction = ui->checkBox_red_curr->isChecked();

        // Map to store scatter series for each shape
        QMap<QString, QScatterSeries*> seriesMap;

        // Predefined list of colors for different shapes
        QList<QColor> colors = {Qt::red, Qt::blue, Qt::green, Qt::yellow, Qt::magenta, Qt::cyan, Qt::gray,
                                Qt::darkBlue, Qt::darkRed, Qt::darkMagenta};

        int colorIndex = 0; // Index to cycle through colors

        qreal maxY = std::numeric_limits<qreal>::min();
        qreal minY = std::numeric_limits<qreal>::max();
        qreal maxX = std::numeric_limits<qreal>::min();
        qreal minX = std::numeric_limits<qreal>::max();

        for (auto& part : particles)
        {
            try {
                int atomic_number = part.Get_Z();
                // Maps initialization
                map<int, double> Z_to_radius_Map;
                DefineAtomicRadiusMap(Z_to_radius_Map);

                map<string, int> Geom_to_SurfaceCN_thresholds;
                DefineSurfaceCNValues(Geom_to_SurfaceCN_thresholds);

                map<string, int> Geom_to_BulkCN;
                DefineBulkCN(Geom_to_BulkCN);

                map<int, double> Z_to_mass_map;
                DefineMassMap(Z_to_mass_map);

                // Load particle details
                Nanoparticle myNano;
                myNano.Load_cluster_from_file_comp(part.Get_path().toStdString());

                myNano.SetBulkRadius(Z_to_radius_Map[atomic_number]);

                double cutoff_r = CalculateCutoffRadius(atomic_number, Z_to_radius_Map);
                myNano.Find_nearest_neighbors(cutoff_r);
                myNano.Count_CN();

                string shape = part.Get_shape().toStdString();
                myNano.Set_shape(shape);
                myNano.Identify_Surface_Atoms(Geom_to_SurfaceCN_thresholds);

                myNano.CalculateAllStrains();

                string reac = ui->comboBox_reaction->currentText().toStdString();
                double coeff = ui->checkBox_corrections->isChecked() ? ui->lineEdit_coeff->text().toDouble() : 1.0;

                // Handle different reactions
                if (reac == "HER")
                {
                    auto DG = (*make_shared<HER_DG_Pt>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());
                    myNano.Setup_Reaction("HER", 0, coeff, DG);
                }
                else if (reac == "CO2RR")
                {
                    auto DG = (*make_shared<CO2RR_DG_Cu>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());
                    myNano.Setup_Reaction("CO2RR", 0, coeff, DG);
                }
                else if(reac == "ARR IPA")
                {
                    auto DG = (*make_shared<ARR_IPA_DG_Pt>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());
                    myNano.Setup_Reaction("ARR_IPA", 0, coeff, DG);
                }
                else if(reac == "ARR Propane")
                {
                    auto DG = (*make_shared<ARR_Prop_DG_Pt>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());
                    myNano.Setup_Reaction("ARR_Propane", 0, coeff, DG);
                }
                // Add other reactions here...

                // Calculate GCN based on reaction type
                switch (myNano.Get_reaction().GetAdsorptionType())
                {
                case 0:
                    myNano.Calculate_aGCN(Geom_to_BulkCN);
                    break;
                case 1:
                    myNano.Calculate_bGCN(Geom_to_BulkCN);
                    break;
                case 2:
                    myNano.Calculate_3hGCN(Geom_to_BulkCN);
                    break;
                case 3:
                    myNano.Calculate_4hGCN(Geom_to_BulkCN);
                    break;
                default:
                    cerr << "\n\tInvalid adsorption type!" << endl;
                    QMessageBox::critical(this, "Error", "Invalid adsorption type!");
                    return;
                }

                // Perform further calculations on nanoparticle
                myNano.Count_gcn_occurences();
                myNano.Print_gcn_occurences();
                myNano.Find_Diameter();

                double Potential = ui->label_minpot->text().toDouble();
                double Temp = ui->label_temp_display->text().toDouble();

                myNano.Calculate_surface_area_void(Geom_to_BulkCN);
                myNano.Calculate_total_mass(Z_to_mass_map, atomic_number);

                double my_mass_act = myNano.Calculate_Mass_Act_at_Pot(Temp, Potential, is_reduction);

                // Adjust axis ranges dynamically
                qreal x = myNano.Get_Diameter() / 10.;
                qreal y = my_mass_act / 1000.;

                if (x > maxX) maxX = x;
                if (x < minX) minX = x;
                if (y > maxY) maxY = y;
                if (y < minY) minY = y;

                // Create series for each shape if it doesn't exist
                QString shapeQString = QString::fromStdString(shape);
                if (!seriesMap.contains(shapeQString)) {
                    QScatterSeries *newSeries = new QScatterSeries();
                    newSeries->setName(shapeQString);
                    newSeries->setColor(colors[colorIndex % colors.size()]); // Cycle through colors
                    newSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
                    newSeries->setMarkerSize(10.0); // Customize size if needed

                    seriesMap[shapeQString] = newSeries;
                    chart->addSeries(newSeries);
                    newSeries->attachAxis(axisX);
                    newSeries->attachAxis(axisY);
                    colorIndex++;
                }

                // Add the point to the corresponding series
                QScatterSeries *series = seriesMap[shapeQString];
                series->append(x, y);

                // Write output files
                ofstream out;
                QString out_dir = ui->lineEdit_filename_out->text();
                QString out_file1 = out_dir + "/MA" + QString::number(part.Get_num()) + ".txt";
                string out1 = out_file1.toStdString();
                out.open(out1);
                out << "Diameter: " << x << endl << "MA: " << y << endl;
                out.close();

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
                qDebug() << "Exception caught:" << e.what();
            } catch (...) {
                QMessageBox::critical(this, "Error", "An unknown error occurred.");
                qDebug() << "Unknown exception caught";
            }
        }

        // Set dynamic axis ranges
        axisX->setRange(minX, maxX);
        axisY->setRange(minY, maxY);

        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);

        setLayout(layout);
        chartView->setChart(chart);

        QMessageBox::information(this, "Complete", "Calculations complete!");
    }
    else        // if Activity vs Potential is checked
    {
        // Initialize chart 1 (Specific Activity)
        QChart *chart = new QChart();
        chart->setTitle("Specific Activity");  // Ensure title is set
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumSize(600, 400);

        // Initialize chart 2 (Mass Activity)
        QChart *chart2 = new QChart();
        chart2->setTitle("Mass Activity");  // Ensure title is set
        chartView2 = new QChartView(chart2);
        chartView2->setRenderHint(QPainter::Antialiasing);
        chartView2->setMinimumSize(600, 400);

        // Create a layout to hold the chart view
        QHBoxLayout* layout = new QHBoxLayout();
        layout->addWidget(chartView);
        layout->addWidget(chartView2);

        // Initialize axes for chart 1
        QValueAxis *axisX = new QValueAxis();
        axisX->setTitleText("Potential (V)");
        axisX->setLabelFormat("%0.2f");

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Current Density (arb. u.)");
        axisY->setLabelFormat("%.1e");

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        // Initialize axes for chart 2
        QValueAxis *axisX2 = new QValueAxis();
        axisX2->setTitleText("Potential (V)");
        axisX2->setLabelFormat("%0.2f");

        QValueAxis *axisY2 = new QValueAxis();
        axisY2->setTitleText("Mass Activity (arb. u.)");
        axisY2->setLabelFormat("%0.1e");

        chart2->addAxis(axisX2, Qt::AlignBottom);
        chart2->addAxis(axisY2, Qt::AlignLeft);

        // Ensure layout and rendering
        chartView->setChart(chart);
        chartView2->setChart(chart2);

        // Force re-layout and repaint to ensure the title appears
        chartView->update();
        chartView->repaint();

        chartView2->update();
        chartView2->repaint();

        //initialise other stuff needed below

        QVector<QColor> colorList =
            {
                QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"),
                QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"), QColor("#17becf"),
                QColor("#393b79"), QColor("#637939"), QColor("#8c6d31"), QColor("#843c39"), QColor("#7b4173"),
                QColor("#a55194"), QColor("#ce6dbd"), QColor("#9e9ac8"), QColor("#756bb1"), QColor("#636363"),
                QColor("#bd9e39"), QColor("#bdb76b"), QColor("#4daf4a"), QColor("#f781bf"), QColor("#a65628"),
                QColor("#984ea3"), QColor("#999999"), QColor("#ff00ff"), QColor("#7e2f8e"), QColor("#00b4d9"),
                QColor("#17a2b8"), QColor("#ff6f00"), QColor("#006400"), QColor("#808000"), QColor("#ff4500")
            };

        int colorIndex = 0;

        vector<double> min_vals;
        vector<double> max_vals;

        bool is_reduction_current = ui->checkBox_red_curr->isChecked();

        for (auto& part: particles)
        {
            RedirectOutput();
            //////////////////////////// Basically all the Nanocode goes here ////////////////////////////
            try {
                // Example user input
                int atomic_number = part.Get_Z();

                // Initialize maps
                map<int, double> Z_to_radius_Map;
                DefineAtomicRadiusMap(Z_to_radius_Map);

                map<string, int> Geom_to_SurfaceCN_thresholds;
                DefineSurfaceCNValues(Geom_to_SurfaceCN_thresholds);

                map<string, int> Geom_to_BulkCN;
                DefineBulkCN(Geom_to_BulkCN);

                map<int, double> Z_to_mass_map;
                DefineMassMap(Z_to_mass_map);


                // File paths

                string str_coordinates = part.Get_path().toStdString();

                Nanoparticle myNano;

                // Load the cluster geometric data
                myNano.Load_cluster_from_file_comp(str_coordinates);

                // Set bulk radius and perform calculations
                myNano.SetBulkRadius(Z_to_radius_Map[atomic_number]);

                double cutoff_r = CalculateCutoffRadius(atomic_number, Z_to_radius_Map);
                myNano.Find_nearest_neighbors(cutoff_r);
                myNano.Count_CN();

                // Set the shape and identify surface atoms
                string shape = part.Get_shape().toStdString();
                myNano.Set_shape(shape);
                myNano.Identify_Surface_Atoms(Geom_to_SurfaceCN_thresholds);

                // Perform further calculations
                myNano.CalculateAllStrains();

                // Setup reaction

                string reac = ui->comboBox_reaction->currentText().toStdString();

                double coeff;
                if (ui->checkBox_corrections->isChecked())
                    coeff =  ui->lineEdit_coeff->text().toDouble();
                else
                    coeff = 1.0;


                if(reac == "HER")
                {
                    auto DG = (*make_shared<HER_DG_Pt>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());

                    myNano.Setup_Reaction("HER", 0, coeff, DG);
                }
                else if (reac == "CO2RR")
                {
                    auto DG = (*make_shared<CO2RR_DG_Cu>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());

                    myNano.Setup_Reaction("CO2RR", 0, coeff, DG);
                }
                else if(reac == "ARR IPA")
                {
                    auto DG = (*make_shared<ARR_IPA_DG_Pt>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());

                    myNano.Setup_Reaction("ARR_IPA", 0, coeff, DG);
                }
                else if(reac == "ARR Propane")
                {
                    auto DG = (*make_shared<ARR_Prop_DG_Pt>()) + (ui->lineEdit_ZPE->text().toDouble() + ui->lineEdit_TDS->text().toDouble());

                    myNano.Setup_Reaction("ARR_Propane", 0, coeff, DG);
                }
                //other possible reactions here with else-if


                // Calculate GCN based on reaction type
                switch (myNano.Get_reaction().GetAdsorptionType())
                {
                case 0:
                    myNano.Calculate_aGCN(Geom_to_BulkCN);
                    break;
                case 1:
                    myNano.Calculate_bGCN(Geom_to_BulkCN);
                    break;
                case 2:
                    myNano.Calculate_3hGCN(Geom_to_BulkCN);
                    break;
                case 3:
                    myNano.Calculate_4hGCN(Geom_to_BulkCN);
                    break;
                default:
                    cerr << "\n\tInvalid adsorption type!" << endl;
                    QMessageBox::critical(this, "Error", "Invalid adsorption type!");
                    return;
                }

                //cout << myNano.Get_adsorption_sites().size() << " SITES" << endl;

                myNano.Count_gcn_occurences();      /// very important otherwise currents will be 0

                myNano.Print_gcn_occurences();

                ///////////////// here we do the graph stuff    ////////////////


                // Here we do the actual I-V curve
                double P_min = ui->label_minpot->text().toDouble();
                double P_max = ui->label_maxpot->text().toDouble();
                double step = ui->lineEdit_step->text().toDouble();
                double Temp = ui->label_temp_display->text().toDouble();
                ////// the magic happens here ////
                map <double, double>iv_curve = myNano.Calculate_IV_curve(P_min, P_max, step, Temp, is_reduction_current);

                part.Set_i_v(iv_curve);
                // Further options for reduction current and graph normalisation

                QSplineSeries *series = new QSplineSeries();
                series->setName(QString::number(part.Get_num())+ part.Get_shape());     //basically the legend
                series->setColor(colorList[colorIndex % colorList.size()]);

                for (const auto& elem: iv_curve)
                {
                    series->append(elem.first, elem.second);
                }
                chartView->chart()->addSeries(series);
                // setting up x and y axis labels
                series->attachAxis(axisX);
                series->attachAxis(axisY);

                double area= myNano.Calculate_surface_area(Geom_to_BulkCN) * 1E-16;
                double mass = Z_to_mass_map[part.Get_Z()]*part.Get_num() * 1.66053906660E-21;   // num. of atoms * mass of atoms in a.u. * conv factor to mg
                map<double, double> ma_curve;
                for (auto& elem:iv_curve)
                    ma_curve[elem.first] = iv_curve[elem.first]*area/mass;

                QSplineSeries *series2 = new QSplineSeries();
                series2->setName(QString::number(part.Get_num())+ part.Get_shape());     //basically the legend
                series2->setColor(colorList[colorIndex % colorList.size()]);
                colorIndex++;
                for (const auto& elem: ma_curve)
                {
                    series2->append(elem.first, elem.second);
                }
                chartView2->chart()->addSeries(series2);
                // setting up x and y axis labels
                series2->attachAxis(axisX2);
                series2->attachAxis(axisY2);

                // Enable and customize the legend
                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignRight);

                // Enable and customize the legend
                chart2->legend()->setVisible(true);
                chart2->legend()->setAlignment(Qt::AlignRight);

                ///////////////////// end of the nanocode proper //////////////////
            } catch (const std::exception& e)
            {
                QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
                qDebug() << "Exception caught:" << e.what();
            } catch (...)
            {
                QMessageBox::critical(this, "Error", "An unknown error occurred.");
                qDebug() << "Unknown exception caught";
            }
            /////////////////////////////////// END /////////////////////////////////////////////////
        }

        QMessageBox::information(this, "Complete", "Calculations complete!");

        // Set the layout with the chart view
        setLayout(layout);

        // Show the chart view
        chartView->setChart(chart);
        // Show the chart view
        chartView2->setChart(chart2);

        this->showMaximized();
    }
}


void DialogBeau::on_comboBox_reaction_currentTextChanged(const QString &arg1)
{
    string reac = ui->comboBox_reaction->currentText().toStdString();
    if(ui->radioButton_ActvsPot->isChecked())
    {
        if (reac == "HER")
        {
            ui->horizontalSlider_minpot->setValue(-35);
            ui->horizontalSlider_maxpot->setValue(-20);
        }
        else if (reac == "CO2RR")
        {
            ui->horizontalSlider_minpot->setValue(-114);
            ui->horizontalSlider_maxpot->setValue(-108);
        }
        else if (reac == "ARR IPA")
        {
            ui->horizontalSlider_minpot->setValue(0);
            ui->horizontalSlider_maxpot->setValue(20);
        }
        else if (reac == "ARR Propane")
        {
            ui->horizontalSlider_minpot->setValue(-10);
            ui->horizontalSlider_maxpot->setValue(15);
        }

    }
    if (ui->radioButtonActVsSize->isChecked())
    {
        if (reac == "HER")
        {
            ui->horizontalSlider_minpot->setValue(-28);
        }
        else if (reac == "CO2RR")
        {
            ui->horizontalSlider_minpot->setValue(-110);
        }
        else if (reac == "ARR IPA")
        {
            ui->horizontalSlider_minpot->setValue(5);
        }
        else if (reac == "ARR Propane")
        {
            ui->horizontalSlider_minpot->setValue(5);
        }
    }
}


void DialogBeau::on_dial_temp_valueChanged(int value)
{
    ui->label_temp_display->setText(QString::number(value));
}


void DialogBeau::on_horizontalSlider_minpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_minpot->setText(QString::number(real_val, 'f', 2));
}


void DialogBeau::on_horizontalSlider_maxpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_maxpot->setText(QString::number(real_val, 'f', 2));
}


void DialogBeau::on_radioButtonActVsSize_toggled(bool checked)
{
    //radio button logic
    if (checked)
    {
        ui->label_max_Pot->setHidden(true);
        ui->label_maxpot->setHidden(true);
        ui->horizontalSlider_maxpot->setDisabled(true);
        ui->horizontalSlider_maxpot->setHidden(true);
        ui->label_explain_pot_2->setHidden(true);
        ui->label_min_Pot->setText("Potential (V): ");
        ui->lineEdit_step->setHidden(true);
        ui->label_step->setHidden(true);
    }
    else
    {
        ui->label_max_Pot->setHidden(false);
        ui->label_maxpot->setHidden(false);
        ui->horizontalSlider_maxpot->setDisabled(false);
        ui->horizontalSlider_maxpot->setHidden(false);
        ui->label_explain_pot_2->setHidden(false);
        ui->label_min_Pot->setText("Minimum Potential (V): ");
        ui->lineEdit_step->setHidden(false);
        ui->label_step->setHidden(false);
    }
}


void DialogBeau::on_checkBox_corrections_toggled(bool checked)
{
    if (checked)
    {
        ui->lineEdit_ZPE->setHidden(false);
        ui->lineEdit_TDS->setHidden(false);
        ui->lineEdit_coeff->setHidden(false);
        ui->label_coeff->setHidden(false);
        ui->label_TDS->setHidden(false);
        ui->label_ZPE->setHidden(false);

        QMessageBox::warning(this, "READ CAREFULLY!", "Some reactions might already have one or"
        " more of the below corrections applied in the DeltaG. Make sure you "
        "check in Functions.h or the documentation ");
    }
    else
    {
        ui->lineEdit_ZPE->setHidden(true);
        ui->lineEdit_TDS->setHidden(true);
        ui->lineEdit_coeff->setHidden(true);
        ui->label_coeff->setHidden(true);
        ui->label_TDS->setHidden(true);
        ui->label_ZPE->setHidden(true);
    }
}


void DialogBeau::on_pushButton_fileout_clicked()
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

