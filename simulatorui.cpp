#include "simulatorui.h"
#include "ui_simulatorui.h"

ScrollableMessageBox::ScrollableMessageBox(const QString &message, QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint)
{
    this->setWindowTitle("Ayuda");
    this->setMinimumSize(100, 100);
    this->setGeometry(parent->geometry().x() + 200, parent->geometry().y() + 100, 1000, 500);
    this->adjustSize();

    setWindowFlag(Qt::WindowStaysOnTopHint, false);

    layout = new QVBoxLayout(this);

    QTextEdit* textEdit = new QTextEdit(this);
    textEdit->setHtml(message);
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString styleSheet =
        "body { font-size: 13px; font-family: Arial; color: #FFF; text-align: justify; margin-right: 10px; }"
        "h2 { font-size: 15px; font-weight: bold; color: #007bff; }"
        "li { margin-bottom: 8px; }"
        "p { color: #FF0000; }";

    textEdit->document()->setDefaultStyleSheet(styleSheet);

    layout->addWidget(textEdit);
}

SimulatorUi::SimulatorUi(QWidget* parent) :
    QWidget(parent),
    mUi(new Ui::SimulatorUi)
{
    mUi->setupUi(this);

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    for (int i = 0; i <= 255; ++i)
        mUi->comboBox->addItem(QString::number(i));
    mUi->comboBox->setCurrentIndex(0);
    mUi->lineEdit->setText(QString::number(0));
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(QRegularExpression("^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9]?[0-9])$"));
    mUi->lineEdit->setValidator(validator);

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/iconosHS/iconosHS/help-circle.svg"), QSize(), QIcon::Normal, QIcon::Off);
    mUi->helpButton->setIcon(icon);
    mUi->helpButton->setIconSize(QSize(25, 25));

    mLUT = cv::Mat(256, 1, CV_8U);
    uchar* p = mLUT.ptr();
    for (unsigned int i = 0; i < 256; ++i) // Inicialmente la LUT es una diagonal
        p[i] = i;
}

SimulatorUi::~SimulatorUi()
{
    if (mUi)
    {
        delete mUi;
        mUi = nullptr;
    }

    if (mHelpDialog != nullptr)
    {
        delete mHelpDialog;
        mHelpDialog = nullptr;
    }
}

void SimulatorUi::initializeSimulator()
{
    QFile styleFile(":/configsHS/configsHS/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        this->setStyleSheet(styleSheet);

        styleFile.close();
    }
    else
        qCritical() << QObject::tr("%1 - %2 - No se pudo abrir la hoja de estilos").arg(this->metaObject()->className()).arg(__func__);

    setupView();
    this->update();
}

void SimulatorUi::readJson(const QString& jsonPath)
{
    // QString json = ...;
    // mLUT = parseJSONQStringToMat(json);
}

void SimulatorUi::loadImageInView()
{
    QImage originalQImage(mOriginal.data, mOriginal.cols, mOriginal.rows, static_cast<int>(mOriginal.step), QImage::Format_Grayscale8);
    mUi->original->setPixmap(QPixmap::fromImage(originalQImage));
}

