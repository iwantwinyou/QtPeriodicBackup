/*
file:lpPeriodicBackup.h
date:2024/6/17
brief:定期备份关键信息模块
author:wuchaoxi
*/

#pragma once

#include "lpperiodicbackup_global.h"
#include <QTimer>
#include <QProcess>
#include <QThread>
class  lpPeriodicBackup :public lpPeriodicBackupBase
{
	Q_OBJECT
public:
	explicit lpPeriodicBackup(QObject *parent = nullptr);
	virtual ~lpPeriodicBackup();

	void thrdStart()override;
	void thrdStop()override;
	void init()override;
private slots:
	void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
	void loadConfig();
	bool copyRecursively(const QString &srcPath, const QString &dstPath);
	bool createBackup(const QString &backupPath);
	bool compressDirectory(const QString &directoryPath, const QString &zipFilePath);
	//移除图片
	bool removeImagesFromDir(const QString &backupPath);
	void startBackup();
	void backupNow();
private:
	QTimer*			m_backupTimer;
	QStringList		m_importantPaths;
	int				m_backupIntervalHours;
	bool			m_backupOnStartup;
	QString			m_backupBasePath;
	QProcess*		m_process;
	QString         m_currentBackupFolderPath;

	QThread*		m_this_thread_ptr{nullptr};
};
