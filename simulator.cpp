#include "simulator.h"

Simulator::Simulator(QObject* parent)
    : QObject{parent}
{
    qRegisterMetaType<QJsonDocument>("QJsonDocument");
}

Simulator::~Simulator()
{

}

int Simulator::init()
{
    mSimulatorUi = new SimulatorUi();
    QObject::connect(mSimulatorUi, &SimulatorUi::sg_saveJSON, this, &Simulator::on_saveJSON, Qt::DirectConnection);
    mSimulatorUi->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mSimulatorUi->initializeSimulator();
    mSimulatorUi->show();
    mSimulatorUi->adjustSize();

    qDebug() << QObject::tr("%1 - %2 - Simulator created").arg(this->metaObject()->className()).arg(__func__);

    return 0;
}

int Simulator::closeSim()
{
    if (mSimulatorUi)
    {
        delete mSimulatorUi;
        mSimulatorUi = nullptr;
    }

    return 0;
}

void Simulator::on_saveJSON(const QJsonDocument& json_d)
{
    QString jsonString = json_d.toJson(QJsonDocument::Compact);
    printf("\nWriting JSON: %s\n", jsonString.toStdString().c_str());
    fflush(0);

    QString fileName = QFileDialog::getSaveFileName(
        nullptr, "Save JSON", QDir::currentPath(), "JSON Files (*.json)");

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".json", Qt::CaseInsensitive))
        fileName += ".json";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printf("Error opening file for writing.\n");
        return;
    }

    QTextStream out(&file);
    out << jsonString;
    file.close();

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("LUT saved");
    msgBox.setText("LUT successfully saved in " + fileName);
    msgBox.setStandardButtons(QMessageBox::Ok);
}
