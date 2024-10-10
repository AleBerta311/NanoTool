#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog1.h"
#include "dialog2.h"
#include "dialog3.h"
#include "credit_dialog.h"
#include "sitesdialog.h"
#include "dialogmix.h"
#include "dialogbeau.h"
#include <QPropertyAnimation>
#include <QTimer>
#include <QParallelAnimationGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pushButton_AnalyzeShape->hide();
    ui->pushButton_2->hide();
    ui->pushButton_compare_shapes->hide();
    ui->pushButton_sizeactivity->hide();

    // we have to use "hide" instead of SetVisible otherwise stuff doesnt work...how fun

    ui->pushButton_activityanalysis->installEventFilter(this);
    /////////////////////////////////////////////////
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_AnalyzeShape_clicked()
{
    //Modal approach to open new window
    Dialog1 dial;
    dial.setModal(true);
    dial.exec();
}


void MainWindow::on_pushButton_compare_shapes_clicked()
{
    //Modal approach to open new window
    Dialog2 dial;
    dial.setModal(true);
    dial.exec();
}



void MainWindow::on_pushButton_clicked()
{
    //Modal approach to open new window
    credit_dialog dial;
    dial.setModal(true);
    dial.exec();
}


void MainWindow::on_pushButton_AnalyzeSites_clicked()
{
    //Modal approach to open new window
    SitesDialog dial;
    dial.setModal(true);
    dial.exec();
}


void MainWindow::on_pushButton_sizeactivity_clicked()
{
    //Modal approach to open new window
    Dialog3 dial;
    dial.setModal(true);
    dial.exec();
}


void MainWindow::on_pushButton_2_clicked()
{
    //Modal approach to open new window
    DialogMix dial;
    dial.setModal(true);
    dial.exec();
}


void MainWindow::on_pushButton_3_clicked()
{
    //Modal approach to open new window
    DialogBeau dial;
    dial.setModal(true);
    dial.exec();
}


void MainWindow::on_pushButton_activityanalysis_clicked()
{
    animateButtons(ui->pushButton_2, !ui->pushButton_2->isVisible());
    animateButtons(ui->pushButton_AnalyzeShape, !ui->pushButton_AnalyzeShape->isVisible());
    animateButtons(ui->pushButton_compare_shapes, !ui->pushButton_compare_shapes->isVisible());
    animateButtons(ui->pushButton_sizeactivity, !ui->pushButton_sizeactivity->isVisible());
}

// to animate part of the menu
void MainWindow::animateButtons(QPushButton* button, bool show)
{
    // Get the current geometry of the button
    QRect startRect = button->geometry();
    QRect endRect = startRect;

    // Create a new animation
    QPropertyAnimation *animation = new QPropertyAnimation(button, "geometry");
    animation->setDuration(300);  // Animation duration in milliseconds

    if (show)
    {
        // Show the button with animation
        if (startRect.height() == 0)
        {
            // Use a default height if the current height is 0
            int defaultHeight = button->sizeHint().height();
            startRect.setHeight(0); // Start from height 0
            button->setGeometry(QRect(startRect.x(), startRect.y(), startRect.width(), 0)); // Ensure the button is resized to height 0
            endRect.setHeight(defaultHeight); // End at default height
        }
        else
        {
            endRect.setHeight(startRect.height()); // Use existing height if not zero
        }

        // Set the start and end values for the animation
        animation->setStartValue(startRect);
        animation->setEndValue(endRect);
        button->setVisible(true); // Make sure the button is visible before starting animation
    }
    else
    {
        // Hide the button with animation
        endRect.setHeight(0); // Animate to height 0
        animation->setStartValue(startRect);
        animation->setEndValue(endRect);

        // Hide button after animation completes
        connect(animation, &QPropertyAnimation::finished, [button]() {
            button->setVisible(false); // Hide button after animation finishes
        });
    }

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}




// to detect hovering on the analyze button in the menu
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static QTimer hideTimer;

    if (watched == ui->pushButton_activityanalysis)
    {
        if (event->type() == QEvent::Enter)
        {
            qDebug() << "Mouse entered button area";
            hideTimer.stop();  // Stop any pending hide timer

            // Show buttons with animation
            animateButtons(ui->pushButton_2, true);
            animateButtons(ui->pushButton_AnalyzeShape, true);
            animateButtons(ui->pushButton_compare_shapes, true);
            animateButtons(ui->pushButton_sizeactivity, true);
        }
        else if (event->type() == QEvent::Leave)
        {
            qDebug() << "Mouse left button area";

            hideTimer.singleShot(300, [this]() {
                if (!ui->pushButton_2->underMouse() &&
                    !ui->pushButton_AnalyzeShape->underMouse() &&
                    !ui->pushButton_compare_shapes->underMouse() &&
                    !ui->pushButton_sizeactivity->underMouse())
                {
                    qDebug() << "Hiding buttons";

                    // Hide buttons with animation
                    animateButtons(ui->pushButton_2, false);
                    animateButtons(ui->pushButton_AnalyzeShape, false);
                    animateButtons(ui->pushButton_compare_shapes, false);
                    animateButtons(ui->pushButton_sizeactivity, false);
                }
            });
        }
    }

    return QMainWindow::eventFilter(watched, event);
}