void SimulatorUi::loadHistogramInView()
{
    mOriginalHistogram = calculateHistogram(mOriginal);
    mOriginalHistogram = mOriginalHistogram.scaled(mSceneRect.width(), mSceneRect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    mUi->originalHist->setPixmap(QPixmap::fromImage(mOriginalHistogram));

    QImage imageWithDiagonal = mOriginalHistogram.copy();
    QPainter painter(&imageWithDiagonal);
    QPen pen(QColor(40, 215, 40, 170));
    pen.setWidth(2);
    pen.setStyle(Qt::DashDotLine);
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.drawLine(0, imageWithDiagonal.height(), imageWithDiagonal.width(), 0);
    painter.end();

    QGraphicsPixmapItem* histogramItem = new QGraphicsPixmapItem(QPixmap::fromImage(imageWithDiagonal));
    histogramItem->setPos(mSceneRect.topLeft() + QPointF((mSceneRect.width() - imageWithDiagonal.width()) / 2, (mSceneRect.height() - imageWithDiagonal.height()) / 2));
    histogramItem->setZValue(-1);
    mUi->graphicsView->scene()->addItem(histogramItem);
}

void SimulatorUi::updateView(QPointF* skipPoint)
{
    loadHistogramInView();

    // Show interpolated curve.
    interpolateCurve();
    for (QPolygonF::iterator pointIt = mInterpolatedPoints.begin(), pointEnd = mInterpolatedPoints.end(); pointIt != pointEnd; ++pointIt)
    {
        if (pointIt != mInterpolatedPoints.end() - 1)
            mScene->addLine(QLineF(*pointIt, *(pointIt + 1)), QPen("black"));
        mScene->addEllipse(pointIt->x() - 2, pointIt->y() - 2, 4, 4, QPen("black"), QBrush("black"));
    }

    fillKnotVector();

    // Show control points (menos los fijos)
    int number = 0;
    for (QVector<QPointF*>::iterator pointIt = mControlPoints.begin(), pointEnd = mControlPoints.end(); pointIt != pointEnd; ++pointIt)
    {
        if (pointIt != mControlPoints.end() - 1)
            mScene->addLine(QLineF(**pointIt, **(pointIt + 1)), QPen(QColor(0, 0, 255, 50)));
        if (skipPoint && skipPoint == *pointIt)
            continue;

        ControlPoint* pointItem = new ControlPoint((*pointIt)->x() - mControlPointSize / 2, (*pointIt)->y() - mControlPointSize / 2, mControlPointSize, mControlPointSize, this, nullptr, QString::number(number));
        itemToPoint[pointItem] = *pointIt;
        if (pointIt == mControlPoints.begin() + 1)
        {
            pointItem->setBrush(QBrush(QColor(0, 0, 0, 50)));
            pointItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
        else if (pointIt == mControlPoints.begin() || pointIt == mControlPoints.end() - 1)
        {
            pointItem->setBrush(QBrush(QColor(0, 0, 0, 255)));
            pointItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        else
        {
            pointItem->setBrush(QBrush(QColor(0, 0, 255, 255)));
            pointItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        }

        mScene->addItem(pointItem);
        ++number;
    }

    // Show boor net points
    for (QPolygonF::iterator pointIt = mBoorNetPoints.begin(), pointEnd = mBoorNetPoints.end(); pointIt != pointEnd; ++pointIt)
    {
        if (pointIt != mBoorNetPoints.end() - 1)
            mScene->addLine(QLineF(*pointIt, *(pointIt + 1)), QPen("red"));
        mScene->addEllipse(pointIt->x() - 2, pointIt->y() - 2, 4, 4, QPen("green"),  QBrush("green"));
    }
}

void SimulatorUi::interpolateCurve()
{
    if (mControlPoints.size() < 3 || mKnotVector.size() < 5)
        return;

    mInterpolatedPoints.clear();
    mBezierInterpolator.CalculateBoorNet(mControlPoints, mKnotVector, mBoorNetPoints);
    mInterpolatedPoints.push_back(*(mControlPoints.first()));
    for (int counter = 0; counter < mBoorNetPoints.size() - 3; counter += 3)
        mBezierInterpolator.InterpolateBezier(mBoorNetPoints[counter], mBoorNetPoints[counter + 1], mBoorNetPoints[counter + 2], mBoorNetPoints[counter + 3], mInterpolatedPoints);
    mInterpolatedPoints.push_back(*(mControlPoints.last()));
}

void SimulatorUi::on_deleteLastPointButton_released()
{
    if (!mScene || mPointsNumber < 3)
        return;

    --mPointsNumber;
    QVector<QPointF*>::iterator it = mControlPoints.end() - 2; // Eliminar el penúltimo hasta que haya 3 (inmutables)
    mControlPoints.erase(it);
    fillKnotVector();
    mScene->clear();
    updateView();
}

void SimulatorUi::on_saveTfButton_released()
{
    //	createLUT();
    saveJSON(); // Parsear LUT a JSON y guardar
}

void SimulatorUi::on_addPointButton_released()
{
    if (!mScene || mControlPoints.size() < 3)
        return;

    // Calcular dónde insertar
    int x_max = mControlPoints.at(mControlPoints.size() - 1)->x(); // Punto derecha
    int y_border = mSceneRect.height();
    QPointF leftestPoint(0, 0);
    QVector<QPointF*> aux = mControlPoints.mid(1, mControlPoints.size() - 2); // Excluir puntos fijos
    for (const auto& point : aux)
    {
        if (point->x() > leftestPoint.x())
            leftestPoint = *point;
    }
    int x = qMin((int) leftestPoint.x() + (int)QRandomGenerator::global()->generate() % 20 + 15, x_max);
    int y = qMin((int) leftestPoint.y() + (int)QRandomGenerator::global()->generate() % 20, y_border);

    // Insertar como penúltimo
    QVector<QPointF*>::iterator it = mControlPoints.end() - 1;
    mControlPoints.insert(it, new QPointF(x, y));

    fillKnotVector();

    ++mPointsNumber;
    mScene->clear();
    updateView();
}

QImage SimulatorUi::calculateHistogram(const cv::Mat& image)
{
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };
    bool uniform = true;
    bool accumulate = false;
    cv::Mat histImageMat;

    cv::calcHist(&image, 1, 0, cv::Mat(), histImageMat, 1, &histSize, &histRange, uniform, accumulate);
    //	std::cout << std::endl << std::endl << "histImageMat: " << std::endl;
    //	for (unsigned int i = 0; i < 256; ++i)
    //		std::cout << i << ":" << QString::number(histImageMat.at<float>(i)).toStdString() << "  ";

    return createHistogramImage(histImageMat, histSize);
}

QImage SimulatorUi::createHistogramImage(cv::Mat histImageMat, int histSize)
{
    int hist_w = 1200;
    int hist_h = 800;
    int bin_w = cvRound((double)hist_w / histSize);

    QImage histImage(hist_w, hist_h, QImage::Format_RGB32);
    histImage.fill(QColor(220, 220, 220, 150));

    QPainter painter(&histImage);
    //	painter.setRenderHint(QPainter::Antialiasing);

    cv::normalize(histImageMat, histImageMat, 0, hist_h, cv::NORM_MINMAX, -1, cv::Mat());

    for (int i = 1; i < histSize; i++)
    {
        float value = histImageMat.at<float>(i);
        QRectF rect(bin_w * (i - 1), hist_h - 1, bin_w, -value);

        painter.setBrush(QColor(255, 0, 0, 150));
        painter.drawRect(rect);
    }

    painter.end();

    return histImage;
}

void SimulatorUi::createLUT()
{
    interpolateCurve();

    mLUT.setTo(0);
    int leftFixedPointX = static_cast<int>(std::round((mControlPoints.front()->x() / mSceneRect.width()) * 255.0));
    for (int i = 0; i <= leftFixedPointX; ++i)
        mLUT.at<uchar>(i) = 0;

    for (int i = 0; i < mInterpolatedPoints.size() - 1; ++i)
    {
        QPointF startPoint = mInterpolatedPoints[i];
        QPointF endPoint = mInterpolatedPoints[i + 1];

        double startGridX = (startPoint.x() / mSceneRect.width()) * 255.0;
        double startGridY = ((mSceneRect.height() - startPoint.y()) / (mSceneRect.height())) * 255.0; // Porque "y" apunta hacia abajo
        double endGridX = (endPoint.x() / mSceneRect.width()) * 255.0;
        double endGridY = ((mSceneRect.height() - endPoint.y()) / mSceneRect.height()) * 255.0;

        for (int j = startGridX; j <= endGridX; ++j)
        {
            double m = ((double)endGridY - startGridY) / ((double)endGridX - startGridX);
            double b = startGridY - m * startGridX;
            double interpolatedY = j * m + b;

            interpolatedY = static_cast<uchar>(std::round(interpolatedY));
            mLUT.at<uchar>(j) = cv::saturate_cast<uchar>(interpolatedY);
        }
    }

    double rightFixedPointX = static_cast<int>(std::round((mControlPoints.back()->x() / mSceneRect.width()) * 255.0));
    for (int i = static_cast<int>(rightFixedPointX); i < 256; ++i)
        mLUT.at<uchar>(i) = 255;

    //	std::cout << std::endl << std::endl << "mLUT: " << std::endl ;
    //	for (unsigned int i = 0; i < 256; ++i)
    //		std::cout << i << ":" << QString::number(mLUT.at<uchar>(i)).toStdString() << "  ";
}

void SimulatorUi::saveJSON()
{
    QJsonArray jsonArray;

    for (int i = 0; i < mLUT.rows; ++i)
    {
        QJsonObject lutEntry;
        lutEntry["x"] = i;
        lutEntry["y"] = static_cast<int>(mLUT.at<uchar>(i));
        jsonArray.append(lutEntry);
    }

    QJsonObject json_o;
    json_o["dataLUT"] = jsonArray;

    const QJsonDocument json_d(json_o);
    emit sg_saveJSON(json_d);
}

void SimulatorUi::setupView()
{
    mUi->graphicsView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mUi->graphicsView->setRenderHint(QPainter::Antialiasing, true);
    mUi->graphicsView->setRenderHint(QPainter::TextAntialiasing, true);

    mScene = new QGraphicsScene();
    mScene->setSceneRect(0, 0, mUi->graphicsView->width(), mUi->graphicsView->height());
    //	mScene->sceneRect().adjust(mMargin, mMargin, -mMargin, -mMargin);
    mUi->graphicsView->setScene(mScene);
    mScene->clear();
    mSceneRect = mScene->sceneRect().adjusted(mMargin, mMargin, -mMargin, -mMargin);
    //	mSceneRect = mScene->sceneRect();

    mUi->leftSlider->setRange(0, mSceneRect.width());
    mUi->rightSlider->setRange(0, mSceneRect.width());
    mUi->leftVerticalSlider->setRange(0, mSceneRect.height());
    mUi->rightVertSlider->setRange(0, mSceneRect.height());
    mUi->leftSlider->setValue(0);
    mUi->rightSlider->setValue(mSceneRect.width());
    mUi->leftVerticalSlider->setValue(0);
    mUi->rightVertSlider->setValue(mSceneRect.height());
    mUi->leftVerticalSlider->setInvertedControls(false);
    mUi->rightVertSlider->setInvertedControls(false);
    //	mUi->leftSlider->setValue(mUi->leftSlider->minimum());
    //	mUi->rightSlider->setValue(mUi->rightSlider->maximum());

    // Puntos fijos + invisible
    ControlPoint* leftPoint = new ControlPoint(mSceneRect.left() + mControlPointSize, mSceneRect.bottom() - mControlPointSize, mControlPointSize, mControlPointSize, this, nullptr, "pL");
    ControlPoint* rightPoint = new ControlPoint(mSceneRect.right() - mControlPointSize, mControlPointSize, mControlPointSize, mControlPointSize, this, nullptr, "pR");
    ControlPoint* invisible = new ControlPoint(mSceneRect.center().x(), mSceneRect.center().y(), mControlPointSize, mControlPointSize, this, nullptr, "pI");
    mControlPoints.push_front(new QPointF(invisible->xPos, invisible->yPos));
    mControlPoints.push_front(new QPointF(leftPoint->xPos, leftPoint->yPos));
    mControlPoints.push_back(new QPointF(rightPoint->xPos, rightPoint->yPos));

    emit on_leftSlider_valueChanged(mUi->leftSlider->value());
    emit on_rightSlider_valueChanged(mUi->rightSlider->value());
    emit on_leftVerticalSlider_valueChanged(mUi->leftSlider->value());
    emit on_rightVertSlider_valueChanged(mUi->rightVertSlider->value());

    loadImageInView();
    loadHistogramInView();
    updateView();

    mUi->graphicsView->update();
}

void SimulatorUi::fillKnotVector()
{
    int middleKnotNumber = mPointsNumber - 4;
    mKnotVector.clear();
    for (int counter = 0; counter < 4; ++counter)
        mKnotVector.push_back(0.0);
    for (int counter = 1; counter <= middleKnotNumber; ++counter)
        mKnotVector.push_back(1.0 / (middleKnotNumber + 1) * counter);
    for (int counter = 0; counter < 4; ++counter)
        mKnotVector.push_back(1.0);
}

void SimulatorUi::on_saveImagesButton_released()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Seleccionar Carpeta"), QDir::homePath());

    if (!directory.isEmpty())
    {
        QDir dir = directory;
        if (!dir.isEmpty() && !mOriginal.empty() && !mModified.empty())
        {
            QString originalPath = QDir(dir).filePath("SimulatorUi_mOriginal.png");
            QString modifiedPath = QDir(dir).filePath("SimulatorUi_mModified.png");

            cv::imwrite(originalPath.toStdString(), mOriginal);
            cv::imwrite(modifiedPath.toStdString(), mModified);
        }
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Images saved");
    msgBox.setText("Before and after images saved in " + directory);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

ControlPoint::ControlPoint(qreal x, qreal y, qreal width, qreal height, SimulatorUi* SimulatorUi, QGraphicsItem* parent, QString name)
    : QGraphicsEllipseItem(x, y, width, height, parent), mSimulatorUi(SimulatorUi)
{
    this->setToolTip(name);
    xPos = x;
    yPos = y;
}

void ControlPoint::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsEllipseItem::mouseMoveEvent(event);
    const QPointF& pos = event->scenePos();
    QPointF *point = mSimulatorUi->itemToPoint[this];

    *point = pos.toPoint();
    const QList<QGraphicsItem*> &items = mSimulatorUi->mScene->items();
    for (QList<QGraphicsItem*>::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt)
    {
        if (*itemIt != this)
        {
            mSimulatorUi->mScene->removeItem(*itemIt);
            //mainWindow->itemToPoint.remove(static_cast<MovingEllipseItem*>(*itemIt));
            delete *itemIt;
        }
    }
    mSimulatorUi->updateView(point);
}

void SimulatorUi::on_leftSlider_valueChanged(int position)
{
    if (!mScene || mControlPoints.empty())
        return;

    if (mUi->leftSlider->value() > mUi->rightSlider->value())
    {
        mUi->leftSlider->setValue(mUi->rightSlider->value());
        return;
    }

    int newX = position + mUi->graphicsView->mapToScene(0, 0).x();
    newX = qMax(0 + mControlPointSize / 2, newX);


    mControlPoints.front()->setX(newX);
    mScene->clear();
    updateView();
}

void SimulatorUi::on_rightSlider_valueChanged(int position)
{
    if (!mScene || mControlPoints.empty())
        return;

    if (mUi->leftSlider->value() > mUi->rightSlider->value())
        mUi->leftSlider->setValue(mUi->rightSlider->value());

    int newX = position + mUi->graphicsView->mapToScene(0, 0).x();
    newX = qMin((int)mSceneRect.width() - mControlPointSize / 2, newX);

    mControlPoints.back()->setX(newX);
    mScene->clear();
    updateView();
}

void SimulatorUi::on_cargarButton_released()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Seleccionar Imagen"), QDir::homePath(), tr("Archivos de Imagen (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (!filePath.isEmpty())
    {
        std::string imagePath = filePath.toStdString();

        mOriginal = cv::imread(imagePath);
        cv::cvtColor(mOriginal, mOriginal, cv::COLOR_BGR2GRAY);
        mOriginal.convertTo(mOriginal, CV_8U);

        if (!mOriginal.empty())
        {
            mUi->modified->clear();
            mUi->modifiedHist->clear();
            loadImageInView();
            loadHistogramInView();
        }
        else
            qDebug() << QObject::tr("%1 - %2 - Error al leer la imagen: '%3'").arg(this->metaObject()->className()).arg(__func__).arg(filePath);
    }
}

void SimulatorUi::on_transformButton_released()
{
    if (mLUT.empty())
        return;

    //	createLUT();

    cv::LUT(mOriginal, mLUT, mModified);

    //	std::cout << std::endl << std::endl << "on_transformButton_released - mLUT: " << std::endl ;
    //	for (unsigned int i = 0; i < 256; ++i)
    //		std::cout << i << ":" << QString::number(mLUT.at<uchar>(i)).toStdString() << "  ";

    QImage modifiedQImage(mModified.data, mModified.cols, mModified.rows, static_cast<int>(mModified.step), QImage::Format_Grayscale8);
    mUi->modified->setPixmap(QPixmap::fromImage(modifiedQImage));

    QImage modifiedHistogram = calculateHistogram(mModified);
    mUi->modifiedHist->setPixmap(QPixmap::fromImage(modifiedHistogram));
}

void SimulatorUi::on_YButton_released()
{
    bool ok;
    uchar x = static_cast<uchar>(mUi->comboBox->currentText().toInt());
    uchar y = mUi->lineEdit->text().toInt(&ok);
    if (ok && y >= 0 && y <= 255)
        mLUT.at<uchar>(x, 0) = static_cast<uchar>(y);

    qDebug() << QObject::tr("%1 - %2 - Entrada %3 cambiada a %4").arg(this->metaObject()->className()).arg(__func__).arg(x).arg(y);

    //	std::cout << std::endl << std::endl << "on_YButton_released - mLUT: " << std::endl ;
    //	for (unsigned int i = 0; i < 256; ++i)
    //		std::cout << i << ":" << QString::number(mLUT.at<uchar>(i)).toStdString() << "  ";
}

void SimulatorUi::on_leftVerticalSlider_valueChanged(int position)
{
    if (!mScene || mControlPoints.empty())
        return;

    int newY = ((int)mSceneRect.height() - position) + mUi->graphicsView->mapToScene(0, 0).y();
    newY = qMax(0 + mControlPointSize / 2, newY);

    mControlPoints.front()->setY(newY);
    mScene->clear();
    updateView();
}

void SimulatorUi::on_rightVertSlider_valueChanged(int position)
{
    if (!mScene || mControlPoints.empty())
        return;

    int newY = ((int)mSceneRect.height() - position) + mUi->graphicsView->mapToScene(0, 0).y();
    newY = qMin((int)mSceneRect.height() - mControlPointSize / 2, newY);

    mControlPoints.back()->setY(newY);
    mScene->clear();
    updateView();
}

void SimulatorUi::on_comboBox_activated(const QString& arg1)
{
    uchar x = static_cast<uchar>(arg1.toInt());
    uchar y = mLUT.at<uchar>(x, 0);
    mUi->lineEdit->setText(QString::number(y));
}

void SimulatorUi::on_helpButton_released()
{
    if (!mHelpDialog)
    {
        QString helpMessage;
        QFile file(":/configsHS/configsHS/help.html");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            helpMessage += stream.readAll();
            file.close();
        }
        else
            qDebug() << QObject::tr("%1 - %2 - Error al abrir la hoja de ayuda.").arg(this->metaObject()->className()).arg(__func__ );

        mHelpDialog = new ScrollableMessageBox(helpMessage, this);
        mHelpDialog->hide();
    }

    if (mHelpDialog->isHidden())
    {
        QScreen* currentScreen = nullptr;

        QPoint widgetGlobalPos = mHelpDialog->mapToGlobal(QPoint(0, 0));

        for (QScreen* screen : QGuiApplication::screens())
        {
            if (screen->geometry().contains(widgetGlobalPos))
                currentScreen = screen;
        }

        QRect availableGeometry = currentScreen->availableGeometry();
        mHelpDialog->show();
        mHelpDialog->move(availableGeometry.center() - QPoint(mHelpDialog->width() / 2, mHelpDialog->height() / 2));
    }
}

void SimulatorUi::on_createLUTControl_released()
{
    if (!mScene || mControlPoints.empty())
        return;

    createLUT();
    qDebug() << QObject::tr("%1 - %2 - LUT actualizada con los puntos de control.").arg(this->metaObject()->className()).arg(__func__ );

    // std::cout << std::endl << std::endl << "on_createLUTControl_released - mLUT: " << std::endl ;
    // for (unsigned int i = 0; i < 256; ++i)
    //     std::cout << i << ":" << QString::number(mLUT.at<uchar>(i)).toStdString() << "  ";

    QMessageBox msgBox;
    msgBox.setWindowTitle("LUT created");
    msgBox.setText("The graylevel transformation is ready");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void SimulatorUi::on_saveCurveButton_released()
{
    QJsonArray jsonArray;

    for (int i = 0; i < mControlPoints.size(); ++i)
    {
        QPointF* point = mControlPoints[i];
        QJsonObject pointObject;
        pointObject["x"] = point->x();
        pointObject["y"] = point->y();
        jsonArray.append(pointObject);
    }

    QJsonObject json_o;
    json_o["controlPoints"] = jsonArray;

    const QJsonDocument json_d(json_o);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Guardar archivo JSON"), "", tr("Archivos JSON (*.json)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName + ".json");
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(json_d.toJson());
            file.close();
        }
        else
            qDebug() << QObject::tr("%1 - %2 - No se pudo abrir el archivo JSON para escribir.").arg(this->metaObject()->className()).arg(__func__);
    }
}

