#include "ipfExcel.h"
#include <QAxObject>

ipfExcel::ipfExcel() : QObject(nullptr)
{
}


ipfExcel::~ipfExcel()
{
}

QString ipfExcel::open(const QString & excelFile)
{
	// step1：连接控件
	excel = new QAxObject(this);
	if (!excel)	// 检查
		return excelFile + QStringLiteral(": 连接控件失败。");

	excel->setControl("Excel.Application");  // 连接Excel控件
	excel->dynamicCall("SetVisible (bool Visible)", "false"); // 不显示窗体
	excel->setProperty("DisplayAlerts", false);  // 不显示任何警告信息。如果为true, 那么关闭时会出现类似"文件已修改，是否保存"的提示

	// step2: 打开工作簿  
	QAxObject* workbooks = excel->querySubObject("WorkBooks"); // 获取工作簿集合
	if (!workbooks)	// 检查
		return excelFile + QStringLiteral(": 获取工作簿集合失败。");
	workbook = workbooks->querySubObject("Open(const QString&)", excelFile);	// 打开工作簿

	// step3: 打开sheet  
	worksheet = workbook->querySubObject("WorkSheets(int)", 1);
	if (!worksheet)	// 检查
		return excelFile + QStringLiteral(": 获取sheet1失败。");

	return QString();
}

void ipfExcel::close()
{
	workbook->dynamicCall("Save()");  //保存文件  
	workbook->dynamicCall("Close(Boolean)", false);  //关闭文件
	excel->dynamicCall("Quit()");	//关闭excel
	delete excel; excel = 0;
}

void ipfExcel::editCell(const QString & range, const QVariant & value)
{
	QAxObject *cell = worksheet->querySubObject("Range(QVariant, QVariant)", range);	//获取单元格
	cell->dynamicCall("SetValue(conts QVariant&)", value); // 设置单元格的值
}

QString ipfExcel::getCell(const QString & range)
{
	QString str;
	QAxObject *cell = worksheet->querySubObject("Range(QVariant, QVariant)", range);	//获取单元格
	str = cell->dynamicCall("Value2()").toString();
	return str;
}
