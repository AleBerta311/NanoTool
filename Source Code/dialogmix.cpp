#include "dialogmix.h"
#include "ui_dialogmix.h"
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <QDir>
#include <fstream>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QSettings>
#include <QPair>
#include <QInputDialog>

#include "Nanoparticle.h"       // this includes the other nanocode things
#include "SharedFunctions.h"

DialogMix::DialogMix(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMix)
{
    ui->setupUi(this);

    QWidget::setWindowTitle("Mixed Sample Analyzer");
    // reaction choices here
    ui->comboBox_reaction->addItem("ORR");
    ui->comboBox_reaction->addItem("CO2RR");

    // sliders setup
    ui->horizontalSlider_minpot->setRange(-300, 300);
    ui->horizontalSlider_minpot->setValue(70);
    ui->horizontalSlider_maxpot->setRange(-300, 300);
    ui->horizontalSlider_maxpot->setValue(110);

    // dial setup

    ui->dial_temp->setRange(100, 1500);
    ui->dial_temp->setValue(300);

    ui->checkBox_red_curr->setChecked(true);
}

DialogMix::~DialogMix()
{
    delete ui;
}

void DialogMix::on_pushButton_add_sample_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Choose .xyz files", QDir::homePath(), "XYZ Files (*.xyz)");
    int file_num = fileNames.size();
    if (!fileNames.isEmpty())
    {
        int i = 0;
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
                QMessageBox::information(nullptr, "File Content", "Isomer " + QString::number(i+1)  + " out of " + QString::number(file_num) + "\nFirst Line: " + firstLine +
                                                                      "\nSecond Line: " + element + "\nThird Line: " + shape_line);

                // Add the particle to the QVector
                QParticle new_particle(element.toInt(), fileName, shape_line, firstLine.toInt());

                // Prompt user for a percentage
                bool ok;
                int percentage = QInputDialog::getInt(this, "Enter Percentage", "Enter the percentage (0-100):", 0, 0, 100, 1, &ok);
                new_particle.Set_weight(percentage);
                particles.push_back(new_particle);
            }
            else
            {
                QMessageBox::warning(nullptr, "Error", "Could not open the file: " + fileName);
            }
            i++;

        }
    }
}


void DialogMix::on_horizontalSlider_maxpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_maxpot->setText(QString::number(real_val, 'f', 2));
}


void DialogMix::on_horizontalSlider_minpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_minpot->setText(QString::number(real_val, 'f', 2));
}


void DialogMix::on_dial_temp_valueChanged(int value)
{
    ui->label_temp_display->setText(QString::number(value));
}


