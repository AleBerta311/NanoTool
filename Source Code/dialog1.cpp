#include "dialog1.h"
#include "ui_dialog1.h"
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <QDir>
#include <fstream>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QSettings>
#include <QPen>

#include "Nanoparticle.h"   // this already contains the other nanocode .h files
#include "SharedFunctions.h"

Dialog1::Dialog1(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog1)
{
    ui->setupUi(this);
    QWidget::setWindowTitle ("NanoTool analyzer");
    // reaction choices here
    ui->comboBox_reaction->addItem("ORR");
    //ui->comboBox_reaction->addItem("CO2RR");

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

    // sliders setup
    ui->horizontalSlider_minpot->setRange(-300, 300);
    ui->horizontalSlider_maxpot->setRange(-300,300);
    ui->horizontalSlider_minpot->setValue(70);
    ui->horizontalSlider_maxpot->setValue(110);

    // dial setup

    ui->dial_temp->setRange(100, 1500);
    ui->dial_temp->setValue(300);
    // Checkboxes
    ui->checkBox_MA->setChecked(false);

}

Dialog1::~Dialog1()
{
    delete ui;
}

void Dialog1::on_pushButton_start_clicked()
{
    RedirectOutput();

    bool do_mass_act = ui->checkBox_MA->isChecked();

    QChart *chart = new QChart();
    chart->setTitle("I/V graph");
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 400);

    QChart *chart2 = new QChart();
    chart2->setTitle("Mass Activity");
    chartView2 = new QChartView(chart2);
    chartView2->setRenderHint(QPainter::Antialiasing);
    chartView2->setMinimumSize(600, 400);


    layout = new QHBoxLayout();
    layout->addWidget(chartView);
    layout->addWidget(chartView2);
    setLayout(layout);

    // Initialize axes for MA and add them to the chart
    QValueAxis *axisX2 = new QValueAxis();
    axisX2->setTitleText("Potential (V)");
    axisX2->setLabelFormat("%0.2f");

    QValueAxis *axisY2 = new QValueAxis();
    axisY2->setTitleText("Mass Activity (mA/mg)");
    axisY2->setLabelFormat("%0.2f");

    chart2->addAxis(axisX2, Qt::AlignBottom);
    chart2->addAxis(axisY2, Qt::AlignLeft);

    // Initialize SA axes
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Potential (V)");
    axisX->setLabelFormat("%0.2f");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Current Density (mA/cm^2)");
    axisY->setLabelFormat("%0.2f");
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    QSplineSeries *series = new QSplineSeries();
    series->setName("current density");     //basically the legend

    // Set the color of the series
    QPen pen(Qt::darkYellow); // Idk it looks cool
    pen.setWidth(2); // Optional: Set the width of the line
    series->setPen(pen);

    try {
        // Example user input
        int atomic_number = ui->lineEdit_atomicZ->text().toInt();

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
        string str_coordinates = filename_in.toStdString();

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

        myNano.Count_gcn_occurences();      /// very important otherwise currents will be 0

        myNano.Print_gcn_occurences();

        ///////////////// here we do the graph stuff and output   ////////////////

        QString out_dir = ui->lineEdit_filename_out->text();
        QString out_file1 = out_dir + "/Surf.xyz";
        QString out_file2 = out_dir + "/Sites.xyz";
        QString outcurr = out_dir + "/current_densities.txt";
        string out1 = out_file1.toStdString();
        string out2 = out_file2.toStdString();
        string out_curr = outcurr.toStdString();

        myNano.Write_surface_atoms(out1);
        myNano.Write_site_coords(out2);

        ofstream outj;
        outj.open(out_curr);

        // Here we do the actual I-V curve
        double P_min = ui->label_minpot->text().toDouble();
        double P_max = ui->label_maxpot->text().toDouble();
        double step = ui->lineEdit_step->text().toDouble();
        double Temp = ui->label_temp_display->text().toDouble();
        bool is_reduction_current = ui->checkBox_red_curr->isChecked();
        bool should_normalise = ui->checkBox_normalise->isChecked();

        ////// the magic happens here ////
        map<double, double> iv_curve = myNano.Calculate_IV_curve(P_min, P_max, step, Temp, is_reduction_current);

        // Further options for reduction current and graph normalisation
        if (!should_normalise)
        {
            for (const auto& elem: iv_curve)
            {
                series->append(elem.first, elem.second);
                outj << "\nPotential: " << elem.first << "\tj: " << elem.second << endl;
            }
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
                for (const auto& elem: iv_curve)
                {
                    series->append(elem.first, elem.second/min_val);
                    outj << "\nPotential: " << elem.first << "\tj: " << elem.second/min_val << endl;
                }
            }
            else
            {
                // find the max val to do the normalisation
                double max_val = std::numeric_limits<double_t>::min();
                for (const auto& pair : iv_curve)
                    max_val = std::min(max_val, pair.second);

                for (const auto& elem: iv_curve)
                {
                    series->append(elem.first, elem.second/max_val);
                    outj << "\nPotential: " << elem.first << "\tj: " << elem.second/max_val << endl;
                }
            }
        }

        outj.close();

        // if we have to do mass activity as well (No normalisation)
        if (do_mass_act)
        {
            double area= myNano.Calculate_surface_area(Geom_to_BulkCN) * 1E-16;
            double mass = Z_to_mass_map[atomic_number]*myNano.Get_atoms().size() * 1.66053906660E-21;   // num. of atoms * mass of atoms in a.u. * conv factor to mg
            map<double, double> ma_curve;
            for (auto& elem:iv_curve)
                ma_curve[elem.first] = iv_curve[elem.first]*area/mass;

            QSplineSeries *series2 = new QSplineSeries();
            series2->setName(QString::number(myNano.Get_atoms().size()));     //basically the legend
            for (const auto& elem: ma_curve)
            {
                series2->append(elem.first, elem.second);
            }
            chartView2->chart()->addSeries(series2);
            // setting up x and y axis labels
            series2->attachAxis(axisX2);
            series2->attachAxis(axisY2);
        }

        chartView->chart()->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        if(should_normalise)
            axisY->setTitleText("Current density normalised");
        if (ui->checkBox_limityaxis->isChecked())
        {
            if (is_reduction_current)
                axisY->setRange(-1.,0.);
            else
                axisY->setRange(0., 1.);
        }

        // Enable and customize the legend
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
        chart2->legend()->setVisible(true);
        chart2->legend()->setAlignment(Qt::AlignRight);

        chartView->setChart(chart);
        if(do_mass_act)
        {
            chartView2->setChart(chart2);
        }

        ui->label_finish->setText("Operation Complete!");
        QMessageBox::information(this, "Complete", "Calculations complete!");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromStdString(e.what()));
        qDebug() << "Exception caught:" << e.what();
    } catch (...) {
        QMessageBox::critical(this, "Error", "An unknown error occurred.");
        qDebug() << "Unknown exception caught";
    }

}


