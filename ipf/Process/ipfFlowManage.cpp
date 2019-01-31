#include "ipfFlowManage.h"
#include "ipf/Graphics/ipfModelerGraphicItem.h"
#include "ipf/Graphics/ipfModelerArrowItem.h"
#include "ipfModelerProcessBase.h"
#include "ipfModelerProcessIn.h"
#include "ipfModelerProcessOut.h"
#include "head.h"

#include <QUuid>
#include <QGraphicsScene>
#include <QDomDocument>
#include <QFileDialog>

ipfFlowManage *ipfFlowManage::smInstance = nullptr;

ipfFlowManage::ipfFlowManage(QObject *parent)
	: QObject(parent)
	, isCheck(false)
{
	smInstance = this;
}

ipfFlowManage::~ipfFlowManage()
{
}

void ipfFlowManage::new_()
{
	if (!branchManagement.isEmpty())
	{
		if (!(branchManagement.at(0))->isEmpty())
		{
			ipfModelerGraphicItem *item = (branchManagement.at(0))->at(0);
			deleteTreeBranch(item);
			item->removeSelf();
		}
	}
	branchManagement.clear();
	isCheck = false;
}

void ipfFlowManage::save()
{
	QSettings mSettings;
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(0, QStringLiteral("����ģ���ļ�")
		, path + QStringLiteral("/modeler.ipfModeler")
		, QStringLiteral("ģ���ļ� (*.ipfModeler)"));
	if (saveName.isEmpty())
		return;

	QFile file(saveName);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
	QDomDocument doc;

	//д��xmlͷ��  
	QDomProcessingInstruction instruction;
	instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	//��Ӹ��ڵ�
	QDomElement root = doc.createElement("ipfModeler");
	doc.appendChild(root);

	int index = 0;
	foreach(QList<ipfModelerGraphicItem*> *items, branchManagement)
	{
		// ��ӷ�֧
		QDomElement branch = doc.createElement("branch");
		branch.setAttribute("id", ++index);
		for (int i = 0; i < items->size(); ++i)
		{
			// ����ģ��
			ipfModelerGraphicItem* item = items->at(i);
			QDomElement title = doc.createElement(item->label());
			title.setAttribute("ID", item->modelerProcess()->id());
			title.setAttribute("y", item->pos().y());
			title.setAttribute("x", item->pos().x());
			branch.appendChild(title);

			// ����ģ������
			QMap<QString, QString> map = item->modelerProcess()->getParameter();
			QMapIterator<QString, QString> it(map);
			while (it.hasNext())
			{
				QString key = it.next().key();
				QString value = it.value();

				QDomElement author = doc.createElement(key);
				QDomText text = doc.createTextNode(value);
				author.appendChild(text);
				title.appendChild(author);
			}
		}
		root.appendChild(branch);
	}

	//������ļ�  
	QTextStream out_stream(&file);
	doc.save(out_stream, 4); //����4��  
	file.close();
}

void ipfFlowManage::run()
{
	if (!isCheck) return;

	// �����ʶ
	foreach (QList<ipfModelerGraphicItem*> *items, branchManagement)
	{
		for (int i = 0; i < items->size(); ++i)
		{
			ipfModelerGraphicItem *item = nullptr;
			ipfModelerProcessBase *process = nullptr;

			item = items->at(i);
			item->isOkButton(false);
			item->isErrButton(false);
		}
	}

	foreach(QList<ipfModelerGraphicItem*> *items, branchManagement)
	{
		for (int i = 0; i < items->size(); ++i)
		{
			ipfModelerGraphicItem *previousItem = nullptr;
			ipfModelerGraphicItem *nextItem = nullptr;
			ipfModelerProcessBase *previousProcess = nullptr;
			ipfModelerProcessBase *nextProcess = nullptr;
			if (i + 1 < items->size())
			{
				previousItem = items->at(i);
				nextItem = items->at(i + 1);

				previousProcess = previousItem->modelerProcess();
				nextProcess = nextItem->modelerProcess();

				// �����ϸ�ģ�������ļ�
				nextProcess->setInFiles(previousProcess->filesOut());

				// ����
				nextProcess->run();
				QStringList errList = nextProcess->getErrList();
				if (errList.isEmpty())
					nextItem->isOkButton(true);
				else
				{
					nextItem->isErrButton(true);
					return;
				}
			}
		}
	}
}

