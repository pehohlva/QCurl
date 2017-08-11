#ifndef QCURL_GLOBAL_H
#define QCURL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QCURL_LIBRARY)
#  define QCURLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QCURLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QCURL_GLOBAL_H