void Dialog1::on_pushButton_filein_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Choose .xyz file", QDir::homePath());
    filename_in = file_name;
    ui->lineEdit_filename->setText(file_name);
}


void Dialog1::on_pushButton_fileout_clicked()
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



void Dialog1::on_horizontalSlider_minpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_minpot->setText(QString::number(real_val, 'f', 2));
}


void Dialog1::on_horizontalSlider_maxpot_valueChanged(int value)
{
    double real_val = value/100.;
    ui->label_maxpot->setText(QString::number(real_val, 'f', 2));
}


void Dialog1::on_dial_temp_valueChanged(int value)
{
    ui->label_temp_display->setText(QString::number(value));
}




void Dialog1::on_comboBox_reaction_currentTextChanged(const QString &arg1)
{
    QString reac = ui->comboBox_reaction->currentText();
    if (reac == "ORR")
    {
        ui->lineEdit_atomicZ->setText(QString::number(78));
        ui->lineEdit_atomicZ->setDisabled(true);
        ui->checkBox_red_curr->setChecked(true);
    }

    if (reac == "CO2RR")
    {
        ui->lineEdit_atomicZ->setText(QString::number(29));
        ui->lineEdit_atomicZ->setDisabled(true);
        ui->checkBox_red_curr->setChecked(true);
    }


}

