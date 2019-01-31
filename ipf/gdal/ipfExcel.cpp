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
	// step1�����ӿؼ�
	excel = new QAxObject(this);
	if (!excel)	// ���
		return excelFile + QStringLiteral(": ���ӿؼ�ʧ�ܡ�");

	excel->setControl("Excel.Application");  // ����Excel�ؼ�
	excel->dynamicCall("SetVisible (bool Visible)", "false"); // ����ʾ����
	excel->setProperty("DisplayAlerts", false);  // ����ʾ�κξ�����Ϣ�����Ϊtrue, ��ô�ر�ʱ���������"�ļ����޸ģ��Ƿ񱣴�"����ʾ

	// step2: �򿪹�����  
	QAxObject* workbooks = excel->querySubObject("WorkBooks"); // ��ȡ����������
	if (!workbooks)	// ���
		return excelFile + QStringLiteral(": ��ȡ����������ʧ�ܡ�");
	workbook = workbooks->querySubObject("Open(const QString&)", excelFile);	// �򿪹�����

	// step3: ��sheet  
	worksheet = workbook->querySubObject("WorkSheets(int)", 1);
	if (!worksheet)	// ���
		return excelFile + QStringLiteral(": ��ȡsheet1ʧ�ܡ�");

	return QString();
}

void ipfExcel::close()
{
	workbook->dynamicCall("Save()");  //�����ļ�  
	workbook->dynamicCall("Close(Boolean)", false);  //�ر��ļ�
	excel->dynamicCall("Quit()");	//�ر�excel
	delete excel; excel = 0;
}

void ipfExcel::editCell(const QString & range, const QVariant & value)
{
	QAxObject *cell = worksheet->querySubObject("Range(QVariant, QVariant)", range);	//��ȡ��Ԫ��
	cell->dynamicCall("SetValue(conts QVariant&)", value); // ���õ�Ԫ���ֵ
}

QString ipfExcel::getCell(const QString & range)
{
	QString str;
	QAxObject *cell = worksheet->querySubObject("Range(QVariant, QVariant)", range);	//��ȡ��Ԫ��
	str = cell->dynamicCall("Value2()").toString();
	return str;
}
