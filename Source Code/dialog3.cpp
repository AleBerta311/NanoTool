#include "dialog3.h"
#include "ui_dialog3.h"
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <QDir>
#include <fstream>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QSettings>
#include <QPair>
#include <QScatterSeries>
#include <QToolTip>

#include "Nanoparticle.h"       // this includes the other nanocode things
#include "SharedFunctions.h"

Dialog3::Dialog3(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog3)
{
    ui->setupUi(this);

    QWidget::setWindowTitle("Mass Activity vs Size Comparator");
    // reaction choices here
    ui->comboBox_reaction->addItem("ORR");
    //ui->comboBox_reaction->addItem("CO2RR");

    // sliders setup
    ui->horizontalSlider_minpot->setRange(-200, 200);
    ui->horizontalSlider_minpot->setValue(70);

    // dial setup

    ui->dial_temp->setRange(100, 1500);
    ui->dial_temp->setValue(300);

    ui->checkBox_red_curr->setChecked(false);
}

Dialog3::~Dialog3()
{
    delete ui;
}

void Dialog3::on_pushButton_clicked()
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



void Dialog3::on_pushButton_start_clicked()
{
    // Initialize charts and views outside the loop
    QChart *chart = new QChart();
    chart->setTitle("Mass Activity");
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(800, 600);

    // Clear previous series
    chart->removeAllSeries();

    // Create a layout to hold the chart view
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(chartView);

    // Initialize axes once and add them to the chart
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Diameter (nm)");
    axisX->setLabelFormat("%0.3f");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Mass Activity (A/mg)");
    axisY->setLabelFormat("%0.4f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    bool is_reduction = ui->checkBox_red_curr->isChecked();

    // Map to store scatter series for each shape
    QMap<QString, QScatterSeries*> seriesMap;

    // Predefined list of colors for different shapes
    QList<QColor> colors = {Qt::red, Qt::blue, Qt::green, Qt::yellow, Qt::magenta, Qt::cyan, Qt::gray,
                            Qt::darkBlue, Qt::darkRed, Qt::darkMagenta};

    int colorIndex = 0; // Index to cycle through colors

    QVector<qreal> xValues; // Store x values for axisX range
    QVector<qreal> yValues; // Store y values for axisY range

    for (auto& part : particles) {
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
            if (reac == "ORR") {
                auto DG = make_shared<ORR_DG_Pt>();
                myNano.Setup_Reaction("ORR", 0, 12.56, DG);
            }
            // other possible reactions here with else-if

            // Calculate GCN based on reaction type
            switch (myNano.Get_reaction().GetAdsorptionType()) {
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

            myNano.Count_gcn_occurences(); // very important otherwise currents will be 0
            myNano.Print_gcn_occurences();
            myNano.Find_Diameter(); // necessary for the MA vs size graph

            // Here we do the actual calculations of currents
            double Potential = ui->label_minpot->text().toDouble();
            double Temp = ui->label_temp_display->text().toDouble();

            myNano.Calculate_surface_area_void(Geom_to_BulkCN);
            myNano.Calculate_total_mass(Z_to_mass_map, atomic_number);

            double my_mass_act = myNano.Calculate_Mass_Act_at_Pot(Temp, Potential, is_reduction);

            // Prepare data points
            qreal diameter = myNano.Get_Diameter() / 10.;  // Convert to nm
            qreal massActivity = my_mass_act / 1000.;      // Convert to A/mg

            // Store values for range calculation
            xValues.append(diameter);
            yValues.append(massActivity);

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
            series->append(diameter, massActivity);

            // Write output files
            ofstream out;
            out.open("C:/Users/aless/OneDrive/Desktop/QtProject/NanoTool/build/Desktop_Qt_6_7_2_MinGW_64_bit-Debug/debug/MA" + to_string(myNano.Get_atoms().size()) + ".txt");
            out << "Diameter: " << diameter << endl << "MA: " << massActivity << endl;
            out.close();

            myNano.Write_diam_and_surface("C:/Users/aless/OneDrive/Desktop/QtProject/NanoTool/build/Desktop_Qt_6_7_2_MinGW_64_bit-Debug/debug/Diam_Surf" + to_string(myNano.Get_atoms().size()) + ".txt");

        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
            qDebug() << "Exception caught:" << e.what();
        } catch (...) {
            QMessageBox::critical(this, "Error", "An unknown error occurred.");
            qDebug() << "Unknown exception caught";
        }
        /////////////////////////////////// END /////////////////////////////////////////////////
    }

    // After all points have been added, set the axis ranges based on the collected values
    if (!xValues.isEmpty() && !yValues.isEmpty()) {
        axisX->setRange(*std::min_element(xValues.begin(), xValues.end()) - 1, *std::max_element(xValues.begin(), xValues.end()) + 1);
        axisY->setRange(*std::min_element(yValues.begin(), yValues.end()) - 0.1, *std::max_element(yValues.begin(), yValues.end()) + 0.1);
    }

    // Enable and customize the legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    // Set the layout with the chart view
    setLayout(layout);

    // Show the chart view
    chartView->setChart(chart);
}


void Dialog3::on_horizontalSlider_minpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_minpot->setText(QString::number(real_val, 'f', 2));
}


void Dialog3::on_dial_temp_valueChanged(int value)
{
    ui->label_temp_display->setText(QString::number(value));
}


void Dialog3::on_comboBox_reaction_currentTextChanged(const QString &arg1)
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

