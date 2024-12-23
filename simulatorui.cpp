#include "simulatorui.h"
#include "ui_simulatorui.h"

SimulatorUi::SimulatorUi(QWidget* parent) :
    QWidget(parent),
    mUi(new Ui::SimulatorUi)
{
    mUi->setupUi(this);
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


