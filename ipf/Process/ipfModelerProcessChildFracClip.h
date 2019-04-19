#ifndef IPFMODELERPROCESSCHILDFRACCLIP_H
#define IPFMODELERPROCESSCHILDFRACCLIP_H

#include "head.h"
#include "ipfModelerProcessBase.h"
#include "qgspointxy.h"

class ipfModelerFracClipDialog;

class ipfModelerProcessChildFracClip : public ipfModelerProcessBase
{
public:
	ipfModelerProcessChildFracClip(QObject *parent, const QString modelerName);
	~ipfModelerProcessChildFracClip();

	ipfModelerProcessChildFracClip* classType() { return this; };

	bool checkParameter();
	void setParameter();
	QMap<QString, QString> getParameter();
	void setDialogParameter(QMap<QString, QString> map);

	void run();

private:
	// 读取图号文件到thList中
	bool readTh();

private:
	ipfModelerFracClipDialog * clip;

	QStringList thList;
	QString fileName;
	QString dateType;
};

#endif // IPFMODELERPROCESSCHILDFRACCLIP_H