#ifndef CORE_H
#define CORE_H

#include "head.h"
#include <QObject>
#include <QTemporaryDir>

class QIcon;
class QWidget;

class ipfApplication : public QObject
{
	Q_OBJECT
public:
    ipfApplication(void);
    ~ipfApplication(void);

	static ipfApplication *instance() { return smInstance; }

	//! 返回默认路径下的图标
	static QString getThemeIconPath( const QString &theName );

	//! 在当前目录下（包括子级）搜索指定文件夹，找到则返回路径，否则返回空
	static QString getFolder( const QString &folderPath, const QString &name );

	// 生成vrt格式的临时文件，需要路径+文件名称+扩展名
	QString getTempVrtFile(const QString& filePath);

	// 生成指定格式的临时文件，file：完整文件路径, format：扩展名(.vrt)
	QString getTempFormatFile(const QString& filePath, const QString& format);

	/** 搜索文件
	* @param path		搜索该路径及子文件夹
	* @param filters	过滤器列表
	* @return			返回搜索到的文件列表
	*/
	static QStringList searchFiles( const QString &path, QStringList &filters );

	// 计算影像数据量
	static QString dataAmount(const QString &xmlPath);

    static void setStyle(const QString &style);

private:
	static ipfApplication *smInstance;
	QTemporaryDir tempDir1;
};

#endif
