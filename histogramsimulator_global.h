#pragma once

#include <QtCore/qglobal.h>

#if defined(HISTOGRAMSIMULATOR_LIBRARY)
#  define HISTOGRAMSIMULATOR_EXPORT Q_DECL_EXPORT
#else
#  define HISTOGRAMSIMULATOR_EXPORT Q_DECL_IMPORT
#endif
