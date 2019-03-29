#ifndef PLAYSDK_GLOBAL_H
#define PLAYSDK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PLAYSDK_LIBRARY)
#  define PLAYSDKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PLAYSDKSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PLAYSDK_GLOBAL_H
