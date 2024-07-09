库文件名: lpPeriodicBackup
功能说明: 定期备份关键信息模块封装
相关文件: lpPeriodicBackup.dll
依赖库：
无

// 代码使用示例
#include "../lpPeriodicBackup/lpperiodicbackup_global.h"
#if _DEBUG
#pragma comment	(lib, "lpPeriodicBackupd.lib")
#else
#pragma comment (lib, "lpPeriodicBackup.lib")
#endif

{
	m_periodic_backup_ptr = lpCreatePeriodicBackup();
}


V1.0.0.1 2024-06-20
 1、初版
 2、定期备份关键信息模块.
>Master软件运行后，会备份一下关键信息，关键信息路径可在配置文件配置，路径可以是多个
>支持定期备份，时间间隔单位：小时
>备份文件夹路径：x64/backup，文件夹名称为时间，压缩成.zip文件
>备份的关键信息文件夹内的图片文件会删除，不会备份到备份文件夹内

V1.0.0.2 2024-07-05
>版本2
>增加第三方库7z.dll跟7z.exe,利用第三方工具压缩文件夹

V1.0.0.3 2024-07-08
>内部线程化 避免阻塞主线程
>添加接口

V1.0.0.4 2024-07-09
>接口调整


