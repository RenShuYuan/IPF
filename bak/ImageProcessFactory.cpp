#include "ImageProcessFactory.h"
#include "ipf\ipfapplication.h"
#include "ipf\lpfDatabaseManagement.h"

#include <QMessageBox>
#include <QGridLayout>
#include <QFont>
#include <QProgressBar>
#include <QToolButton>

#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include "qgslayertreeview.h"
#include "gui/qgsmessagebar.h"
#include "qgis/app/qgisappstylesheet.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgsstatusbar.h"
#include "qgis/app/qgsstatusbarcoordinateswidget.h"
#include "qgis/app/qgsstatusbarscalewidget.h"
#include "qgsmaptoolpan.h"

ImageProcessFactory *ImageProcessFactory::sInstance = nullptr;

ImageProcessFactory::ImageProcessFactory(QWidget *parent)
	: QMainWindow(parent)
{
	if (sInstance)
	{
		QMessageBox::critical(this, QStringLiteral("多个实例"), QStringLiteral("检测到多个应用对象的实例。"));
		abort();
	}
	sInstance = this;

	ui.setupUi(this);

	QWidget *centralWidget = this->centralWidget();
	QGridLayout *centralLayout = new QGridLayout(centralWidget);
	centralWidget->setLayout(centralLayout);
	centralLayout->setContentsMargins(0, 0, 0, 0);

	// 创建绘图区
	mMapCanvas = new QgsMapCanvas(centralWidget);
	mMapCanvas->setObjectName(QStringLiteral("theMapCanvas"));
	connect(mMapCanvas, &QgsMapCanvas::messageEmitted, this, &ImageProcessFactory::displayMessage);
	mMapCanvas->setWhatsThis(tr("Map canvas. This is where raster and vector "
		"layers are displayed when added to the map"));

	//mMapCanvas->setCanvasColor(QColor("#3d3d3d"));
	centralLayout->addWidget(mMapCanvas, 1, 0, 1, 1);
	mMapCanvas->setFocus();

	// 初始化信息显示条
	mInfoBar = new QgsMessageBar(centralWidget);
	mInfoBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	centralLayout->addWidget(mInfoBar, 0, 0, 1, 1);

	// 设置全局字体
	mStyleSheetBuilder = new QgisAppStyleSheet(this);
	QMap<QString, QVariant> mStyleSheetNewOpts = mStyleSheetBuilder->defaultOptions();
	mStyleSheetNewOpts.insert(QStringLiteral("fontFamily"), QVariant(QStringLiteral("微软雅黑")));
	mStyleSheetBuilder->buildStyleSheet(mStyleSheetNewOpts);

	initActions();
	createStatusBar();
	createCanvasTools();
	mMapCanvas->freeze();

	// 设置主题
	QgsApplication::setUITheme("Night Mapping");

	// 初始化所需环境
	QgsApplication::initQgis();
  	mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  	mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

	// IPF class
	ipf = new lpfDatabaseManagement(this);
	
}

void ImageProcessFactory::displayMessage(const QString &title, const QString &message, Qgis::MessageLevel level)
{
	messageBar()->pushMessage(title, message, level, messageTimeout());
}

