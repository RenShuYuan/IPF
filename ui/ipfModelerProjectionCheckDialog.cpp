#include "ipfModelerProjectionCheckDialog.h"
#include "qgsprojectionselectionwidget.h"
#include "qgscoordinatereferencesystem.h"

ipfModelerProjectionCheckDialog::ipfModelerProjectionCheckDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	// 添加参照坐标系选择小组件
	leUavLayerSrcCrs = new QgsProjectionSelectionWidget(this);
	leUavLayerSrcCrs->setOptionVisible(QgsProjectionSelectionWidget::DefaultCrs, false);
	ui.verticalLayout->insertWidget(0, leUavLayerSrcCrs);
}

ipfModelerProjectionCheckDialog::~ipfModelerProjectionCheckDialog()
{
}

QMap<QString, QString> ipfModelerProjectionCheckDialog::getParameter()
{
	QMap<QString, QString> map;
	map["s_srs"] = s_srs;
	map["saveName"] = saveName;
	return map;
}

void ipfModelerProjectionCheckDialog::setParameter(QMap<QString, QString> map)
{
	s_srs = map["s_srs"];
	QgsCoordinateReferenceSystem srcCrs;
	srcCrs.createFromString(s_srs);
	leUavLayerSrcCrs->setCrs(srcCrs);

	saveName = map["saveName"];
	ui.lineEdit->setText(saveName);
}

void ipfModelerProjectionCheckDialog::on_pushButton_2_clicked()
{
	QString path = mSettings.value("/rasterPath", "/home").toString();
	QString saveName = QFileDialog::getSaveFileName(this, QStringLiteral("输出错误文件")
		, path + QStringLiteral("/投影检查问题.txt")
		, QStringLiteral("文本文件(*.txt)"));
	if (!saveName.isEmpty())
	{
		ui.lineEdit->setText(QDir::toNativeSeparators(saveName));     // 将"/"转换为"\\"
	}
}

void ipfModelerProjectionCheckDialog::on_pushButton_clicked()
{
	QgsCoordinateReferenceSystem srcCrs = leUavLayerSrcCrs->crs();
	if (srcCrs.isValid())
	{
		s_srs = srcCrs.authid();
	}
	saveName = ui.lineEdit->text();

	accept();
}