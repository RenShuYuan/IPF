#include "ipfGraphicsScene.h"
#include "ipfModelerArrowItem.h"
#include "ipfModelerGraphicItem.h"
#include "ipf/Process/ipfModelerProcessBase.h"
#include "ipf/Process/ipfModelerProcessIn.h"
#include "ipf/Process/ipfModelerProcessOut.h"
#include "ipf/Process/ipfModelerProcessChild.h"
#include "ipf/Process/ipfModelerProcessChildTypeConvert.h"
#include "ipf/Process/ipfModelerProcessChildFracClip.h"
#include "ipf/Process/ipfModelerProcessChildQuickView.h"
#include "ipf/Process/ipfModelerProcessChildResample.h"
#include "ipf/Process/ipfModelerProcessChildMosaic.h"
#include "ipf/Process/ipfModelerProcessChildTransform.h"
#include "ipf/Process/ipfModelerProcessChildClipVector.h"
#include "ipf/Process/ipfModelerProcessChildPixelDecimals.h"
#include "ipf/Process/ipfModelerProcessChildCreateMetadata.h"
#include "ipf/Process/ipfModelerProcessChildCreateTfw.h"
#include "ipf/Process/ipfModelerProcessChildbuildOverviews.h"
#include "ipf/Process/ipfModelerProcessChildProcessRasterRectPosition.h"
#include "ipf/Process/ipfModelerProcessChildDifferenceCheck.h"
#include "ipf/Process/ipfModelerProcessChildFracExtentCheck.h"
#include "ipf/Process/ipfModelerProcessChildPixelMoidfyValue.h"
#include "ipf/Process/ipfModelerProcessChildWaterFlatten.h"
#include "ipf/Process/ipfModelerProcessChildProjectionCheck.h"
#include "ipf/Process/ipfModelerProcessChildZCheck.h"
#include "ipf/Process/ipfModelerProcessChildConsistency.h"
#include "ipf/Process/ipfModelerProcessChildSlopCalculation.h"
#include "ipf/Process/ipfModelerProcessChildInvalidValueCheck.h"
#include "ipf/Process/ipfModelerProcessChildDemGrossErrorCheck.h"
#include "ipf/Process/ipfModelerProcessChildRasterInfoPrint.h"
#include "ipf/Process/ipfModelerProcessChildVegeataionExtraction.h"
#include "ipf/Process/ipfModelerProcessChildExtractRasterRange.h"
#include "ipf/Process/ipfModelerProcessChildWatersExtraction.h"
#include "ipf/Process/ipfModelerProcessChildSetNodata.h"
#include "ipf/Process/ipfModelerProcessChildDSMDEMDifferenceCheck.h"
#include "ipf/Process/ipfModelerProcessChildDSMDEMDifferenceProcess.h"
#include "ipf/Process/ipfModelerProcessChildRangeMoidfyValue.h"
#include "ipf/Process/ipfFlowManage.h"
#include "../../ui/ipfSelectModelerDialog.h"
#include "head.h"

#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>

ipfGraphicsScene::ipfGraphicsScene(ipfFlowManage *flow, QObject *parent)
	: QGraphicsScene(parent)
	, flow(flow)
	, item(nullptr)
{
	setItemIndexMethod(QGraphicsScene::NoIndex);
	setSceneRect(QRectF(0, 0, 4000, 4000));

	// 初始化图形的右键处理菜单
	menu = new QMenu(Q_NULLPTR);
	QAction *removeAction = menu->addAction(QStringLiteral("创建分支"));
	connect(removeAction, &QAction::triggered, this, &ipfGraphicsScene::createBranch);
}

ipfGraphicsScene::~ipfGraphicsScene()
{
	RELEASE(menu);
}

