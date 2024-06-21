/*
file:lpPeriodicBackup.h
date:2024/6/17
brief:定期备份关键信息模块
author:wuchaoxi
copyright:杭州利珀科技有限公司
*/

#pragma once

#include "lpperiodicbackup_global.h"
#include <QTimer>
class  lpPeriodicBackup :public lpPeriodicBackupBase
{
	Q_OBJECT
public:
	explicit lpPeriodicBackup(QObject *parent = nullptr);
	virtual ~lpPeriodicBackup();

	void startBackup()override;
	void backupNow()override;

private:
	void loadConfig();
	bool copyRecursively(const QString &srcPath, const QString &dstPath);
	bool createBackup(const QString &backupPath);
	bool compressDirectory(const QString &directoryPath, const QString &zipFilePath);
	//移除图片
	bool removeImagesFromDir(const QString &backupPath);

	QTimer*			m_backupTimer;
	QStringList		m_importantPaths;
	int				m_backupIntervalHours;
	bool			m_backupOnStartup;
	QString			m_backupBasePath;
};