ImageProcessFactory::~ImageProcessFactory()
{
	delete mStyleSheetBuilder;

	delete mMapTools.mZoomIn;
	delete mMapTools.mZoomOut;
	delete mMapTools.mPan;
	delete mMapTools.mAddPart;
	delete mMapTools.mAddRing;
	delete mMapTools.mFillRing;
	delete mMapTools.mAnnotation;
	delete mMapTools.mChangeLabelProperties;
	delete mMapTools.mDeletePart;
	delete mMapTools.mDeleteRing;
	delete mMapTools.mFeatureAction;
	delete mMapTools.mFormAnnotation;
	delete mMapTools.mHtmlAnnotation;
	delete mMapTools.mMeasureAngle;
	delete mMapTools.mMeasureArea;
	delete mMapTools.mMeasureDist;
	delete mMapTools.mMoveFeature;
	delete mMapTools.mMoveFeatureCopy;
	delete mMapTools.mMoveLabel;
	delete mMapTools.mVertexTool;
	delete mMapTools.mOffsetCurve;
	delete mMapTools.mPinLabels;
	delete mMapTools.mReshapeFeatures;
	delete mMapTools.mRotateFeature;
	delete mMapTools.mRotateLabel;
	delete mMapTools.mRotatePointSymbolsTool;
	delete mMapTools.mOffsetPointSymbolTool;
	delete mMapTools.mSelectFreehand;
	delete mMapTools.mSelectPolygon;
	delete mMapTools.mSelectRadius;
	delete mMapTools.mSelectFeatures;
	delete mMapTools.mShowHideLabels;
	delete mMapTools.mSimplifyFeature;
	delete mMapTools.mSplitFeatures;
	delete mMapTools.mSplitParts;
	delete mMapTools.mSvgAnnotation;
	delete mMapTools.mTextAnnotation;
	delete mMapTools.mCircularStringCurvePoint;
	delete mMapTools.mCircularStringRadius;
	delete mMapTools.mCircle2Points;
	delete mMapTools.mCircle3Points;
	delete mMapTools.mCircle3Tangents;
	delete mMapTools.mCircle2TangentsPoint;
	delete mMapTools.mCircleCenterPoint;
	delete mMapTools.mEllipseCenter2Points;
	delete mMapTools.mEllipseCenterPoint;
	delete mMapTools.mEllipseExtent;
	delete mMapTools.mEllipseFoci;
	delete mMapTools.mRectangleCenterPoint;
	delete mMapTools.mRectangleExtent;
	delete mMapTools.mRectangle3Points;
	delete mMapTools.mRegularPolygon2Points;
	delete mMapTools.mRegularPolygonCenterPoint;
	delete mMapTools.mRegularPolygonCenterCorner;
}

QgsMessageBar *ImageProcessFactory::messageBar()
{
	Q_ASSERT(mInfoBar);
	return mInfoBar;
}

int ImageProcessFactory::messageTimeout()
{
	return mSettings.value(QStringLiteral("qgis/messageTimeout"), 5).toInt();
}

void ImageProcessFactory::initActions()
{
	mActionPan = new QAction(QStringLiteral("平移地图"), this);
	mActionPan->setStatusTip(QStringLiteral("平移地图"));
	mActionPan->setIcon(QgsApplication::getThemeIcon("mActionPan.svg"));
	connect(mActionPan, &QAction::triggered, this, &ImageProcessFactory::pan);

	mActionPanToSelected = new QAction(QStringLiteral("选中部分居中"), this);
	mActionPanToSelected->setStatusTip(QStringLiteral("选中部分居中"));
	mActionPanToSelected->setIcon(QgsApplication::getThemeIcon("mActionPanToSelected.svg"));
	connect(mActionPanToSelected, &QAction::triggered, this, &ImageProcessFactory::cs);

	ui.mainToolBar->addAction(mActionPan);
	ui.mainToolBar->addAction(mActionPanToSelected);
	ui.mainToolBar->orientationChanged(Qt::Horizontal);
}

void ImageProcessFactory::createStatusBar()
{
	statusBar()->setStyleSheet(QStringLiteral("QStatusBar::item {border: none;}"));

	mStatusBar = new QgsStatusBar();

	statusBar()->addPermanentWidget(mStatusBar, 10);

	mProgressBar = new QProgressBar(mStatusBar);
	mProgressBar->setObjectName(QStringLiteral("mProgressBar"));
	mProgressBar->setMaximumWidth(100);
	mProgressBar->setMaximumHeight(18);
	mProgressBar->hide();
	mStatusBar->addPermanentWidget(mProgressBar, 1);

	connect(mMapCanvas, &QgsMapCanvas::renderStarting, this, &ImageProcessFactory::canvasRefreshStarted);
	connect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &ImageProcessFactory::canvasRefreshFinished);

	//coords status bar widget
	mCoordsEdit = new QgsStatusBarCoordinatesWidget(mStatusBar);
	mCoordsEdit->setObjectName(QStringLiteral("mCoordsEdit"));
	mCoordsEdit->setMapCanvas(mMapCanvas);
	mStatusBar->addPermanentWidget(mCoordsEdit, 0);

	mScaleWidget = new QgsStatusBarScaleWidget(mMapCanvas, mStatusBar);
	mScaleWidget->setObjectName(QStringLiteral("mScaleWidget"));
	connect(mScaleWidget, &QgsStatusBarScaleWidget::scaleLockChanged, mMapCanvas, &QgsMapCanvas::setScaleLocked);
	mStatusBar->addPermanentWidget(mScaleWidget, 0);

	mOnTheFlyProjectionStatusButton = new QToolButton(mStatusBar);
	mOnTheFlyProjectionStatusButton->setAutoRaise(true);
	mOnTheFlyProjectionStatusButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	mOnTheFlyProjectionStatusButton->setObjectName(QStringLiteral("mOntheFlyProjectionStatusButton"));
	mOnTheFlyProjectionStatusButton->setMaximumHeight(mScaleWidget->height());
	mOnTheFlyProjectionStatusButton->setIcon(QgsApplication::getThemeIcon(QStringLiteral("mIconProjectionEnabled.svg")));
	mOnTheFlyProjectionStatusButton->setWhatsThis(tr("This icon shows whether "
		"on the fly coordinate reference system transformation is enabled or not. "
		"Click the icon to bring up "
		"the project properties dialog to alter this behavior."));
	mOnTheFlyProjectionStatusButton->setToolTip(tr("CRS status - Click "
		"to open coordinate reference system dialog"));
	//connect(mOnTheFlyProjectionStatusButton, &QAbstractButton::clicked, this, &QgisApp::projectPropertiesProjections);
	mStatusBar->addPermanentWidget(mOnTheFlyProjectionStatusButton, 0);
	mStatusBar->showMessage(tr("Ready"));
}