void DialogMix::on_pushButton_start_clicked()
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


    // Create a layout to hold the chart view
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(chartView);
    layout->addWidget(chartView2);

    // Initialize axes once and add them to the chart
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Potential (V)");
    axisX->setLabelFormat("%0.2f");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Sample Current Density (mA/cm^2)");
    axisY->setLabelFormat("%0.2f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Initialize axes once and add them to the chart
    QValueAxis *axisX2 = new QValueAxis();
    axisX2->setTitleText("Potential (V)");
    axisX2->setLabelFormat("%0.2f");

    QValueAxis *axisY2 = new QValueAxis();
    axisY2->setTitleText("Sample Mass Activity (mA/mg)");
    axisY2->setLabelFormat("%0.2f");

    chart2->addAxis(axisX2, Qt::AlignBottom);
    chart2->addAxis(axisY2, Qt::AlignLeft);


    bool is_reduction = ui->checkBox_red_curr->isChecked();
    double P_min = ui->label_minpot->text().toDouble();
    double P_max = ui->label_maxpot->text().toDouble();
    double Temp = ui->label_temp_display->text().toDouble();
    double step = ui->lineEdit_step->text().toDouble();

    // Create vector of the i-v curves to calculated weighted averages later

    vector<map<double, double>> all_i_v_curves;
    vector<map<double, double>> all_ma_curves;


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

            if(reac == "ORR")
            {
                auto DG = make_shared<ORR_DG_Pt>();

                myNano.Setup_Reaction("ORR", 0, 12.56, DG);
            }

            if (reac == "CO2RR")
            {
                auto DG = make_shared<CO2RR_DG_Cu>();
                myNano.Setup_Reaction("CO2RR", 0, 3.01E-14, DG);
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

            myNano.Count_gcn_occurences();      /// very important otherwise currents will be 0

            myNano.Print_gcn_occurences();

            myNano.Find_Diameter();             /// necessary for the MA vs size graph

            ////// the magic happens here ////

            myNano.Calculate_surface_area_void(Geom_to_BulkCN);
            myNano.Calculate_total_mass(Z_to_mass_map, atomic_number);

            map<double, double>i_v = myNano.Calculate_IV_curve(P_min, P_max, step, Temp, is_reduction);
            for (auto& elem: i_v)
                elem.second*=part.Get_weight_perc();

            all_i_v_curves.push_back(i_v);

            ///now do the mass act

            map<double, double> m_a;
            for (auto& elem: i_v)
            {
                double pot = elem.first;
                double j = elem.second;

                m_a[pot] = j*(myNano.Get_Surface_Area() * 1E-16)/myNano.Get_Total_Mass() * part.Get_weight_perc();
                cout << "Multiplying by weight perc" << part.Get_weight_perc() << "from weight" << part.Get_weight() << endl;
            }

            all_ma_curves.push_back(m_a);

            /*
            ofstream out;
            out.open("C:/Users/aless/OneDrive/Desktop/QtProject/NanoTool/build/Desktop_Qt_6_7_2_MinGW_64_bit-Debug/debug/MA" + to_string(myNano.Get_atoms().size())+ ".txt");
            out << "Diameter: " << myNano.Get_Diameter()/10. << endl << "MA: " << my_mass_act/1000. << endl;

            out.close();
            myNano.Write_diam_and_surface("C:/Users/aless/OneDrive/Desktop/QtProject/NanoTool/build/Desktop_Qt_6_7_2_MinGW_64_bit-Debug/debug/Diam_Surf" + to_string(myNano.Get_atoms().size())+ ".txt");
            */

            ///////////////////// end of the nanocode proper //////////////////
        }
        catch (const std::exception& e)
        {
            QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
            qDebug() << "Exception caught:" << e.what();
        }
        catch (...)
        {
            QMessageBox::critical(this, "Error", "An unknown error occurred.");
            qDebug() << "Unknown exception caught";
        }
        /////////////////////////////////// END /////////////////////////////////////////////////
    }

    map <double, double> sample_iv_curve;
    map <double, double> sample_ma_curve;
    vector<double> pots;

    for (int i = 0; i < static_cast<int>((P_max-P_min)/step); i++)
        pots.push_back(P_min + i*step);

    int N_tot = all_i_v_curves.size();
    for (auto& pot : pots)
    {
        double sum_iv = 0;
        double sum_ma = 0;
        for (size_t i = 0; i < N_tot; i++)
        {
            sum_iv += all_i_v_curves[i][pot];
            sum_ma += all_ma_curves[i][pot];
        }

        sample_iv_curve[pot] = sum_iv;
        sample_ma_curve[pot] = sum_ma;

    }

    QSplineSeries *series = new QSplineSeries();
    QSplineSeries *series2 = new QSplineSeries();

    series->setName("sample j");
    series->setName("sample MA");

    for(auto& elem: sample_iv_curve)
        series->append(elem.first, elem.second);
    for(auto& elem: sample_ma_curve)
        series2->append(elem.first, elem.second);

    chartView->chart()->addSeries(series);
    chartView2->chart()->addSeries(series2);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    series2->attachAxis(axisX2);
    series2->attachAxis(axisY2);

    // Enable and customize the legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    // Enable and customize the legend
    chart2->legend()->setVisible(true);
    chart2->legend()->setAlignment(Qt::AlignRight);

    QString out_dir = ui->lineEdit_filename_out->text();
    QString out_iv = out_dir + "/Sample_I_V.txt";
    QString out_ma = out_dir + "/Sample_MA.txt";
    string out_i_v = out_iv.toStdString();
    string out_m_a = out_ma.toStdString();

    ofstream out;

    out.open(out_i_v);
    out << "Pot (V)" << "  " << " j (mA/cm^2)" << endl;
    for (auto & elem: sample_iv_curve)
        out << elem.first << "\t " << elem.second << endl;
    out.close();

    out.open(out_m_a);
    out << "Pot (V)" << "  " << " Mass Act (mA/mg)" << endl;
    for (auto & elem: sample_ma_curve)
        out << elem.first << "\t " << elem.second << endl;
    out.close();

    QMessageBox::information(this, "Complete", "Calculations complete!");

    // Set the layout with the chart view
    setLayout(layout);

    // Show the chart views
    chartView->setChart(chart);
    chartView2->setChart(chart2);

}


void DialogMix::on_pushButton_fileout_clicked()
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

