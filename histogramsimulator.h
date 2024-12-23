#pragma once

#include "histogramsimulatorui.h"

#include <QVBoxLayout>

class HistogramSimulator : public QObject
{
    Q_OBJECT

    public:
		explicit HistogramSimulator();
		virtual  ~HistogramSimulator();

        int init();
        int closeSim();

    private:
		HistogramSimulatorUI* mHistogramSimulatorUI = nullptr;

	public slots:
        void on_saveJSON(const QJsonDocument& json_d);
};
