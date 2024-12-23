// #include "histogramsimulator.h"
#include "simulator.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // HistogramSimulator sim;
    // sim.init();

    Simulator sim;
    sim.init();

    return a.exec();
}