void ipfGraphicsScene::addModel(const QString &itemName, QPointF pt)
{
	// 添加“输入”模块
	//for inp in list(model.parameterComponents().values()) :
	//	item = (inp, model, controls, scene = self)
	//	item.setFlag(QGraphicsItem.ItemIsMovable, True)
	//	item.setFlag(QGraphicsItem.ItemIsSelectable, True)
	//	self.addItem(item)
	//	item.setPos(inp.position().x(), inp.position().y())
	//	self.paramItems[inp.parameterName()] = item

	ipfModelerProcessBase* process = createProcessBase(itemName);
	if (!process) return;

	ipfModelerGraphicItem *item = new ipfModelerGraphicItem(process, menu);
	item->setPos(pt);
	addItem(item);
	flow->appendItem(item);

	// 输入相关箭头符号
	//for input_name in list(model.parameterComponents().keys()) :
	//	idx = 0
	//	parameter_def = model.parameterDefinition(input_name)
	//	if hasattr(parameter_def, 'parentLayerParameterName') and parameter_def.parentLayerParameterName() :
	//		parent_name = parameter_def.parentLayerParameterName()
	//		if input_name in self.paramItems and parent_name in self.paramItems :
	//			input_item = self.paramItems[input_name]
	//			parent_item = self.paramItems[parent_name]
	//			arrow = ModelerArrowItem(parent_item, -1, input_item, -1)
	//			input_item.addArrow(arrow)
	//			parent_item.addArrow(arrow)
	//			arrow.setPenStyle(Qt.DotLine)
	//			arrow.updatePath()
	//			self.addItem(arrow)

	//// 添加“算法”模块
	//for alg in list(model.childAlgorithms().values()) :
	//	item = ModelerGraphicItem(alg, model, controls, scene = self)
	//	item.setFlag(QGraphicsItem.ItemIsMovable, True)
	//	item.setFlag(QGraphicsItem.ItemIsSelectable, True)
	//	self.addItem(item)
	//	item.setPos(alg.position().x(), alg.position().y())
	//	self.algItems[alg.childId()] = item

	// 添加箭头符号
	//for alg in list(model.childAlgorithms().values()) :
	//	idx = 0
	//	for parameter in alg.algorithm().parameterDefinitions() :
	//		if not parameter.isDestination() and not parameter.flags() & QgsProcessingParameterDefinition.FlagHidden :
	//			if parameter.name() in alg.parameterSources() :
	//				sources = alg.parameterSources()[parameter.name()]
	//			else :
	//				sources = []
	//			for source in sources :
	//				sourceItems = self.getItemsFromParamValue(source, alg.childId(), context)
	//				for sourceItem, sourceIdx in sourceItems :
	//					arrow = ModelerArrowItem(sourceItem, sourceIdx, self.algItems[alg.childId()], idx)
	//					sourceItem.addArrow(arrow)
	//					self.algItems[alg.childId()].addArrow(arrow)
	//					arrow.updatePath()
	//					self.addItem(arrow)
	//				idx += 1
	//	for depend in alg.dependencies() :
	//		arrow = ModelerArrowItem(self.algItems[depend], -1, self.algItems[alg.childId()], -1)
	//		self.algItems[depend].addArrow(arrow)
	//		self.algItems[alg.childId()].addArrow(arrow)
	//		arrow.updatePath()
	//		self.addItem(arrow)

	// 最后是输出
	//for alg in list(model.childAlgorithms().values()) :
	//	outputs = alg.modelOutputs()
	//	outputItems = {}
	//	idx = 0
	//	for key, out in outputs.items() :
	//		if out is not None :
	//			item = ModelerGraphicItem(out, model, controls, scene = self)
	//			item.setFlag(QGraphicsItem.ItemIsMovable, True)
	//			item.setFlag(QGraphicsItem.ItemIsSelectable, True)
	//			self.addItem(item)
	//			pos = out.position()
	//			if pos is None :
	//				pos = (alg.position() + QPointF(ModelerGraphicItem.BOX_WIDTH, 0) +
	//						self.algItems[alg.childId()].getLinkPointForOutput(idx))
	//			item.setPos(pos)
	//			outputItems[key] = item
	//			arrow = ModelerArrowItem(self.algItems[alg.childId()], idx, item, -1)
	//			self.algItems[alg.childId()].addArrow(arrow)
	//			item.addArrow(arrow)
	//			arrow.updatePath()
	//			self.addItem(arrow)
	//			idx += 1
	//		else:
	//			outputItems[key] = None
	//	self.outputItems[alg.childId()] = outputItems
}

