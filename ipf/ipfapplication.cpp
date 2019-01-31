#include <QIcon>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QProgressDialog>
#include <QApplication>

#include "ipfapplication.h"

ipfApplication::ipfApplication(void)
{
}


ipfApplication::~ipfApplication(void)
{
}

QString ipfApplication::getThemeIconPath( const QString &theName )
{
    QString defaultThemePath = QApplication::applicationDirPath() + QStringLiteral("/resources/button/") + theName;

	if ( QFile::exists(defaultThemePath) )
	{
		return defaultThemePath;
	}
	else
	{
		return QString();
	}
}

QString ipfApplication::getFolder( const QString &folder, const QString &name )
{
	QDir dir(folder);

	if (!dir.exists())
	{
		return "";
	}

	QDirIterator dirIterator(folder, 
		QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, 
		QDirIterator::Subdirectories);

	while (dirIterator.hasNext())
	{
		dirIterator.next();
		QFileInfo file_info = dirIterator.fileInfo();
		qDebug() << file_info.fileName() << "::" << file_info.baseName();
		if (file_info.baseName() == name)
		{
			return file_info.filePath();
		}
	}

	return "";
}

QStringList ipfApplication::searchFiles( const QString &path, QStringList &filters )
{
	QStringList list;

	QDir dir(path);
	if (!dir.exists())
	{
		return list;
	}

	QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
	while (dir_iterator.hasNext())
	{
		dir_iterator.next();
		QFileInfo file_info = dir_iterator.fileInfo();
		list.append(file_info.filePath());
	}
    return list;
}

QString ipfApplication::dataAmount(const QString & xmlPath)
{
	//保存影像文件大小
	qint64 qint = 0;
	QString str_fileSize;
	double f_fileSize = 0.0;

	QString imgNamep = xmlPath;
	QString igeNamep = xmlPath.left(xmlPath.size() - 3) + "IGE";
	QFileInfo imginfop(imgNamep);
	QFileInfo igeinfop(igeNamep);
	if (imginfop.exists())
		qint = imginfop.size();
	if (igeinfop.exists())
		qint += igeinfop.size();

	// 单位换算
	if (qint > 1073741824 || qint < 104857600)
	{
		f_fileSize = (double)qint / 1024 / 1024;
		if (qint > 1073741824)
			str_fileSize = QString::number(f_fileSize, 'f', 2);
		else
		{
			str_fileSize = QString::number(f_fileSize, 'f', 6);
			str_fileSize = str_fileSize.left(str_fileSize.size() - 4);
		}
	}
	else
	{
		f_fileSize = qint / 1024 / 1024;
		str_fileSize = QString::number(f_fileSize, 'f', 2);
	}

	return str_fileSize;
}

void ipfApplication::setStyle(const QString &style)
{
    QFile qss(style);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}
