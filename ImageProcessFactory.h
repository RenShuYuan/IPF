#ifndef IMAGEPROCESSFACTORY_H
#define IMAGEPROCESSFACTORY_H

#include <QtWidgets/QMainWindow>
#include <QObject>
#include "ui_ImageProcessFactory.h"
#include "head.h"

class ipfGraphicsView;
class ipfGraphicsScene;
class QGridLayout;
class QHBoxLayout;
class ipfFlowManage;

class ImageProcessFactory : public QMainWindow
{
	Q_OBJECT

public:
	ImageProcessFactory(QWidget *parent = Q_NULLPTR);
	~ImageProcessFactory();

	void load(const QString &file);

private slots:
	void new_();
	void load();
	void save();
	void run();
	void check();
	void about();
private:
	void initGraphicsView();
	void initTreeView();
	void initButton();

private:
	Ui::ImageProcessFactoryClass ui;
	QSettings mSettings;

	//graphics
	ipfGraphicsView *graphicsView;
	ipfGraphicsScene *scene;

	QGridLayout *grid;
	QHBoxLayout *hbox;

	ipfFlowManage *flow;
};

#endif // IMAGEPROCESSFACTORY_H