void SimulatorUi::on_loadCurveButton_released()
{
    if (!mScene)
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Seleccionar archivo JSON"), "", tr("Archivos JSON (*.json)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << QObject::tr("%1 - %2 - No se pudo abrir el archivo JSON para lectura.").arg(this->metaObject()->className()).arg(__func__);
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    if (jsonDoc.isNull())
    {
        qDebug() << QObject::tr("%1 - %2 - Error al analizar el archivo JSON: %3").arg(this->metaObject()->className()).arg(__func__).arg(parseError.errorString());
        return;
    }

    for (QPointF* point : mControlPoints)
        delete point;
    mControlPoints.clear();

    QJsonArray controlPointsArray = jsonDoc.object()["controlPoints"].toArray();

    for (int i = 0; i < controlPointsArray.size(); ++i)
    {
        QJsonObject pointObject = controlPointsArray[i].toObject();
        qreal x = pointObject["x"].toDouble();
        qreal y = pointObject["y"].toDouble();
        mControlPoints.push_back(new QPointF(x, y));
    }

    if (!mControlPoints.isEmpty())
    {
        QPointF* firstPoint = mControlPoints.first();
        QPointF* lastPoint = mControlPoints.last();
        int firstPointX = qRound(firstPoint->x());
        int lastPointX = qRound(lastPoint->x());
        int firstPointY = qRound(mSceneRect.height() - firstPoint->y());
        int lastPointY = qRound(mSceneRect.height() - lastPoint->y());
        mUi->leftSlider->setValue(firstPointX);
        mUi->rightSlider->setValue(lastPointX);
        mUi->leftVerticalSlider->setValue(firstPointY);
        mUi->rightVertSlider->setValue(lastPointY);
        emit on_leftSlider_valueChanged(firstPointX);
        emit on_rightSlider_valueChanged(lastPointX);
        emit on_leftVerticalSlider_valueChanged(firstPointY);
        emit on_rightVertSlider_valueChanged(lastPointY);
    }

    mScene->clear();
    updateView();
}

