﻿#ifndef CORE_H
#define CORE_H

#include "head.h"
#include <QObject>

class QIcon;
class QWidget;

class ipfApplication : public QObject
{
	Q_OBJECT
public:
    ipfApplication(void);
    ~ipfApplication(void);

	//! 返回默认路径下的图标
	static QString getThemeIconPath( const QString &theName );

	//! 在当前目录下（包括子级）搜索指定文件夹，找到则返回路径，否则返回空
	static QString getFolder( const QString &folderPath, const QString &name );

	/** 搜索文件
	* @param path		搜索该路径及子文件夹
	* @param filters	过滤器列表
	* @return			返回搜索到的文件列表
	*/
	static QStringList searchFiles( const QString &path, QStringList &filters );

	// 计算影像数据量
	static QString dataAmount(const QString &xmlPath);

    static void setStyle(const QString &style);
};

#endif
