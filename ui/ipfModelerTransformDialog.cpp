#include "ipfModelerTransformDialog.h"
#include "qgsprojectionselectionwidget.h"
#include "qgscoordinatereferencesystem.h"

ipfModelerTransformDialog::ipfModelerTransformDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList list;
	list << QStringLiteral("near")
		<< QStringLiteral("bilinear")
		<< QStringLiteral("cubic")
		<< QStringLiteral("cubicspline")
		<< QStringLiteral("lanczos")
		<< QStringLiteral("average")
		<< QStringLiteral("mode")
		<< QStringLiteral("max")
		<< QStringLiteral("min")
		<< QStringLiteral("med")
		<< QStringLiteral("Q1")
		<< QStringLiteral("Q3");
	ui.comboBox->addItems(list);
	resampling_method = QStringLiteral("near");

	// 添加参照坐标系选择小组件
	leUavLayerSrcCrs = new QgsProjectionSelectionWidget(this);
	leUavLayerSrcCrs->setOptionVisible(QgsProjectionSelectionWidget::DefaultCrs, false);
	ui.verticalLayout->insertWidget(0, leUavLayerSrcCrs);

	leUavLayerDestCrs = new QgsProjectionSelectionWidget(this);
	leUavLayerDestCrs->setOptionVisible(QgsProjectionSelectionWidget::DefaultCrs, false);
	ui.verticalLayout->insertWidget(1, leUavLayerDestCrs);
}

ipfModelerTransformDialog::~ipfModelerTransformDialog()
{
}

QMap<QString, QString> ipfModelerTransformDialog::getParameter()
{
	QMap<QString, QString> map;
	map["resampling_method"] = resampling_method;
	map["s_srs"] = s_srs;
	map["t_srs"] = t_srs;

	return map;
}

void ipfModelerTransformDialog::setParameter(QMap<QString, QString> map)
{
	resampling_method = map["resampling_method"];
	s_srs = map["s_srs"];
	t_srs = map["t_srs"];

	ui.comboBox->setCurrentText(resampling_method);

	QgsCoordinateReferenceSystem srcCrs;
	srcCrs.createFromString(s_srs);
	leUavLayerSrcCrs->setCrs(srcCrs);
	QgsCoordinateReferenceSystem destCrs;
	destCrs.createFromString(t_srs);
	leUavLayerDestCrs->setCrs(destCrs);
}

void ipfModelerTransformDialog::on_pushButton_clicked()
{
	resampling_method = ui.comboBox->currentText();

	QgsCoordinateReferenceSystem srcCrs = leUavLayerSrcCrs->crs();
	QgsCoordinateReferenceSystem destCrs = leUavLayerDestCrs->crs();
	if (srcCrs.isValid() && destCrs.isValid())
	{
		s_srs = srcCrs.authid();
		t_srs = destCrs.authid();
	}

	accept();
}