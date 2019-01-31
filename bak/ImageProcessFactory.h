#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ImageProcessFactory.h"
#include "head.h"

#include <QTime>
#include <QTimer>

class lpfDatabaseManagement;
class QAction;
class QProgressBar;
class QgsStatusBarCoordinatesWidget;
class QgsStatusBarScaleWidget;
class QgsDoubleSpinBox;
class QgsLayerTreeView;
class QgsLayerTreeMapCanvasBridge;
class QgsCustomLayerOrderWidget;
class QgsClipboard;
class QgsMessageLogViewer;
class QgisAppStyleSheet;
class QgsMapTool;
class QgsMapOverviewCanvas;
class QgsRasterLayer;
class QgsLayerTreeNode;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMessageBar;
class QgsStatusBar;
class QgsStatusBarCoordinatesWidget;

class ImageProcessFactory : public QMainWindow
{
	Q_OBJECT

public:
	ImageProcessFactory(QWidget *parent = Q_NULLPTR);
	~ImageProcessFactory();

	QgsMessageBar *messageBar();
	int messageTimeout();

public slots:
	void showProgress(int progress, int totalSteps);

private:
	void initActions();

	void createStatusBar();
	void canvasRefreshStarted();
	void canvasRefreshFinished();
	void addIpfDatabase();

private slots:
	void displayMessage(const QString &title, const QString &message, Qgis::MessageLevel level);
	void cs();

	void pan();
	void createCanvasTools();
private:
	Ui::ImageProcessFactoryClass ui;
	static ImageProcessFactory *sInstance;
	QSettings mSettings;

	// QGis
	QgsMapCanvas *mMapCanvas;
	QgsStatusBarCoordinatesWidget *mCoordsEdit = nullptr;
	QgsStatusBarScaleWidget *mScaleWidget = nullptr;
	QgsDoubleSpinBox *mRotationEdit = nullptr;
	QgisAppStyleSheet *mStyleSheetBuilder = nullptr;
	QgsLayerTreeView *mLayerTreeView = nullptr;
	QgsMessageBar* mInfoBar = nullptr;
	QgsStatusBar *mStatusBar = nullptr;
	QProgressBar *mProgressBar = nullptr;
	QToolButton *mOnTheFlyProjectionStatusButton = nullptr;
	//QgsLayerTreeMapCanvasBridge *mLayerTreeCanvasBridge;
	//QgsCustomLayerOrderWidget *mMapLayerOrder;
	//QgsClipboard *mInternalClipboard;
	//QgsMessageLogViewer *mLogViewer;

	QTime mLastRenderTime;
	double mLastRenderTimeSeconds = 0;
	QTimer mRenderProgressBarTimer;
	QMetaObject::Connection mRenderProgressBarTimerConnection;

	// QActions
	QAction *mActionPan;
	QAction *mActionPanToSelected;

	QString mVectorFileFilter;
	QString mRasterFileFilter;

	// IPF class
	lpfDatabaseManagement *ipf;

	class Tools
	{
	public:

		Tools() = default;

		QgsMapTool *mZoomIn = nullptr;
		QgsMapTool *mZoomOut = nullptr;
		QgsMapTool *mPan = nullptr;
		//QgsMapToolIdentifyAction *mIdentify = nullptr;
		QgsMapTool *mFeatureAction = nullptr;
		QgsMapTool *mMeasureDist = nullptr;
		QgsMapTool *mMeasureArea = nullptr;
		QgsMapTool *mMeasureAngle = nullptr;
		//QgsMapToolAddFeature *mAddFeature = nullptr;
		QgsMapTool *mCircularStringCurvePoint = nullptr;
		QgsMapTool *mCircularStringRadius = nullptr;
		QgsMapTool *mCircle2Points = nullptr;
		QgsMapTool *mCircle3Points = nullptr;
		QgsMapTool *mCircle3Tangents = nullptr;
		QgsMapTool *mCircle2TangentsPoint = nullptr;
		QgsMapTool *mCircleCenterPoint = nullptr;
		QgsMapTool *mEllipseCenter2Points = nullptr;
		QgsMapTool *mEllipseCenterPoint = nullptr;
		QgsMapTool *mEllipseExtent = nullptr;
		QgsMapTool *mEllipseFoci = nullptr;
		QgsMapTool *mRectangleCenterPoint = nullptr;
		QgsMapTool *mRectangleExtent = nullptr;
		QgsMapTool *mRectangle3Points = nullptr;
		QgsMapTool *mRegularPolygon2Points = nullptr;
		QgsMapTool *mRegularPolygonCenterPoint = nullptr;
		QgsMapTool *mRegularPolygonCenterCorner = nullptr;
		QgsMapTool *mMoveFeature = nullptr;
		QgsMapTool *mMoveFeatureCopy = nullptr;
		QgsMapTool *mOffsetCurve = nullptr;
		QgsMapTool *mReshapeFeatures = nullptr;
		QgsMapTool *mSplitFeatures = nullptr;
		QgsMapTool *mSplitParts = nullptr;
		QgsMapTool *mSelect = nullptr;
		QgsMapTool *mSelectFeatures = nullptr;
		QgsMapTool *mSelectPolygon = nullptr;
		QgsMapTool *mSelectFreehand = nullptr;
		QgsMapTool *mSelectRadius = nullptr;
		QgsMapTool *mVertexAdd = nullptr;
		QgsMapTool *mVertexMove = nullptr;
		QgsMapTool *mVertexDelete = nullptr;
		QgsMapTool *mAddRing = nullptr;
		QgsMapTool *mFillRing = nullptr;
		QgsMapTool *mAddPart = nullptr;
		QgsMapTool *mSimplifyFeature = nullptr;
		QgsMapTool *mDeleteRing = nullptr;
		QgsMapTool *mDeletePart = nullptr;
		QgsMapTool *mVertexTool = nullptr;
		QgsMapTool *mRotatePointSymbolsTool = nullptr;
		QgsMapTool *mOffsetPointSymbolTool = nullptr;
		QgsMapTool *mAnnotation = nullptr;
		QgsMapTool *mFormAnnotation = nullptr;
		QgsMapTool *mHtmlAnnotation = nullptr;
		QgsMapTool *mSvgAnnotation = nullptr;
		QgsMapTool *mTextAnnotation = nullptr;
		QgsMapTool *mPinLabels = nullptr;
		QgsMapTool *mShowHideLabels = nullptr;
		QgsMapTool *mMoveLabel = nullptr;
		QgsMapTool *mRotateFeature = nullptr;
		QgsMapTool *mRotateLabel = nullptr;
		QgsMapTool *mChangeLabelProperties = nullptr;
	} mMapTools;
};
