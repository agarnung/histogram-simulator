#pragma once

#include "simulatorui.h"

#include <QObject>

class Simulator : public QObject
{
    Q_OBJECT

    public:
        explicit Simulator(QObject *parent = nullptr);

        int init();

    private:
        SimulatorUi* mSimulatorUi = nullptr;

    signals:
};
