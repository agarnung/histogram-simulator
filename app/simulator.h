#pragma once

#include "simulatorui.h"

#include <QObject>

class Simulator : public QObject
{
    Q_OBJECT

    public:
        explicit Simulator(QObject *parent = nullptr);
        ~Simulator();

        int init();
        int closeSim();

    private:
        SimulatorUi* mSimulatorUi = nullptr;

    public slots:
        void on_saveJSON(const QJsonDocument& json_d);
};