void ipfFlowManage::check()
{
	isCheck = true;

	// ÿ��ģ������Ƿ���ȷ
	for (int j = 0; j < branchManagement.size(); ++j)
	{
		QList<ipfModelerGraphicItem*> *items = branchManagement.at(j);

		for (int i = 0; i < items->size(); ++i)
		{
			ipfModelerGraphicItem *item = nullptr;
			ipfModelerProcessBase *process = nullptr;

			item = items->at(i);
			item->isOkButton(false);

			process = item->modelerProcess();

			// ����ģ�����
			bool is = process->checkParameter();
			if (is)
				item->isErrButton(false);
			else
			{
				item->isErrButton(true);
				isCheck = false;
			}

			// ����ֻ�ܳ��������ߵ�һ��ģ��
			if (i!=0)
			{
				if (process->name() == MODELER_IN)
				{
					item->isErrButton(true);
					isCheck = false;
				}
			}
		}
	}

	// ÿ��֧�����һ������ʱ���ģ��
	foreach(QList<ipfModelerGraphicItem*> *items, branchManagement)
	{
		ipfModelerGraphicItem *item = items->last();
		if (!(item->modelerProcess()->name() == MODELER_OUT
			|| item->modelerProcess()->name() == MODELER_EXCEL_METADATA
			|| item->modelerProcess()->name() == MODELER_TFW
			|| item->modelerProcess()->name() == MODELER_BUILDOVERVIEWS
			|| item->modelerProcess()->name() == MODELER_FRACDIFFERCHECK
			|| item->modelerProcess()->name() == MODELER_FRACEXTENTCHECK
			|| item->modelerProcess()->name() == MODELER_WATERFLATTENCHECK
			|| item->modelerProcess()->name() == MODELER_PROJECTIONCHECK
			|| item->modelerProcess()->name() == MODELER_ZCHECK
			|| item->modelerProcess()->name() == MODELER_INVALIDVALUECHECK
			|| item->modelerProcess()->name() == MODELER_DEMGROSSERRORCHECK
			|| item->modelerProcess()->name() == MODELER_RASTERINFOPRINT
			|| item->modelerProcess()->name() == MODELER_VEGETATION_EXTRACTION
			|| item->modelerProcess()->name() == MODELER_EXTRACT_RASTER_RANGE
			|| item->modelerProcess()->name() == MODELER_WATERS_EXTRACTION))
		{
			item->isErrButton(true);
			isCheck = false;
		}
	}
}

QString ipfFlowManage::getTempVrtFile(const QString & file)
{
	QFileInfo info(file);
	QString fileName = info.baseName();
	QStringList list = fileName.split(NAME_DELIMITER);
	if (list.isEmpty())
		return QString();
	else
		fileName = list.at(0);

	QString id = QUuid::createUuid().toString();
	id.remove('{').remove('}').remove('-');
	fileName = fileName + NAME_DELIMITER + id + QStringLiteral(".vrt");

	return tempDir.filePath(fileName);
}

QString ipfFlowManage::getTempFormatFile(const QString & file, const QString & format)
{
	QFileInfo info(file);
	QString fileName = info.baseName();
	QStringList list = fileName.split(NAME_DELIMITER);
	if (list.isEmpty())
		return QString();
	else
		fileName = list.at(0);

	QString id = QUuid::createUuid().toString();
	id.remove('{').remove('}').remove('-');
	fileName = fileName + NAME_DELIMITER + id + format;

	return tempDir.filePath(fileName);
}

void ipfFlowManage::appendItem(ipfModelerGraphicItem * item)
{
	QList<ipfModelerGraphicItem*> *items = nullptr;
	if (branchManagement.isEmpty())
	{
		items = new QList<ipfModelerGraphicItem *>;
		branchManagement.append(items);
	}
	else
		items = branchManagement.at(0);

	if (!items->isEmpty())
	{
		ipfModelerGraphicItem *sourceItem = items->last();
		ipfModelerArrowItem *arrow = new ipfModelerArrowItem(sourceItem, -1, item, -1);
		sourceItem->addArrow(arrow);
		item->addArrow(arrow);
		arrow->updatePath();
		item->scene()->addItem(arrow);
	}
	items->append(item);

	printBranch();
}

