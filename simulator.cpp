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
    QString cadena = json_d.toJson();
    cadena = cadena.replace("\n", "");
    printf("\nWriting JSON: %s\n", QJsonDocument(json_d).toJson().toStdString().c_str());fflush(0);

    // Escribir en archivo local
    //...
}
