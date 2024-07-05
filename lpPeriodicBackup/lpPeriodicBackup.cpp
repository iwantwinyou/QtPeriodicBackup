#include "lpPeriodicBackup.h"
#include <QFile>
#include <QJsonDocument>
#include<QJsonObject>
#include <QVariant>
#include <QProcess>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include "QtGui/QGuiApplication"
#include <Windows.h>
#include <Shobjidl.h>
#include <Shlwapi.h>
#include <iostream>

#pragma comment(lib, "Shlwapi.lib")
#pragma execution_character_set("utf-8")
QSharedPointer<lpPeriodicBackupBase>lpCreatePeriodicBackup()
{
	return QSharedPointer<lpPeriodicBackupBase>(new lpPeriodicBackup());
}
lpPeriodicBackup::lpPeriodicBackup(QObject *parent)
	:lpPeriodicBackupBase(parent)
{
	loadConfig();
	m_backupTimer = new QTimer(this);
	connect(m_backupTimer, &QTimer::timeout, this, &lpPeriodicBackup::backupNow);
	m_process = new QProcess(this);
	connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		this, &lpPeriodicBackup::onProcessFinished);
}

lpPeriodicBackup::~lpPeriodicBackup()
{

}

void lpPeriodicBackup::startBackup()
{
	qDebug() << "startBackup:" << __FUNCTION__;
	//如果目标文件夹为空则不需要备份
	if (m_importantPaths.isEmpty())
		return;
	if (m_backupOnStartup)
	{
		backupNow();
	}
	m_backupTimer->start(m_backupIntervalHours*1000*60*60);
	qDebug() << "backupOnStartup:  "<<m_backupOnStartup << " backupIntervalHours: "<<m_backupIntervalHours << __FUNCTION__;
}

void lpPeriodicBackup::backupNow()
{
	QString timeTemp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
	QString backupFolderPath = m_backupBasePath + "/" + timeTemp;
	QString zipFilePath = m_backupBasePath + "/" + timeTemp + ".zip";
	if (createBackup(backupFolderPath))
	{
		if (compressDirectory(backupFolderPath, zipFilePath))
		{
			qDebug() << "Backup and compression successful!! --zipFilePath:" << zipFilePath << __FUNCTION__;
			// 删除临时备份文件夹
			m_currentBackupFolderPath = backupFolderPath; // 保存当前备份文件夹路径
		}
		else
		{
			qDebug() << "Compression failed!! --backupFolderPath:" << backupFolderPath << __FUNCTION__;
			// 如果压缩失败，可以选择删除临时备份文件夹
			QDir(backupFolderPath).removeRecursively();
		}
	}
	else
	{
		qDebug() << "Backup creation failed!! --backupFolderPath:" << backupFolderPath << __FUNCTION__;
	}

}

void lpPeriodicBackup::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitStatus == QProcess::NormalExit && exitCode == 0)
	{
		qDebug() << "zip successfully!!" << __FUNCTION__;
		// 压缩成功后删除临时备份文件夹
		if (!m_currentBackupFolderPath.isEmpty())
		{
			QDir(m_currentBackupFolderPath).removeRecursively();
			qDebug() << "Temporary backup folder deleted:" << m_currentBackupFolderPath << __FUNCTION__;
			m_currentBackupFolderPath.clear(); // 清空当前备份文件夹路径
		}
	}
	else
	{
		qDebug() << "Failed to execute zip command";
		qDebug() << "Error output: " << m_process->errorString();
	}
}

void lpPeriodicBackup::loadConfig()
{
	QString configFilePath = qApp->applicationDirPath() + "/lpIPUConfig/backup_config.json";
	QFile configFile(configFilePath);
	if (configFile.open(QIODevice::ReadOnly))
	{
		QByteArray configData = configFile.readAll();
		QJsonDocument configDoc = QJsonDocument::fromJson(configData);
		QJsonObject configObj = configDoc.object();

		QStringList relativePaths = configObj["important_paths"].toVariant().toStringList();
		QSet<QString>uniquePaths;
		foreach(const QString &relativePath, relativePaths)
		{
			QString absolutePath = qApp->applicationDirPath()+ relativePath;
			if (!uniquePaths.contains(absolutePath))
			{
				uniquePaths.insert(absolutePath);
				m_importantPaths.append(absolutePath);
			}
			
		}
		m_backupIntervalHours = configObj["backup_interval_hours"].toInt();
		m_backupOnStartup = configObj["backup_on_startup"].toBool();
		QString backupBasePath = configObj["backup_path"].toString();
		m_backupBasePath = qApp->applicationDirPath()+backupBasePath;
	}
}

