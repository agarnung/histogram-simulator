#pragma once

#include "bezierinterpolator.h"

#include <QWidget>
#include <QJsonDocument>
#include <QInputDialog>
#include <QColorDialog>
#include <QJsonObject>
#include <QFileDialog>
#include <QJsonArray>
#include <QString>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QtGlobal>
#include <QRegularExpressionValidator>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QDebug>
#include <QScreen>
#include <QGraphicsSceneMouseEvent>
#include <QObject>
#include <QMessageBox>

#include <opencv4/opencv2/opencv.hpp>

namespace Ui {
class SimulatorUi;
}

class SimulatorUi;
class ControlPoint;
class ScrollableMessageBox;

class ControlPoint : public QGraphicsEllipseItem
{
    public:
        ControlPoint(qreal x, qreal y, qreal width, qreal height, SimulatorUi* histogramSimulatorUI, QGraphicsItem* parent = nullptr, QString name = "p");

        int xPos;
        int yPos;

    private:
        SimulatorUi* mSimulatorUi = nullptr;

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
        QImage mOriginalHistogram, mModifiedHistogram;
        QPainterPath mPainterPath;
        QVector<QPointF*> mControlPoints;// En el espacio 2D de la rejilla
        QVector<qreal> mKnotVector;
        QGraphicsScene* mScene = nullptr;
        int mPointsNumber = 2; // Sin contar a los dos fijos
        const int mControlPointSize = 10;
        int mMargin = 0;
        QRectF mSceneRect;
        QPolygonF mInterpolatedPoints, mBoorNetPoints;

        cv::Mat mModified;
        cv::Mat mLUT; // Vector 1D de 0-255 puntos que mapean a salidas entre 0 y 1

        void initializeSimulator();
        void updateRangeForSlidersAndSpinboxes();
        void readJson(const QString& jsonPath);
        void loadImageInView();
        void loadHistogramInView();
        void updateView(QPointF* skipPoint = nullptr); /// Actualiza y lo muestra la vista (curva) en la escena en función de los puntos de control actuales
        void interpolateCurve(); /// Calcula nuevos puntos de control con el algoritmo de Boor; rompe la curva en múltiples curvas de Bezier e interpola cada una

    private:
        Ui::SimulatorUi* mUi = nullptr;

        ScrollableMessageBox* mHelpDialog = nullptr;
        BezierInterpolator mBezierInterpolator;
        QHash<ControlPoint*, QPointF*> itemToPoint; /// Mapear items a posiciones en la escena

        QImage calculateHistogram(const cv::Mat& image);
        QPainterPath calculatePath(QList<QPoint> pointList);
        QImage createHistogramImage(cv::Mat histImageMat, int histSize);
        void createLUT(); // Hay que discretizar en 256 valores...
        void saveJSON(); // Guardar mControlPoints o mInterpolatedPoints
        void setupView();
        void fillKnotVector(); /// knotVector with knots for uniform cubic B-spline that passes through endpoints
        cv::Mat parseJSONQStringToMat(QString& json);

    signals:
        void sg_saveJSON(const QJsonDocument& json_d);

    private slots:
        void on_deleteLastPointButton_released();
        void on_saveTfButton_released();
        void on_saveImagesButton_released();
        void on_addPointButton_released();
        void on_leftSlider_valueChanged(int position);
        void on_rightSlider_valueChanged(int position);
        void on_cargarButton_released();
        void on_transformButton_released();
        void on_YButton_released();
        void on_leftVerticalSlider_valueChanged(int position);
        void on_rightVertSlider_valueChanged(int position);
        void on_comboBox_activated(const QString& arg1);
        void on_helpButton_released();
        void on_createLUTControl_released();
        void on_saveCurveButton_released();
        void on_loadCurveButton_released();
        void on_fullscreenButton_released();

        friend class ControlPoint;
};

