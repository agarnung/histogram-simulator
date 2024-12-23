#include "simulator.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("HistogramSimulator");
    a.setApplicationVersion("0.0");
    a.setStyle("fusion");

    Simulator sim;
    sim.init();

    return a.exec();
}
