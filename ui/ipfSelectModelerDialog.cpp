#include "ipfSelectModelerDialog.h"
#include "head.h"

ipfSelectModelerDialog::ipfSelectModelerDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QStringList moder;
	moder << MODELER_OUT
		<< MODELER_TYPECONVERT
		<< MODELER_QUICKVIEW
		<< MODELER_RESAMPLE
		<< MODELER_MOSAIC
		<< MODELER_TRANSFORM
		<< MODELER_CLIP_VECTOR
		<< MODELER_PIXELDECIMALS
		<< MODELER_RECTPOSITION
		<< MODELER_PIXEL_REPLACE
		<< MODELER_RANGEMOIDFYVALUE
		<< MODELER_WATERS_EXTRACTION
		<< MODELER_RASTERINFOPRINT
		<< MODELER_BUILDOVERVIEWS
		<< MODELER_SETNODATA
		<< MODELER_FRACCLIP
		<< MODELER_VEGETATION_EXTRACTION
		//<< MODELER_FRACEXTENTPROCESS
		<< MODELER_TFW
		<< MODELER_EXCEL_METADATA
		<< MODELER_DSMDEMDIFFEPROCESS
		<< MODELER_FRACDIFFERCHECK
		<< MODELER_FRACEXTENTCHECK
		<< MODELER_WATERFLATTENCHECK
		<< MODELER_ZCHECK
		<< MODELER_PROJECTIONCHECK
		<< MODELER_INVALIDVALUECHECK
		<< MODELER_DEMGROSSERRORCHECK
		<< MODELER_DSMDEMDIFFECHECK;
		//<< MODELER_SLOPCALCULATION
		//<< MODELER_EXTRACT_RASTER_RANGE;

	ui.comboBox->addItems(moder);
}

ipfSelectModelerDialog::~ipfSelectModelerDialog()
{
}

void ipfSelectModelerDialog::on_pushButton_clicked()
{
	moderName = ui.comboBox->currentText();
	accept();
}