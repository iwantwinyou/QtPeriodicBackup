#pragma once

#include <QtCore/qglobal.h>
#include<QObject>
#ifndef BUILD_STATIC
# if defined(LPPERIODICBACKUP_LIB)
#  define LPPERIODICBACKUP_EXPORT Q_DECL_EXPORT
# else
#  define LPPERIODICBACKUP_EXPORT Q_DECL_IMPORT
# endif
#else
# define LPPERIODICBACKUP_EXPORT
#endif


class LPPERIODICBACKUP_EXPORT lpPeriodicBackupBase :public QObject
{
	Q_OBJECT
public:
	lpPeriodicBackupBase(QObject *parent = nullptr) :QObject(parent) {}
	virtual ~lpPeriodicBackupBase() {}

	virtual void backupNow() = 0;
	virtual void initBackup() = 0;
};
LPPERIODICBACKUP_EXPORT QSharedPointer<lpPeriodicBackupBase> lpCreatePeriodicBackup();