void ipfFlowManage::appendBranchItem(ipfModelerGraphicItem * prvItem, ipfModelerGraphicItem * item)
{
	// ����һ���·�֧, ���ϸ�ģ���뵱ǰģ����ӵ��·�֧��
	QList<ipfModelerGraphicItem*> *items = new QList<ipfModelerGraphicItem *>;
	branchManagement.append(items);
	items->append(prvItem);
	items->append(item);

	// ������ͷ����
	ipfModelerArrowItem *arrow = new ipfModelerArrowItem(prvItem, -1, item, -1);
	prvItem->addArrow(arrow);
	item->addArrow(arrow);
	arrow->updatePath();
	item->scene()->addItem(arrow);

	printBranch();
}

void ipfFlowManage::deleteAllBranch(QList<ipfModelerGraphicItem*>* items)
{
	// ������һ��ͼ��(��Ϊ��һ��ͼ����������һ��֧���ڹ���)
	while (items->size() > 1)
	{
		ipfModelerGraphicItem *prvItem = items->at(items->size() - 2);
		ipfModelerGraphicItem *lastItem = items->last();

		QList<ipfModelerArrowItem*> arrows = lastItem->getArrows();
		lastItem->deleteArrows(arrows); // �Ƴ����м�ͷ
		foreach(ipfModelerArrowItem *arrow, arrows)
		{
			prvItem->deleteArrow(arrow); // ���ϸ�ģ���Ƴ���ͷ
			arrow->removeSelf(); // ɾ����ͷͼ��
		}
		qDebug() << lastItem->label();
		lastItem->removeSelf();
	}

	// ɾ��֧��
	branchManagement.removeOne(items);
}

ipfModelerGraphicItem* ipfFlowManage::findModeler(const QString & id)
{
	for (int j = 0; j < branchManagement.size(); ++j)
	{
		QList<ipfModelerGraphicItem*> *items = branchManagement.at(j);
		for (int i = 0; i < items->size(); ++i)
		{
			if (items->at(i)->modelerProcess()->id() == id)
			{
				return items->at(i);
			}
		}
	}

	return nullptr;
}

bool ipfFlowManage::isEmpty()
{
	if (branchManagement.isEmpty())
		return true;
	return false;
}

void ipfFlowManage::deleteTreeBranch(ipfModelerGraphicItem * item)
{
	for(int j = 0; j < branchManagement.size(); ++j)
	{
		QList<ipfModelerGraphicItem*> *items = branchManagement.at(j);
		if (items->first() == item)
		{
			for (int i = 1; i < items->size(); ++i)
			{
				ipfModelerGraphicItem *item = items->at(i);
				deleteTreeBranch(item);
			}
			deleteAllBranch(items);
		}
	}
	printBranch();
}

void ipfFlowManage::printBranch()
{
	int count = 0;
	foreach(QList<ipfModelerGraphicItem*> *items, branchManagement)
	{
		QString str;
		foreach(ipfModelerGraphicItem *item, *items)
			str += item->label() + QStringLiteral("  ");
		qDebug() << ++count << ": " << str;
	}
	qDebug() << endl;
}

