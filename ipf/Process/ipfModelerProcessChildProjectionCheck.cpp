#include "ipfModelerProcessChildProjectionCheck.h"
#include "ipfFlowManage.h"
#include "../../ui/ipfModelerProjectionCheckDialog.h"
#include "../ipfOgr.h"

#include "QgsRasterLayer.h"
#include "qgscoordinatereferencesystem.h"

#include <QFileInfo>
#include <QProgressDialog>

ipfModelerProcessChildProjectionCheck::ipfModelerProcessChildProjectionCheck(QObject *parent, const QString modelerName)
	: ipfModelerProcessOut(parent, modelerName)
{
	setId(QUuid::createUuid().toString());
	dialog = new ipfModelerProjectionCheckDialog();
}

ipfModelerProcessChildProjectionCheck::~ipfModelerProcessChildProjectionCheck()
{
	RELEASE(dialog);
}

bool ipfModelerProcessChildProjectionCheck::checkParameter()
{
	bool isbl = true;

	if (s_srs.isEmpty())
	{
		isbl = false;
		addErrList(QStringLiteral("Դ�ο�����ϵ��Ŀ��ο�����ϵ����ȷ��"));
	}

	QDir dir = QFileInfo(saveName).dir();
	if (!dir.exists())
	{
		addErrList(QStringLiteral("��Ч�����·����"));
		isbl = false;
	}

	return isbl;
}

void ipfModelerProcessChildProjectionCheck::setParameter()
{
	if (dialog->exec())
	{
		QMap<QString, QString> map = dialog->getParameter();
		s_srs = map["s_srs"];
		saveName = map["saveName"];
	}
}

QMap<QString, QString> ipfModelerProcessChildProjectionCheck::getParameter()
{
	QMap<QString, QString> map;
	map["s_srs"] = s_srs;
	map["saveName"] = saveName;

	return map;
}

void ipfModelerProcessChildProjectionCheck::setDialogParameter(QMap<QString, QString> map)
{
	dialog->setParameter(map);

	s_srs = map["s_srs"];
	saveName = map["saveName"];
}

void ipfModelerProcessChildProjectionCheck::run()
{
	clearOutFiles();
	clearErrList();
	QStringList outList;

	//������
	int prCount = 0;
	QProgressDialog dialog(QStringLiteral("դ������ͶӰ���..."), QStringLiteral("ȡ��"), 0, filesIn().size(), nullptr);
	dialog.setWindowTitle(QStringLiteral("դ������ͶӰ���"));
	dialog.setWindowModality(Qt::WindowModal);
	dialog.show();

	foreach(QString var, filesIn())
	{
		dialog.setValue(++prCount);
		QApplication::processEvents();
		if (dialog.wasCanceled())
			return;

		// ��QGis��դ��
		QFileInfo info(var);
		QString layerName = info.baseName();
		QgsRasterLayer* layer = new QgsRasterLayer(var, layerName, "gdal");
		if (!layer || !layer->isValid())
		{
			addErrList(var + QStringLiteral(": դ�����ݶ�ȡʧ�ܡ�"));
			continue;
		}
		QgsCoordinateReferenceSystem layerCrs = layer->crs(); 
		QString crsId = layerCrs.authid();
		RELEASE(layer);

		if (s_srs != crsId)
		{
			outList << var + QStringLiteral(": ͶӰ��Ϣ��һ�¡�");
			outList << QStringLiteral("\tӰ��: ") + crsId;
			outList << QStringLiteral("\t��׼: ") + s_srs;
		}
		else
		{
			outList << var + QStringLiteral(": ͶӰ��ȷ��");
		}
	}

	// �������
	printErrToFile(saveName, outList);
}
