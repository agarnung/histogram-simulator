#include "histogramsimulator.h"

HistogramSimulator::HistogramSimulator()
{
	qRegisterMetaType<QJsonDocument>("QJsonDocument");
}

HistogramSimulator::~HistogramSimulator()
{
}

int HistogramSimulator::init()
{
	mHistogramSimulatorUI = new HistogramSimulatorUI();
    QObject::connect(mHistogramSimulatorUI, &HistogramSimulatorUI::sg_saveJSON, this, &HistogramSimulator::on_saveJSON, Qt::DirectConnection);
	mHistogramSimulatorUI->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	mHistogramSimulatorUI->show();
	mHistogramSimulatorUI->adjustSize();

    qDebug() << QObject::tr("%1 - %2 - Simulator created").arg(this->metaObject()->className()).arg(__func__);

    return 0;
}

int HistogramSimulator::closeSim()
{
	if (mHistogramSimulatorUI)
    {
		delete mHistogramSimulatorUI;
		mHistogramSimulatorUI = nullptr;
    }

	return 0;
}

void HistogramSimulator::on_saveJSON(const QJsonDocument& json_d)
{
	QString cadena = json_d.toJson();
	cadena = cadena.replace("\n", "");
    printf("\nWriting JSON: %s\n", QJsonDocument(json_d).toJson().toStdString().c_str());fflush(0);

    // Escribir en archivo local
    //...
}


