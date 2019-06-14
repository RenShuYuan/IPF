#ifndef IPFMODELERPROCESSBASE_H
#define IPFMODELERPROCESSBASE_H

#include "head.h"

#include <QObject>
#include <QUrl>

class ipfModelerProcessBase : public QObject
{
	Q_OBJECT

public:
	ipfModelerProcessBase(QObject *parent, const QString modelerName);
	~ipfModelerProcessBase();

	// ģ������
	QString name() { return modelerName; };
	void setId(const QString mId) { this->mId = mId; };
	QString id() { return mId; };

	// �������Ӧ��dialog�л�ò���
	virtual void setParameter();

	// ������
	virtual bool checkParameter();
	
	// ���ز���������ֵ
	virtual QMap<QString, QString> getParameter() { return QMap<QString, QString>(); };

	// ���򴫵ݲ�����dialog������dialog��ʼ��
	virtual void setDialogParameter(QMap<QString, QString> map) {};

	// ģ����ļ����ݣ��������
	void setInFiles(const QStringList files);
	void setOutFiles(const QStringList files);
	void appendOutFile(const QString file);
	QStringList& filesIn() { return mFilesIn; };
	QStringList& filesOut() { return mFilesOut; };
	void clearOutFiles() { mFilesOut.clear(); };

	// �����б����
	QStringList getErrList() { return errList; };
	void addErrList(const QString err) { errList.append(err); };
	void clearErrList() { errList.clear(); };

	/* �ָ�������
	 * name: ����һ������·�����ļ�
	 *��������Ƴ�·������չ��
	 *����ֻ�����ļ����Ʊ���
	*/
	QString removeDelimiter(const QString file);

	// ִ���㷨
	virtual void run();

	// ������������
	virtual ipfModelerProcessBase* classType() { return this; };
	virtual const QString typeName() { return QString(); };

private:
	/*ģ������				*/ QString modelerName;
	/*ģ��UUID				*/ QString mId;
	/*��������ļ�			*/ QStringList mFilesIn;
	/*�Ѵ������ļ�			*/ QStringList mFilesOut;
	/*�����б�				*/ QStringList errList;
};

#endif // IPFMODELERPROCESSBASE_H