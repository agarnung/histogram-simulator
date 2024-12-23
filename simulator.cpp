#include "simulator.h"

Simulator::Simulator(QObject* parent)
    : QObject{parent}
{}

int Simulator::init()
{
    mSimulatorUi = new SimulatorUi();
    mSimulatorUi->show();
    mSimulatorUi->adjustSize();

    return 0;
}
