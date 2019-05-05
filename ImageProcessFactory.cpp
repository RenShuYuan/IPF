#include "ImageProcessFactory.h"
#include "ipf/ipfapplication.h"
#include "ipf/Graphics/ipfGraphicsView.h"
#include "ipf/Graphics/ipfGraphicsScene.h"
#include "ipf/Process/ipfFlowManage.h"
#include "ipf/gdal/ipfPushButton.h"
#include "ui/ipfAboutDialog.h"

#include <QMessageBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QProgressBar>
#include <QToolButton>
#include <QPushButton>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QFile>
#include <QDir>
#include <QBitmap>
#include <QSvgRenderer>

ImageProcessFactory::ImageProcessFactory(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	grid = new QGridLayout(ui.centralWidget);
	ui.centralWidget->setLayout(grid);

	hbox = new QHBoxLayout(this);
	hbox->setMargin(20);
	hbox->setSpacing(20);
	grid->addLayout(hbox, 0, 10);

	flow = new ipfFlowManage(this);

	initGraphicsView();
	initButton();
	initTreeView();

	// style
	QString style = QApplication::applicationDirPath() + QStringLiteral("/resources/themes/StyleSheet.txt");
	QFile qss(style);
	qss.open(QFile::ReadOnly);
	qApp->setStyleSheet(qss.readAll());

	//添加环境变量
	QString str = QApplication::applicationDirPath() + QStringLiteral("/resources/data");
	CPLSetConfigOption("GDAL_DATA", str.toStdString().c_str());
	CPLSetConfigOption("USE_RRD", "YES");
	CPLSetConfigOption("GDAL_CACHEMAX", "70%");

	// 注册所有影像格式
	GDALAllRegister();
}

ImageProcessFactory::~ImageProcessFactory()
{
}

void ImageProcessFactory::initGraphicsView()
{
	//QGraphicsView
	scene = new ipfGraphicsScene(flow);
	graphicsView = new ipfGraphicsView(this);
	graphicsView->setScene(scene);
	graphicsView->setAcceptDrops(true);
	grid->addWidget(graphicsView, 0, 0, 20, 20);
}

void ImageProcessFactory::initTreeView()
{
	ui.treeView->setDragDropMode(QTreeWidget::DragOnly);
	ui.treeView->setDropIndicatorShown(true);
	ui.treeView->setHeaderHidden(true);

	// input
	QIcon icon_Input(ipfApplication::getThemeIconPath("input.svg"));
	// output
	QIcon icon_Output(ipfApplication::getThemeIconPath("output.svg")); 
	// 算法
	QIcon icon_sf(ipfApplication::getThemeIconPath("process.svg"));

	QStandardItemModel *model = new QStandardItemModel(ui.treeView);
	ui.treeView->setModel(model);

	QStandardItem* itemProject = new QStandardItem(QStringLiteral("数据管理"));
	model->appendRow(itemProject);
	itemProject->appendRow(new QStandardItem(icon_Input, MODELER_IN));
	itemProject->appendRow(new QStandardItem(icon_Output, MODELER_OUT));

	QStandardItem* itemCoreAlgorithm = new QStandardItem(QStringLiteral("核心模块"));
	model->appendRow(itemCoreAlgorithm);
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_TYPECONVERT));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_QUICKVIEW));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_RESAMPLE));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_MOSAIC));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_TRANSFORM));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_CLIP_VECTOR));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_PIXELDECIMALS));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_PIXEL_REPLACE));
	//itemCoreAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_RANGEMOIDFYVALUE));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_RECTPOSITION));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_WATERS_EXTRACTION));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_RASTERINFOPRINT));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_BUILDOVERVIEWS));
	itemCoreAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_SETNODATA));

	QStandardItem* itemCunstomQQAlgorithm = new QStandardItem(QStringLiteral("全球测图数据处理"));
	model->appendRow(itemCunstomQQAlgorithm);
	itemCunstomQQAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_FRACCLIP));
	itemCunstomQQAlgorithm->appendRow(new QStandardItem(icon_sf, MODELER_DSMDEMDIFFEPROCESS));
	itemCunstomQQAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_VEGETATION_EXTRACTION));
	itemCunstomQQAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_TFW));
	itemCunstomQQAlgorithm->appendRow(new QStandardItem(icon_Output, MODELER_EXCEL_METADATA));

	QStandardItem* itemCunstomQQCheck = new QStandardItem(QStringLiteral("全球测图质量检查"));
	model->appendRow(itemCunstomQQCheck);
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_FRACDIFFERCHECK));
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_FRACEXTENTCHECK));
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_WATERFLATTENCHECK));
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_ZCHECK));
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_PROJECTIONCHECK));
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_INVALIDVALUECHECK));
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_DEMGROSSERRORCHECK)); 
	itemCunstomQQCheck->appendRow(new QStandardItem(icon_Output, MODELER_DSMDEMDIFFECHECK));

	//QStandardItem* itemCunstomSTBC = new QStandardItem(QStringLiteral("科研测试功能"));
	//model->appendRow(itemCunstomSTBC);
	//itemCunstomSTBC->appendRow(new QStandardItem(icon_sf, MODELER_SLOPCALCULATION));
	//itemCunstomSTBC->appendRow(new QStandardItem(icon_sf, MODELER_FRACEXTENTPROCESS));
	//itemCunstomSTBC->appendRow(new QStandardItem(icon_sf, MODELER_EXTRACT_RASTER_RANGE));

	ui.treeView->expandAll();
}

void ImageProcessFactory::initButton()
{
	// new
	ipfPushButton *newButton = new ipfPushButton(QStringLiteral("new.png"), this);
	hbox->addWidget(newButton);
	connect(newButton, &ipfPushButton::clicked, this, &ImageProcessFactory::new_);

	// load
	ipfPushButton *loadButton = new ipfPushButton(QStringLiteral("load.png"), this);
	hbox->addWidget(loadButton);
	connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));

	// save
	ipfPushButton *saveButton = new ipfPushButton(QStringLiteral("save.png"), this);
	hbox->addWidget(saveButton);
	connect(saveButton, &ipfPushButton::clicked, this, &ImageProcessFactory::save);

	hbox->addStretch();

	// check
	ipfPushButton *checkButton = new ipfPushButton(QStringLiteral("check.png"), this);
	hbox->addWidget(checkButton);
	connect(checkButton, &ipfPushButton::clicked, this, &ImageProcessFactory::check);

	// run
	ipfPushButton *runButton = new ipfPushButton(QStringLiteral("run.png"), this);
	hbox->addWidget(runButton);
	connect(runButton, &ipfPushButton::clicked, this, &ImageProcessFactory::run);

	// about
	ipfPushButton *aboutButton = new ipfPushButton(QStringLiteral("about.png"), this);
	hbox->addWidget(aboutButton);
	connect(aboutButton, &ipfPushButton::clicked, this, &ImageProcessFactory::about);
}

void ImageProcessFactory::new_()
{
	scene->new_();
}

void ImageProcessFactory::load()
{
	scene->load();
}

void ImageProcessFactory::load(const QString &file)
{
	scene->load(file);
}

void ImageProcessFactory::save()
{
	scene->save();
}

void ImageProcessFactory::check()
{
	flow->check();
}

void ImageProcessFactory::run()
{
	flow->run();
}

void ImageProcessFactory::about()
{
	ipfAboutDialog dialog;
	dialog.exec();
}