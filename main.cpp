#include "ImageProcessFactory.h"
#include "ipf/ipfapplication.h"
#include "qgsapplication.h"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QMessagebox>
#include <QDir>
#include <QPainter>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
	QgsApplication a(argc, argv, true);

	QPixmap pixmap;
	pixmap.load(":/image/button/SplashScreen.jpg");

	QSplashScreen screen(pixmap);
	QColor color(238, 234, 157);
	screen.setFont(QFont(QStringLiteral("΢���ź�"), 9));
	screen.show();

	screen.showMessage(QStringLiteral("��������"), Qt::AlignHCenter | Qt::AlignBottom, color);
	QThread::msleep(250);

	// [ 1 ]
	screen.showMessage(QStringLiteral("���û���"), Qt::AlignHCenter | Qt::AlignBottom, color);
	QThread::msleep(250);
	
	//! ʹ�� QSettings
	QCoreApplication::setOrganizationName("YuanLong");
	QCoreApplication::setApplicationName("IPF");

	// QGis init
	QgsApplication::setDefaultSvgPaths(QStringList(QApplication::applicationDirPath() + "/Resources/images/svg"));
	QgsApplication::setPrefixPath(QApplication::applicationDirPath(), true);
	QgsApplication::initQgis();
	// [ 1 ]

	// [ 2 ]
	screen.showMessage(QStringLiteral("���srs.db���ݿ�"), Qt::AlignHCenter | Qt::AlignBottom, color);
	QThread::msleep(250);

	if (!QFile::exists(QApplication::applicationDirPath() + "/Resources/srs.db"))
		QMessageBox::warning(0, QStringLiteral("����"), QStringLiteral("srs.dbû���ҵ���ͶӰ�任���ܽ��޷�ʹ�á�"));
	// [ 2 ]

	// [ 3 ]
	screen.showMessage(QStringLiteral("�����Դ�ļ�"), Qt::AlignHCenter | Qt::AlignBottom, color);
	QThread::msleep(250);

	QDir dirButton(QApplication::applicationDirPath() + "/Resources/button");
	if (!dirButton.exists())
		QMessageBox::warning(0, QStringLiteral("����"), QStringLiteral("��Դû���ҵ���������ʾ����������"));
	// [ 3 ]

	ImageProcessFactory w;
	w.show();
	screen.finish(&w);

	if (argc==2)
	{
		w.load(QString::fromLocal8Bit(argv[1]));
	}

	return a.exec();
}