bool lpPeriodicBackup::copyRecursively(const QString &srcPath, const QString &dstPath)
{
	QDir srcDir(srcPath);
	if (!srcDir.exists())
	{
		return false;
	}

	QDir dstDir(dstPath);
	if (!dstDir.exists()&&!dstDir.mkdir("."))
	{
		return false;
	}

	foreach(const QFileInfo &fileInfo, srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries))
	{
		QString srcFilePath = fileInfo.absoluteFilePath();
		QString dstFilePath = dstPath + "/" + fileInfo.fileName();

		if (fileInfo.isDir())
		{
			if (!copyRecursively(srcFilePath, dstFilePath))
				return false;
		}
		else
		{
			if (!QFile::copy(srcFilePath, dstFilePath))
				return false;
		}
	}
	return true;
	
}

bool lpPeriodicBackup::createBackup(const QString &backupPath)
{
	// 确保备份路径存在
	QDir backDir(QDir::cleanPath(m_backupBasePath));
	if (!backDir.exists())
	{
		if (!backDir.mkdir("."))
		{
			qDebug() << "Failed to create base backup directory:" << m_backupBasePath<<__FUNCTION__;
			return false;
		}
	}

	QString backupFolderPath = QDir::cleanPath(m_backupBasePath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
	if (!QDir().mkdir(backupFolderPath))
	{
		qDebug() << "Failed to create backup folder:" << backupFolderPath<<__FUNCTION__;
		return false;
	}
	// 复制所有重要路径到临时备份文件夹
	for (const auto& path : m_importantPaths)
	{
		QFileInfo fileInfo(path);
		QString targetPath = backupFolderPath + "/" + fileInfo.fileName(); // 使用 fileName 获取文件名或文件夹名

		if (fileInfo.isDir())
		{
			if (!QDir().mkpath(targetPath))
			{
				qDebug() << "Failed to create target directory:" << targetPath<<__FUNCTION__;
				return false;
			}
			QDir dir(path);
			foreach(const QFileInfo &file, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries))
			{
				QString srcFilePath = file.absoluteFilePath();
				QString destFilePath = targetPath + "/" + file.fileName();
				if (file.isDir())
				{
					if (!QDir().mkpath(destFilePath) || !copyRecursively(srcFilePath, destFilePath))
					{
						qDebug() << "Failed to copy directory:" << srcFilePath<<__FUNCTION__;
						return false;
					}
				}
				else
				{
					if (!QFile::copy(srcFilePath, destFilePath))
					{
						qDebug() << "Failed to copy file:" << srcFilePath<<__FUNCTION__;
						return false;
					}
				}
			}
		}
		else
		{
			if (!QFile::copy(path, targetPath))
			{
				qDebug() << "Failed to copy file:" << path<<__FUNCTION__;
				return false;
			}
		}
	}

	// 递归删除备份路径中的图片文件
	if (!removeImagesFromDir(backupFolderPath))
	{
		qDebug() << "Failed to remove images from backup folder:" << backupFolderPath << __FUNCTION__;
		return false;
	}
	return true;
}

bool lpPeriodicBackup::compressDirectory(const QString &directoryPath, const QString &zipFilePath)
{ 
	// 确保在启动新命令前停止当前运行的进程
	if (m_process->state() == QProcess::Running)
	{
		m_process->kill();
		m_process->waitForFinished();
	}
	// 检查目录是否存在
	if (!QDir(directoryPath).exists())
	{
		qDebug() << "Directory does not exist:" << directoryPath << __FUNCTION__;
		return false;
	}
	// 构建7z命令
	QString command = QString("7z a \"%1\" \"%2\"").arg(zipFilePath, directoryPath);
	m_process->start(command);
	// 异步处理，立即返回 true 表示命令已经开始执行
	if (!m_process->waitForStarted())
	{
		qDebug() << "Failed to start process:" << m_process->errorString() << __FUNCTION__;
		return false;
	}

	qDebug() << "Compression process started:" << command << __FUNCTION__;
	return true;
}

bool lpPeriodicBackup::removeImagesFromDir(const QString &backupPath)
{
	QDir dir(backupPath);
	if (!dir.exists())
	{
		qDebug() << "Directory does not exist:" << backupPath << __FUNCTION__;
		return false;
	}

	QStringList imageExtensions = QStringList() << "jpg" << "jpeg" << "png" << "gif"<<"bmp";
	QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

	int deletedCount = 0;

	for (const QFileInfo& fileInfo : list)
	{
		if (fileInfo.isDir())
		{
			// 递归删除子目录中的图片文件
			if (!removeImagesFromDir(fileInfo.absoluteFilePath()))
			{
				qDebug() << "Failed to remove images from directory:" << fileInfo.absoluteFilePath() << __FUNCTION__;
				return false;
			}
		}
		else if (imageExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive))
		{
			QString filePath = fileInfo.absoluteFilePath();
			if (QFile::remove(filePath))
			{
				qDebug() << "Deleted: " << filePath << __FUNCTION__;
				++deletedCount;
			}
			else
			{
				qDebug() << "Failed to delete:" << filePath << __FUNCTION__;
			}
		}
	}
	return true;
}