void ImageProcessFactory::canvasRefreshStarted()
{
	mLastRenderTime.restart();
	// if previous render took less than 0.5 seconds, delay the appearance of the
	// render in progress status bar by 0.5 seconds - this avoids the status bar
	// rapidly appearing and then disappearing for very fast renders
	if (mLastRenderTimeSeconds > 0 && mLastRenderTimeSeconds < 0.5)
	{
		mRenderProgressBarTimer.setSingleShot(true);
		mRenderProgressBarTimer.setInterval(500);
		disconnect(mRenderProgressBarTimerConnection);
		mRenderProgressBarTimerConnection = connect(&mRenderProgressBarTimer, &QTimer::timeout, this, [=]()
		{
			showProgress(-1, 0);
		}
		);
		mRenderProgressBarTimer.start();
	}
	else
	{
		showProgress(-1, 0); // trick to make progress bar show busy indicator
	}
}

void ImageProcessFactory::canvasRefreshFinished()
{
	mRenderProgressBarTimer.stop();
	mLastRenderTimeSeconds = mLastRenderTime.elapsed() / 1000.0;
	showProgress(0, 0); // stop the busy indicator
}

void ImageProcessFactory::showProgress(int progress, int totalSteps)
{
	if (progress == totalSteps)
	{
		mProgressBar->reset();
		mProgressBar->hide();
	}
	else
	{
		//only call show if not already hidden to reduce flicker
		if (!mProgressBar->isVisible())
		{
			mProgressBar->show();
		}
		mProgressBar->setMaximum(totalSteps);
		mProgressBar->setValue(progress);
	}
}

void ImageProcessFactory::addIpfDatabase()
{
	QList<QgsMapLayer *> myList;
	QStringList list = ipf->getAllLayer();

	foreach (QString layerName, list)
	{
		QString uri(QString("%1|layername=%2").arg(ipf->getIpfDatabasePath(), layerName));
		//QString uri(QString("%1").arg(ipf->getIpfDatabasePath()));
		QgsVectorLayer *layer = new QgsVectorLayer(uri, layerName, "ogr");
		if (layer->isValid())
		{
			myList << layer;
			//qDebug() << layer->name();
			//QgsProject::instance()->addMapLayer(layer);
		}
		else
		{
			//QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg(layerName));
			QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg("layername"));
			delete layer;
		}
	}

	//mMapCanvas->freeze(true);

	if (myList == QgsProject::instance()->addMapLayers(myList))
		QMessageBox::about(this, QStringLiteral("info"), QStringLiteral("加载成功"));
	else
		QMessageBox::about(this, QStringLiteral("info"), QStringLiteral("加载失败"));
	//QgsProject::instance()->addMapLayers(myList);

	//mMapCanvas->freeze(false);
	//mMapCanvas->stopRendering();
	//mMapCanvas->refreshAllLayers();
	//mMapCanvas->zoomToFullExtent();
}

void ImageProcessFactory::cs()
{
	QMessageBox::critical(this, tr("begin"), tr("aaaaa."));
	addIpfDatabase();
}

void ImageProcessFactory::pan()
{
	mMapCanvas->setMapTool(mMapTools.mPan);
}

void ImageProcessFactory::createCanvasTools()
{
	mMapTools.mPan = new QgsMapToolPan(mMapCanvas);
	mMapTools.mPan->setAction(mActionPan);
}