#include "dialog2.h"
#include "ui_dialog2.h"
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <QDir>
#include <fstream>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QSettings>
#include <QToolTip>

#include "Nanoparticle.h"       // this includes the other nanocode things
#include "SharedFunctions.h"

Dialog2::Dialog2(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog2)
{
    ui->setupUi(this);
    QWidget::setWindowTitle("NanoParticle Comparator");
    // reaction choices here
    ui->comboBox_reaction->addItem("ORR");
    //ui->comboBox_reaction->addItem("CO2RR");

    // sliders setup
    ui->horizontalSlider_minpot->setRange(-200, 200);
    ui->horizontalSlider_maxpot->setRange(-200 , 200);
    ui->horizontalSlider_minpot->setValue(70);
    ui->horizontalSlider_maxpot->setValue(110);

    // dial setup

    ui->dial_temp->setRange(100, 1500);
    ui->dial_temp->setValue(300);

    //checkboxes

    ui->checkBox_SA->setChecked(true);
    ui->checkBox_MA->setChecked(false);
    ui->checkBox_SA->setEnabled(false);
    ui->checkBox_red_curr->setChecked(true);
}

Dialog2::~Dialog2()
{
    delete ui;
}

void Dialog2::on_pushButton_clicked()
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

void Dialog2::on_pushButton_start_clicked()
{
    // Initialize charts and views outside the loop
    QChart *chart = new QChart();
    chart->setTitle("Specific Activity");
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 400);

    QChart *chart2 = new QChart();
    chart2->setTitle("Mass Activity");
    chartView2 = new QChartView(chart2);
    chartView2->setRenderHint(QPainter::Antialiasing);
    chartView2->setMinimumSize(600, 400);
    // Clear previous series
    chart->removeAllSeries();
    // Clear previous series
    chart2->removeAllSeries();

    // Create a layout to hold the chart view
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(chartView);
    layout->addWidget(chartView2);

    // Initialize axes once and add them to the chart
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Potential (V)");
    axisX->setLabelFormat("%0.2f");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current Density (mA/cm^2)");
    axisY->setLabelFormat("%0.2f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Initialize axes once and add them to the chart
    QValueAxis *axisX2 = new QValueAxis();
    axisX2->setTitleText("Potential (V)");
    axisX2->setLabelFormat("%0.2f");

    QValueAxis *axisY2 = new QValueAxis();
    axisY2->setTitleText("Mass Activity (mA/mg)");
    axisY2->setLabelFormat("%0.2f");

    chart2->addAxis(axisX2, Qt::AlignBottom);
    chart2->addAxis(axisY2, Qt::AlignLeft);


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
    bool should_normalise = ui->checkBox_normalise->isChecked();
    bool do_mass_act = ui->checkBox_MA->isChecked();

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
            if(do_mass_act)
            {
                DefineMassMap(Z_to_mass_map);
            }

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

            if(reac == "ORR")
            {
                auto DG = make_shared<ORR_DG_Pt>();

                myNano.Setup_Reaction("ORR", 0, 12.56, DG);
            }
            /*
            if (reac == "CO2RR")
            {
                auto DG = make_shared<CO2RR_DG_Cu>();
                myNano.Setup_Reaction("CO2RR", 0, 3.01E+14, DG);
            }
            */
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
            if (!should_normalise)
            {

                QSplineSeries *series = new QSplineSeries();
                series->setName(QString::number(part.Get_num())+ part.Get_shape());     //basically the legend
                series->setColor(colorList[colorIndex % colorList.size()]);
                colorIndex++;
                for (const auto& elem: iv_curve)
                {
                    series->append(elem.first, elem.second);
                }
                chartView->chart()->addSeries(series);
                // setting up x and y axis labels
                series->attachAxis(axisX);
                series->attachAxis(axisY);
            }
            else
            {
                if (is_reduction_current)
                {
                    // find the min val to do the normalisation (the most negative value)
                    double min_val = std::numeric_limits<double_t>::max();
                    for (const auto& pair : iv_curve)
                        min_val = std::min(min_val, pair.second);

                    min_val = -1.*min_val;      // we don't want the graph to be flipped
                    min_vals.push_back(min_val);
                }
                else
                {
                    // find the max val to do the normalisation
                    double max_val = std::numeric_limits<double_t>::min();
                    for (const auto& pair : iv_curve)
                        max_val = std::max(max_val, pair.second);
                    max_vals.push_back(max_val);
                }

            }

            // if we have to do mass activity as well (No normalisation)

            if (do_mass_act)
            {
                double area= myNano.Calculate_surface_area(Geom_to_BulkCN) * 1E-16;
                double mass = Z_to_mass_map[part.Get_Z()]*part.Get_num() * 1.66053906660E-21;   // num. of atoms * mass of atoms in a.u. * conv factor to mg
                map<double, double> ma_curve;
                for (auto& elem:iv_curve)
                    ma_curve[elem.first] = iv_curve[elem.first]*area/mass;

                QSplineSeries *series2 = new QSplineSeries();
                series2->setName(QString::number(part.Get_num())+ part.Get_shape());     //basically the legend
                colorIndex--;
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
            }

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

    if(should_normalise)
    {
        axisY->setTitleText("Current density normalised");
        for (auto& part : particles)
        {
            map<double, double> retrieved_curve = part.Get_i_v();
            qDebug() << "IV Curve size:" << retrieved_curve.size();

            if (retrieved_curve.empty())
            {
                qDebug() << "Warning: Retrieved I-V Curve is empty!";
            }

            // Create and append to QSplineSeries
            QSplineSeries *series = new QSplineSeries();
            series->setName(QString::number(part.Get_num()) + part.Get_shape());
            series->setColor(colorList[colorIndex % colorList.size()]);
            colorIndex++;

            if (is_reduction_current)
            {
                axisY->setRange(-1., 0.);
                auto normconst_it = std::max_element(min_vals.begin(), min_vals.end());
                double normconst = *normconst_it;

                for (const auto& elem : retrieved_curve)
                {
                    double normalized_value = elem.second / normconst;
                    series->append(elem.first, normalized_value);
                    qDebug() << "Appending to series - Potential:" << elem.first << ", Normalized Current:" << normalized_value;
                }
            }
            else
            {
                axisY->setRange(0., 1.);
                auto normconst_it = std::max_element(max_vals.begin(), max_vals.end());
                double normconst = *normconst_it;

                for (const auto& elem : retrieved_curve)
                {
                    double normalized_value = elem.second / normconst;
                    series->append(elem.first, normalized_value);
                    qDebug() << "Appending to series - Potential:" << elem.first << ", Normalized Current:" << normalized_value;
                }
            }

            // Add series to chart and attach axes
            chartView->chart()->addSeries(series);
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }


    }

    if(ui->checkBoxSAlimit->isChecked())
    {
        if(is_reduction_current)
            axisY->setRange(-1., 0.);
        else
            axisY->setRange(0., 1.);
    }

    if(ui->checkBoxMAlimit->isChecked())
    {
        if(is_reduction_current)
            axisY2->setRange(-1., 0.);
        else
            axisY2->setRange(0., 1.);
    }

    QMessageBox::information(this, "Complete", "Calculations complete!");

    // Set the layout with the chart view
    setLayout(layout);

    // Show the chart view
    chartView->setChart(chart);

    if(do_mass_act)
    {
        // Show the chart view
        chartView2->setChart(chart2);
    }

    this->showMaximized();

}

void Dialog2::on_horizontalSlider_minpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_minpot->setText(QString::number(real_val, 'f', 2));
}


void Dialog2::on_horizontalSlider_maxpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_maxpot->setText(QString::number(real_val, 'f', 2));
}




void Dialog2::on_dial_temp_valueChanged(int value)
{
    ui->label_temp_display->setText(QString::number(value));
}


void Dialog2::on_comboBox_reaction_currentTextChanged(const QString &arg1)
{
    bool stop = false;
    for(auto& part: particles)
    {
        if (part.Get_Z() != 29 && ui->comboBox_reaction->currentText().toStdString() == "CO2RR")
        {
            stop = true;
            break;
        }
    }

    if (stop)
    {
        QMessageBox::critical(this, "Nah", "Can't do CO2RR without Copper");
        this->close();
    }
}

