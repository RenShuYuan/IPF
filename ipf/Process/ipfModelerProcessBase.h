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

	// 模块名称
	QString name() { return modelerName; };
	void setId(const QString mId) { this->mId = mId; };
	QString id() { return mId; };

	// 从子类对应的dialog中获得参数
	virtual void setParameter();

	// 检查参数
	virtual bool checkParameter();
	
	// 返回参数名称与值
	virtual QMap<QString, QString> getParameter() { return QMap<QString, QString>(); };

	// 反向传递参数给dialog，用于dialog初始化
	virtual void setDialogParameter(QMap<QString, QString> map) {};

	// 模块间文件传递，输入输出
	void setInFiles(const QStringList files);
	void setOutFiles(const QStringList files);
	void appendOutFile(const QString file);
	QStringList& filesIn() { return mFilesIn; };
	QStringList& filesOut() { return mFilesOut; };
	void clearOutFiles() { mFilesOut.clear(); };

	// 错误列表管理
	QStringList getErrList() { return errList; };
	void addErrList(const QString err) { errList.append(err); };
	void clearErrList() { errList.clear(); };

	/* 分隔符管理
	 * name: 输入一个完整路径的文件
	 *，处理后将移除路径及扩展名
	 *，将只返回文件名称本身
	*/
	QString removeDelimiter(const QString file);

	// 执行算法
	virtual void run();

	// 返回自身类型
	virtual ipfModelerProcessBase* classType() { return this; };
	virtual const QString typeName() { return QString(); };

private:
	/*模块名称				*/ QString modelerName;
	/*模块UUID				*/ QString mId;
	/*待处理的文件			*/ QStringList mFilesIn;
	/*已处理后的文件			*/ QStringList mFilesOut;
	/*错误列表				*/ QStringList errList;
};

#endif // IPFMODELERPROCESSBASE_H