void ipfGraphicsScene::insrtModel(ipfModelerGraphicItem * prvItem, const QString &itemName)
{
	ipfModelerProcessBase* process = createProcessBase(itemName);
	if (!process) return;

	ipfModelerGraphicItem *item = new ipfModelerGraphicItem(process, menu);
	addItem(item);
	item->setPos(prvItem->pos() + QPointF(25, 100));

	flow->insrtItem(prvItem, item);
}

void ipfGraphicsScene::new_()
{
	if (flow->isEmpty())
		return;

	QMessageBox::StandardButton sb = QMessageBox::information(
		NULL, QStringLiteral("新建"),
		QStringLiteral("确认要清空当前流程图？"),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	if (sb != QMessageBox::Yes)
		return;

	flow->new_();
}

void ipfGraphicsScene::load()
{
	if (!flow->isEmpty())
	{
		QMessageBox::StandardButton sb = QMessageBox::information(
			NULL, QStringLiteral("加载"),
			QStringLiteral("确认是否要清空当前流程图，加载新的模型文件？"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (sb != QMessageBox::Yes)
			return;
	}

	//打开文件
	QSettings mSettings;
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString fileName = QFileDialog::getOpenFileName(0, QStringLiteral("选择模型文件"), path, QStringLiteral("模型文件 (*.ipfModeler)"));
	if (fileName.isEmpty())
		return;

	load(fileName);
}

void ipfGraphicsScene::load(const QString & fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
	{
		QMessageBox::critical(0, QStringLiteral("加载模型文件"), fileName + QStringLiteral(": 模型文件打开失败，可能已被损坏。"));
		return;
	}

	flow->new_();
	QDomDocument doc;

	QString errorStr;
	int errorLine;
	int errorColumn;
	if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
	{
		QMessageBox::critical(0, tr("Error"),
			tr("Parse error at line %1, column %2: %3")
			.arg(errorLine).arg(errorColumn).arg(errorStr));
		QMessageBox::critical(0, QStringLiteral("读取模型文件"), QStringLiteral("读取模型文件失败，内容可能已被损坏。"));
		return;
	}
	file.close();

	//返回根节点
	QDomElement root = doc.documentElement();
	if (root.nodeName() != QStringLiteral("ipfModeler"))
	{
		QMessageBox::critical(0, QStringLiteral("读取模型文件"), QStringLiteral("读取模型文件失败，内容已被损坏。"));
		return;
	}

	//获得第一个支线节点
	QDomNode branchNode = root.firstChild();

	// 如果节点不空
	while (!branchNode.isNull())
	{
		if (branchNode.isElement())
		{
			// 转换为元素
			QDomElement branchElement = branchNode.toElement();
			if (branchElement.tagName() != QStringLiteral("branch"))
			{
				QMessageBox::critical(0, QStringLiteral("读取模型文件"), QStringLiteral("读取模型文件失败，内容已被损坏。"));
				return;
			}

			// 获得支线下所有模块
			QDomNodeList modelers = branchElement.childNodes();
			for (int i = 0; i < modelers.size(); ++i)
			{
				QDomNode modeler = modelers.at(i);
				if (modeler.isElement())
				{
					double x = modeler.toElement().attribute("x").toDouble();
					double y = modeler.toElement().attribute("y").toDouble();
					QString id = modeler.toElement().attribute("ID");

					if (branchElement.attribute("id").toInt() == 1) // 主线
					{
						ipfModelerProcessBase* process = createProcessBase(modeler.nodeName());
						if (!process)
						{
							QMessageBox::critical(0, QStringLiteral("构建模块"), QStringLiteral("构建模块失败，内容已被损坏。"));
							flow->new_();
							return;
						}
						process->setId(id);
						setModelerDialogParameter(process, modeler);

						ipfModelerGraphicItem *item = new ipfModelerGraphicItem(process, menu);
						item->setPos(QPointF(x, y));
						addItem(item);
						flow->appendItem(item);
					}
					else // 支线
					{
						if (modelers.size() < 2) continue;
						if (i+1 >= modelers.size()) continue;

						ipfModelerGraphicItem *prvItem = flow->findModeler(id);
						if (!prvItem)
						{
							QMessageBox::critical(0, QStringLiteral("构建模块"), QStringLiteral("构建模块失败，内容已被损坏。"));
							flow->new_();
							return;
						}

						//
						QDomNode modelerPlus = modelers.at(i+1);
						double xPlus = modelerPlus.toElement().attribute("x").toDouble();
						double yPlus = modelerPlus.toElement().attribute("y").toDouble();
						QString idPlus = modelerPlus.toElement().attribute("ID");
						
						ipfModelerProcessBase* processPlus = createProcessBase(modelerPlus.nodeName());
						if (!processPlus)
						{
							QMessageBox::critical(0, QStringLiteral("构建模块"), QStringLiteral("构建模块失败，内容已被损坏。"));
							flow->new_();
							return;
						}
						processPlus->setId(idPlus);
						setModelerDialogParameter(processPlus, modelerPlus);

						ipfModelerGraphicItem *itemPlus = new ipfModelerGraphicItem(processPlus, menu);
						itemPlus->setPos(QPointF(xPlus, yPlus));
						addItem(itemPlus);

						if (i==0) // 创建分支
						{
							flow->appendBranchItem(prvItem, itemPlus);
						}
						else // 插入分支
						{
							flow->insrtItem(prvItem, itemPlus);
						}
					}
				}
			}
		}

		branchNode = branchNode.nextSibling();
	}
}

void ipfGraphicsScene::save()
{
	flow->save();
}

void ipfGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	if (mouseEvent->button() == Qt::RightButton)
	{
		QList<QGraphicsItem *> list = items(mouseEvent->scenePos());
		if (!list.isEmpty())
			item = list.at(0);
	}
	
	QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void ipfGraphicsScene::createBranch()
{
	if (!item) return;

	ipfModelerGraphicItem* prvItem = dynamic_cast<ipfModelerGraphicItem *>(item);
	if (!prvItem) return;

	// 创建新模块
	QString moderName;
	ipfSelectModelerDialog dialog;
	if (dialog.exec())
		moderName = dialog.getParameter();
	else
		return;

	ipfModelerProcessBase* process = createProcessBase(moderName);
	if (!process) return;

	ipfModelerGraphicItem *currItem = new ipfModelerGraphicItem(process, menu);
	currItem->setPos(prvItem->pos() + QPointF(250, 80));
	addItem(currItem);

	flow->appendBranchItem(prvItem, currItem);

	item = nullptr;
}

void ipfGraphicsScene::setModelerDialogParameter(ipfModelerProcessBase* process, QDomNode node)
{
	QMap<QString, QString> map;
	QDomNodeList parameters = node.childNodes();

	for(int i = 0; i<parameters.size(); ++i)
	{
		QDomNode parameter = parameters.at(i);
		if (parameter.isElement())
			map[parameter.nodeName()] = parameter.toElement().text();
	}

	process->setDialogParameter(map);
}

ipfModelerProcessBase* ipfGraphicsScene::createProcessBase(const QString & itemName)
{
	ipfModelerProcessBase *base = nullptr;

	if (itemName == MODELER_IN)
		base = new ipfModelerProcessIn(this, MODELER_IN);
	else if (itemName == MODELER_OUT)
		base = new ipfModelerProcessOut(this, MODELER_OUT);
	else if (itemName == MODELER_TYPECONVERT)
		base = new ipfModelerProcessChildTypeConvert(this, MODELER_TYPECONVERT);
	else if (itemName == MODELER_FRACCLIP)
		base = new ipfModelerProcessChildFracClip(this, MODELER_FRACCLIP);
	else if (itemName == MODELER_QUICKVIEW)
		base = new ipfModelerProcessChildQuickView(this, MODELER_QUICKVIEW);
	else if (itemName == MODELER_RESAMPLE)
		base = new ipfModelerProcessChildResample(this, MODELER_RESAMPLE);
	else if (itemName == MODELER_MOSAIC)
		base = new ipfModelerProcessChildMosaic(this, MODELER_MOSAIC);
	else if (itemName == MODELER_TRANSFORM)
		base = new ipfModelerProcessChildTransform(this, MODELER_TRANSFORM);
	else if (itemName == MODELER_CLIP_VECTOR)
		base = new ipfModelerProcessChildClipVector(this, MODELER_CLIP_VECTOR);
	else if (itemName == MODELER_PIXELDECIMALS)
		base = new ipfModelerProcessChildPixelDecimals(this, MODELER_PIXELDECIMALS);
	else if (itemName == MODELER_EXCEL_METADATA)
		base = new ipfModelerProcessChildCreateMetadata(this, MODELER_EXCEL_METADATA);
	else if (itemName == MODELER_TFW)
		base = new ipfModelerProcessChildCreateTfw(this, MODELER_TFW);
	else if (itemName == MODELER_BUILDOVERVIEWS)
		base = new ipfModelerProcessChildbuildOverviews(this, MODELER_BUILDOVERVIEWS);
	else if (itemName == MODELER_RECTPOSITION)
		base = new ipfModelerProcessChildProcessRasterRectPosition(this, MODELER_RECTPOSITION);
	else if (itemName == MODELER_FRACDIFFERCHECK)
		base = new ipfModelerProcessChildDifferenceCheck(this, MODELER_FRACDIFFERCHECK);
	else if (itemName == MODELER_FRACEXTENTCHECK)
		base = new ipfModelerProcessChildFracExtentCheck(this, MODELER_FRACEXTENTCHECK);
	else if (itemName == MODELER_PIXEL_REPLACE)
		base = new ipfModelerProcessChildPixelMoidfyValue(this, MODELER_PIXEL_REPLACE);
	else if (itemName == MODELER_WATERFLATTENCHECK)
		base = new ipfModelerProcessChildWaterFlatten(this, MODELER_WATERFLATTENCHECK);
	else if (itemName == MODELER_PROJECTIONCHECK)
		base = new ipfModelerProcessChildProjectionCheck(this, MODELER_PROJECTIONCHECK);
	else if (itemName == MODELER_ZCHECK)
		base = new ipfModelerProcessChildZCheck(this, MODELER_ZCHECK);
	else if (itemName == MODELER_FRACEXTENTPROCESS)
		base = new ipfModelerProcessChildConsistency(this, MODELER_FRACEXTENTPROCESS);
	else if (itemName == MODELER_SLOPCALCULATION)
		base = new ipfModelerProcessChildSlopCalculation(this, MODELER_SLOPCALCULATION);
	else if (itemName == MODELER_INVALIDVALUECHECK)
		base = new ipfModelerProcessChildInvalidValueCheck(this, MODELER_INVALIDVALUECHECK);
	else if (itemName == MODELER_DEMGROSSERRORCHECK)
		base = new ipfModelerProcessChildDemGrossErrorCheck(this, MODELER_DEMGROSSERRORCHECK);
	else if (itemName == MODELER_RASTERINFOPRINT)
		base = new ipfModelerProcessChildRasterInfoPrint(this, MODELER_RASTERINFOPRINT);
	else if (itemName == MODELER_VEGETATION_EXTRACTION)
		base = new ipfModelerProcessChildVegeataionExtraction(this, MODELER_VEGETATION_EXTRACTION);
	else if (itemName == MODELER_EXTRACT_RASTER_RANGE)
		base = new ipfModelerProcessChildExtractRasterRange(this, MODELER_EXTRACT_RASTER_RANGE);
	else if (itemName == MODELER_WATERS_EXTRACTION)
		base = new ipfModelerProcessChildWatersExtraction(this, MODELER_WATERS_EXTRACTION);
	else if (itemName == MODELER_SETNODATA)
		base = new ipfModelerProcessChildSetNodata(this, MODELER_SETNODATA);
	else if (itemName == MODELER_DSMDEMDIFFECHECK)
		base = new ipfModelerProcessChildDSMDEMDifferenceCheck(this, MODELER_DSMDEMDIFFECHECK);
	else if (itemName == MODELER_DSMDEMDIFFEPROCESS)
		base = new ipfModelerProcessChildDSMDEMDifferenceProcess(this, MODELER_DSMDEMDIFFEPROCESS);
	else if (itemName == MODELER_RANGEMOIDFYVALUE || itemName == MODELER_SEAMOIDFYVALUE)
		base = new ipfModelerProcessChildRangeMoidfyValue(this, itemName);

	return base;
}
