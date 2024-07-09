/*
file:lpPeriodicBackup.h
date:2024/6/17
brief:���ڱ��ݹؼ���Ϣģ��
author:wuchaoxi
copyright:��������Ƽ����޹�˾
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
private slots:
	void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
	void loadConfig();
	void backupNow()override;
	void initBackup()override;
	bool copyRecursively(const QString &srcPath, const QString &dstPath);
	bool createBackup(const QString &backupPath);
	bool compressDirectory(const QString &directoryPath, const QString &zipFilePath);
	//�Ƴ�ͼƬ
	bool removeImagesFromDir(const QString &backupPath);
	void startBackup();
	void thrdStart();
	void thrdStop();
	void init();

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