cv::Mat SimulatorUi::parseJSONQStringToMat(QString& json)
{
    cv::Mat customLUT = cv::Mat(256, 1, CV_8U);;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.toUtf8());

    if (!jsonDoc.isObject())
    {
        qDebug() << QObject::tr("%1 - %2 - Error: El JSON de LUT no es un objeto").arg(this->metaObject()->className()).arg(__func__);
        uchar* p = customLUT.ptr();
        for (unsigned int i = 0; i < 256; ++i) // Inicialmente la LUT es una diagonal
            p[i] = i;
    }
    else
    {
        QJsonObject jsonObj = jsonDoc.object();
        QJsonArray jsonArray = jsonObj.value("dataLUT").toArray();

        if (jsonArray.size() != 256)
        {
            qDebug() << QObject::tr("%1 - %2 - Error: El tamaño del array de LUT no es 256").arg(this->metaObject()->className()).arg(__func__);
            uchar* p = customLUT.ptr();
            for (unsigned int i = 0; i < 256; ++i) // Inicialmente la LUT es una diagonal
                p[i] = i;
        }
        else
        {
            for (int i = 0; i < 256; ++i)
                customLUT.at<uchar>(i) = static_cast<uchar>(jsonArray.at(i).toObject().value("y").toInt());
        }
    }

    //			std::cout << std::endl << std::endl << "EntityEqualizer - customLUT: " << std::endl ;
    //			for (unsigned int i = 0; i < 256; ++i)
    //				std::cout << i << ":" << QString::number(customLUT.at<uchar>(i)).toStdString() << "  ";

    return customLUT;
}

