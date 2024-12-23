#pragma once

#include <QWidget>
#include <QGraphicsEllipseItem>
#include <QDialog>
#include <QVBoxLayout>

#include <opencv4/opencv2/opencv.hpp>

namespace Ui {
class SimulatorUi;
}

class ControlPoint;
class ScrollableMessageBox;

class ControlPoint : public QGraphicsEllipseItem
{
    public:
        ControlPoint(qreal x, qreal y, qreal width, qreal height, Ui::SimulatorUi* histogramSimulatorUI, QGraphicsItem* parent = nullptr, QString name = "p");

        int xPos;
        int yPos;

    private:
        Ui::SimulatorUi* mHistogramSimulatorUI = nullptr;

    protected:
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

class ScrollableMessageBox : public QDialog
{
    public:
        explicit ScrollableMessageBox(const QString& message, QWidget* parent = nullptr);

    private:
        QVBoxLayout* layout = nullptr;
};

class SimulatorUi : public QWidget
{
    Q_OBJECT

    public:
        explicit SimulatorUi(QWidget *parent = nullptr);
        ~SimulatorUi();

        cv::Mat mOriginal;

    private:
        Ui::SimulatorUi* mUi = nullptr;

        ScrollableMessageBox* mHelpDialog = nullptr;
};