void ipfFlowManage::insrtItem(ipfModelerGraphicItem* prvItem, ipfModelerGraphicItem* item)
{
	ipfModelerGraphicItem *nextItem = nullptr;
	QList<ipfModelerGraphicItem*> *items = nullptr;
	int nextIndex = 0;

	// ƥ��prvItem��������֧
	for (int j = 0; j < branchManagement.size(); ++j)
	{
		items = branchManagement.at(j);
		nextIndex = items->indexOf(prvItem);
		if (nextIndex != -1)
			break;
	}

	if (items->last()== prvItem)
	{
		ipfModelerArrowItem *arrow = new ipfModelerArrowItem(prvItem, -1, item, -1);
		prvItem->addArrow(arrow);
		item->addArrow(arrow);
		arrow->updatePath();
		item->scene()->addItem(arrow);
		items->append(item);
	} 
	else
	{
		nextIndex += 1;
		nextItem = items->at(nextIndex);
		if (!nextItem) return;

		// ��ȡ��������ģ��Ĺ��м�ͷ
		QList<ipfModelerArrowItem*> prevArrows = prvItem->getArrows();
		QList<ipfModelerArrowItem*> nextArrows = nextItem->getArrows();
		QList<ipfModelerArrowItem*> newArrows;
		foreach(ipfModelerArrowItem *arrow, nextArrows)
		{
			if (prevArrows.contains(arrow))
				newArrows.append(arrow);
		}

		// ��prvItemָ���¸�ģ��ļ�ͷָ��item
		foreach(ipfModelerArrowItem *arrow, newArrows)
		{
			arrow->setEndItem(item, -1); // ���ϸ�ģ��ļ�ͷָ�����ģ��
			item->addArrow(arrow); // ����ͷָ����뵽����ģ��
			nextItem->deleteArrow(arrow); // ����ͷָ����¸�ģ���Ƴ�
		}

		// ������nextItem֮����¼�ͷ
		ipfModelerArrowItem *arrow = new ipfModelerArrowItem(item, -1, nextItem, -1);
		nextItem->addArrow(arrow);
		item->addArrow(arrow);
		arrow->updatePath();
		item->scene()->addItem(arrow);

		// ����item
		items->insert(nextIndex, item);
	}
	printBranch();
}

void ipfFlowManage::deleteItem(ipfModelerGraphicItem * item)
{
	if (branchManagement.isEmpty()) return;

	if (branchManagement.at(0)->first() == item)
	{
		deleteTreeBranch(item);
		if (!branchManagement.isEmpty() && !branchManagement.at(0)->isEmpty())
			deleteTreeBranch(branchManagement.at(0)->first());
	}
	else
		deleteTreeBranch(item);

	QList<ipfModelerGraphicItem*> *items = nullptr;
	int index = -1;

	for (int j = 0; j < branchManagement.size(); ++j)
	{
		// ƥ��prvItem��������֧
		items = branchManagement.at(j);
		index = items->indexOf(item);
		if (index != -1)
		{
			break;
		}
	}

	if (index == -1) return;

	ipfModelerGraphicItem *previousItem = nullptr;
	ipfModelerGraphicItem *nextItem = nullptr;

	// ���ɾ��ģ�������ģ��ָ��
	if (index == 0)
		nextItem = items->at(index+1);
	else if (item == items->last())
		previousItem = items->at(index - 1);
	else
	{
		previousItem = items->at(index - 1);
		nextItem = items->at(index + 1);
	}


	QList<ipfModelerArrowItem*> arrows = item->getArrows();

	// ɾ��ģ��λ��֧�߶�����β��
	if (!previousItem || !nextItem)
	{
		if (!previousItem) nextItem->deleteArrows(arrows);
		if (!nextItem) previousItem->deleteArrows(arrows);

		foreach(ipfModelerArrowItem *arrow, arrows)
			arrow->removeSelf();
	}
	else // ɾ��ģ��λ��֧���м�
	{
		QList<ipfModelerArrowItem*> prevArrows = previousItem->getArrows();
		QList<ipfModelerArrowItem*> nextArrows = nextItem->getArrows();
		
		QList<ipfModelerArrowItem*> newArrows;
		foreach(ipfModelerArrowItem *arrow, arrows)
		{
			if (prevArrows.contains(arrow))
				newArrows.append(arrow);
		}

		item->deleteArrows(newArrows);
		previousItem->deleteArrows(newArrows);
		
		foreach(ipfModelerArrowItem *arrow, newArrows)
			arrow->removeSelf();

		arrows = item->getArrows();
		foreach(ipfModelerArrowItem *arrow, arrows)
		{
			arrow->setStartItem(previousItem, -1);
			previousItem->addArrow(arrow);
		}
	}
	items->removeAt(index);

	// ��ֻ��һ��ģ���֧�����
	for (int i=0; i < branchManagement.size(); ++i)
	{
		// ��������������
		if (i==0) continue;

		QList<ipfModelerGraphicItem*> *items = branchManagement.at(i);
		if (items->size() == 1)
			branchManagement.removeOne(items);
	}
	printBranch();
}
