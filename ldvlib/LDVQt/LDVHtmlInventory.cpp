#include "LDVHtmlInventory.h"
#include <LDLib/LDPartsList.h>
#include <LDLib/LDPartCount.h>
#include <LDLoader/LDLMainModel.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/mystring.h>
#include <LDLib/LDPreferences.h>
#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDViewPoint.h>
#include <string>

#include <QFileInfo>
#include <QApplication>
#include <QProgressDialog>
#include <QMessageBox>

#include <LDVWidgetDefaultKeys.h>
#include "messageboxresizable.h"
#include "annotations.h"
#include "meta.h"
#include "version.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

const char *LDVHtmlInventory::sm_style = "\
body\n\
{\n\
	font-family: tahoma, sans-serif;\n\
}\n\
\n\
div\n\
{\n\
	position: absolute;\n\
}\n\
\n\
table\n\
{\n\
	border-collapse: collapse;\n\
	border: 2px solid #000000;\n\
	background-color: #FFFFDD;\n\
	padding: 0px;\n\
}\n\
\n\
table.titleImage\n\
{\n\
	width: 100%;\n\
	border-bottom: 0px;\n\
}\n\
\n\
th.titleImage\n\
{\n\
	background-color: #EEEEEE;\n\
	color: #000000;\n\
	font-size: 150%;\n\
	border-bottom-style: none;\n\
	padding-bottom: 0px;\n\
}\n\
\n\
tr\n\
{\n\
	page-break-inside: avoid;\n\
}\n\
\n\
th\n\
{\n\
	background-color: #0080D0;\n\
	color: #FFFFFF;\n\
	border-bottom: 1px solid #000000;\n\
	border-right: 1px solid #00558A;\n\
	padding: 4px 8px;\n\
}\n\
\n\
th.title\n\
{\n\
	background-color: #EEEEEE;\n\
	color: #000000;\n\
	font-size: 150%;\n\
}\n\
\n\
td\n\
{\n\
	vertical-align: top;\n\
	background-color: #FFFFDD;\n\
	border-bottom: 1px solid #BFBFA5;\n\
	border-right: 1px solid #BFBFA5;\n\
	padding: 2px 8px;\n\
}\n\
\n\
td.elementid\n\
{\n\
    text-align: right;\n\
}\n\
\n\
td.quantity\n\
{\n\
	text-align: right;\n\
}\n\
\n\
td.colorNumber\n\
{\n\
	text-align: right;\n\
	border-right-style: none;\n\
}\n\
\n\
td.image\n\
{\n\
	border-right-style: none;\n\
}\n\
\n\
table.color\n\
{\n\
	border-style: none;\n\
	width: 100%;\n\
}\n\
\n\
table.color td\n\
{\n\
	border-style: none;\n\
	padding: 0px 8px 0px 0px;\n\
	width: 100%\n\
}\n\
\n\
table.color td.colorBox\n\
{\n\
	padding: 1px 0px;\n\
	width: auto\n\
}\n\
\n\
table.colorBox\n\
{\n\
	border: 1px solid black;\n\
	width: 2em;\n\
	height: 1em;\n\
	padding: 0px;\n\
}\n\
\n\
table.colorBox td\n\
{\n\
	padding: 0px;\n\
}\n\
\n\
table.credits\n\
{\n\
	border-collapse: collapse;\n\
	border-style: none;\n\
	background-color: transparent;\n\
	margin: 0px;\n\
	width: 100%;\n\
}\n\
\n\
td.credits\n\
{\n\
	background-color: #EEEEEE;\n\
	border-style: none;\n\
	padding: 0px;\n\
}\n\
\n\
table.credits td\n\
{\n\
	background-color: transparent;\n\
	color: #808080;\n\
	border-style: none;\n\
	font-size: 65%;\n\
}\n\
\n\
a\n\
{\n\
	text-decoration: none;\n\
	border-style: none;\n\
}\n\
\n\
a:link\n\
{\n\
	color: #000080;\n\
}\n\
\n\
a:visited\n\
{\n\
	color: #0000FF;\n\
}\n\
\n\
a:hover\n\
{\n\
	text-decoration: underline;\n\
	color: #000080;\n\
}\n\
\n\
img\n\
{\n\
	float: right;\n\
}\n\
\n\
th img\n\
{\n\
	float: none;\n\
}\n\
\n\
a img\n\
{\n\
	border-style: none;\n\
}\n\
\n\
:link:hover img\n\
{\n\
	background-color: #D0E8FF;\n\
}\n\
\n\
:visited:hover img\n\
{\n\
	background-color: #A0C0FF;\n\
}\n\
";

const char *LDVHtmlInventory::sm_cssHeader = "\
/* CSS Style Sheet auto-generated by LDView */\n\
\n\
";

const char *LDVHtmlInventory::sm_cssFilename = "LDViewPartsList.css";

LDVHtmlInventory::LDVHtmlInventory(void) :
	m_prefs(new LDPreferences),
    m_viewPoint(nullptr)
{
	int i;

	m_prefs->loadInventorySettings();
	m_showModel = m_prefs->getInvShowModel();
	m_overwriteSnapshot = m_prefs->getInvOverwriteSnapshot();
	m_externalCss = m_prefs->getInvExternalCss();
	m_partImages = m_prefs->getInvPartImages();
	m_showFile = m_prefs->getInvShowFile();
	m_showTotal = m_prefs->getInvShowTotal();
	m_lastSavePath = m_prefs->getInvLastSavePath();

    loadOtherSettings(); // this must come before columnOrder init

	const LongVector &columnOrder = m_prefs->getInvColumnOrder();
	for (i = 0; i < (int)columnOrder.size(); i++)
	{
		m_columnOrder.push_back((LDVPartListColumn)columnOrder[i]);
	}

    Meta meta;
    bool bl = meta.LPub.bom.partElements.bricklinkElements.value();
    m_lookupDefault = bl ? LookUp::Bricklink : LookUp::Peeron;
    m_elementDefault = bl ? ElementSrc::BL : ElementSrc::LEGO;
}

LDVHtmlInventory::~LDVHtmlInventory(void)
{
}

void LDVHtmlInventory::dealloc(void)
{
	m_prefs->release();
	TCObject::dealloc();
}

void LDVHtmlInventory::populateColumnMap(void)
{
	int i;

	m_columns = (int)m_columnOrder.size();
	m_columnMap.clear();
	for (i = 0; i < (int)m_columnOrder.size(); i++)
	{
		LDVPartListColumn column = m_columnOrder[i];

		m_columnMap[column] = true;
		switch (column)
		{
        case LDVPLCPart:
			if (m_partImages)
			{
				m_columns++;
			}
			break;
        case LDVPLCColor:
			m_columns++;
			break;
		default:
			// Get rid of gcc warning
			break;
		}
	}
}

bool LDVHtmlInventory::generateHtml(
    const char *filename,
    LDPartsList *partsList,
    const char *modelName)
{
    FILE *file = ucfopen(filename, "w");
    size_t nSlashSpot;

    m_lastFilename = filename;
    m_lastSavePath = filename;
    populateColumnMap();
    nSlashSpot = m_lastSavePath.find_last_of("/\\");
    if (nSlashSpot < m_lastSavePath.size())
    {
        m_lastSavePath = m_lastSavePath.substr(0, nSlashSpot);
    }
    m_prefs->setInvLastSavePath(m_lastSavePath.c_str());
    m_prefs->commitInventorySettings();
    m_modelName = modelName;
    nSlashSpot = m_modelName.find_last_of("/\\");
    if (nSlashSpot < m_modelName.size())
    {
        m_modelName = m_modelName.substr(nSlashSpot + 1);
    }
    if (file)
    {
        QProgressDialog* ProgressDialog = new QProgressDialog(nullptr);
        ProgressDialog->setWindowFlags(ProgressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
        ProgressDialog->setWindowTitle(QString("HTML Part List"));
        ProgressDialog->setLabelText(QString("Generating %1 HTML Part List...")
                                             .arg(QFileInfo(filename).baseName()));
        ProgressDialog->setMinimum(0);
        ProgressDialog->setValue(0);
        ProgressDialog->setCancelButton(nullptr);
        ProgressDialog->setAutoReset(false);
        ProgressDialog->setModal(true);
        ProgressDialog->show();

        const LDPartCountVector &partCounts = partsList->getPartCounts();
        int i, j, pc;

        pc = int(partCounts.size());

        ProgressDialog->setMaximum(pc);

        writeHeader(file);
        writeTableHeader(file, partsList->getTotalParts());
        for (i = 0; i < pc; i++)
        {
            ProgressDialog->setValue(i);
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            const LDPartCount &partCount = partCounts[i];
            const IntVector &colors = partCount.getColors();
            LDLModel *model = const_cast<LDLModel *>(partCount.getModel());
            LDLPalette *palette = model->getMainModel()->getPalette();
            //int partTotal = partCount.getTotalCount();

            for (j = 0; j < (int)colors.size(); j++)
            {
                int colorNumber = colors[j];
                LDLColorInfo colorInfo = palette->getAnyColorInfo(colorNumber);

                writePartRow(file, partCount, palette, colorInfo, colorNumber);
            }
        }
        writeTableFooter(file);
        writeFooter(file);
        fclose(file);

        ProgressDialog->setValue(pc);
        ProgressDialog->hide();
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        ProgressDialog->deleteLater();

        return true;
    }
    else
    {
        return false;
    }
}

// *** Settings Flags ***

// Note: this class doesn't actually use these flags; it just keeps track of
// the value and makes sure it gets saved and restored in the user defaults.
void LDVHtmlInventory::setGeneratePdfFlag(bool value)
{
    setOtherSetting(m_generatePdf, value, PARTS_LIST_GENERATE_PDF_KEY, true);
}

void LDVHtmlInventory::setLookupSiteFlag(int value)
{
    setOtherSetting(m_lookupSite, value, PARTS_LIST_LOOKUP_SITE_KEY, true);
}

void LDVHtmlInventory::setElementSourceFlag(int value)
{
    setOtherSetting(m_elementSource, value, PARTS_LIST_ELEMENT_SRURCE_KEY, true);
}

bool LDVHtmlInventory::getGeneratePdfFlag()
{
    m_generatePdf = getBoolSetting(PARTS_LIST_GENERATE_PDF_KEY);
    return m_generatePdf;
}

int LDVHtmlInventory::getLookupSiteFlag()
{
    m_lookupSite = getIntSetting(PARTS_LIST_LOOKUP_SITE_KEY,m_lookupDefault);
    return m_lookupSite;
}

int LDVHtmlInventory::getElementSourceFlag()
{
    m_elementSource = getIntSetting(PARTS_LIST_ELEMENT_SRURCE_KEY,m_elementDefault);
    return m_elementSource;
}

void LDVHtmlInventory::loadOtherSettings(void)
{
    getGeneratePdfFlag();
    getLookupSiteFlag();
    getElementSourceFlag();
    LongVector columnOrder;
    columnOrder.push_back(1);	// Part
    columnOrder.push_back(3);	// Color
    columnOrder.push_back(4);	// Element
    columnOrder.push_back(5);	// Quantity
    m_prefs->setInvColumnOrder(columnOrder);
}

void LDVHtmlInventory::setOtherSetting(bool &setting, bool value, const char *key,
                                       bool commit)
{
    if (setting != value)
    {
        setting = value;
        if (commit)
        {
            TCUserDefaults::setLongForKey(value ? 1 : 0, key,
                true /*!m_globalSettings[key]*/);
        }
    }
}

void LDVHtmlInventory::setOtherSetting(int &setting, int value, const char *key,
                                       bool commit)
{
    if (setting != value)
    {
        setting = value;
        if (commit)
        {
            TCUserDefaults::setLongForKey(value, key, true /*!m_globalSettings[key]*/);
        }
    }
}

bool LDVHtmlInventory::getBoolSetting(const char *key, bool defaultValue)
{
    return TCUserDefaults::longForKey(key, (long)defaultValue,
        true /*!m_globalSettings[key]*/) != 0;
}

int LDVHtmlInventory::getIntSetting(const char *key, int defaultValue)
{
    return (int)TCUserDefaults::longForKey(key, defaultValue,
            true /*!m_globalSettings[key]*/);
}

// *** Settings Flags End ***

FILE *LDVHtmlInventory::safeOpenCssFile(const std::string &cssFilename,
									   bool &match)
{
	FILE *cssFile = ucfopen(cssFilename.c_str(), "r");

	match = false;
	if (cssFile)
	{
		size_t headerSize = strlen(sm_cssHeader);
		size_t contentSize = strlen(sm_style);

		std::string fileHeader;
		std::string fileContents;

		fileHeader.resize(headerSize);
		fileContents.resize(contentSize);
		if (fread(&fileHeader[0], headerSize, 1, cssFile) == 1 &&
			fread(&fileContents[0], contentSize, 1, cssFile) == 1 &&
			fileHeader == sm_cssHeader &&
			fileContents == sm_style)
		{
			TCByte test;

			if (fread(&test, 1, 1, cssFile) == 0)
			{
				match = true;
			}
		}
		fclose(cssFile);
        return nullptr;
	}
	return ucfopen(cssFilename.c_str(), "w");
}

bool LDVHtmlInventory::writeExternalCss(void)
{
	if (m_externalCss)
	{
		std::string cssFilename = m_lastSavePath + "/" + sm_cssFilename;
		bool match;
		FILE *cssFile = safeOpenCssFile(cssFilename, match);

		if (cssFile)
		{
			fprintf(cssFile, "%s", sm_cssHeader);
			fprintf(cssFile, "%s", sm_style);
			fclose(cssFile);
		}
		if (cssFile || match)
		{
			return true;
		}
	}
	return false;
}

void LDVHtmlInventory::writeHeader(FILE *file)
{
	//unsigned char utf8Signature[3] = { 0xEF, 0xBB, 0xBF };
	//fwrite(utf8Signature, 3, 1, file);
	fprintf(file, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" "
		"\"http://www.w3.org/TR/html4/strict.dtd\">\n");
	fprintf(file, "<html>\n");
	fprintf(file, "<head>\n");
	fprintf(file, "<meta http-equiv=\"Content-Type\" "
		"content=\"text/html; charset=UTF-8\">\n");
	char title[1024];
	sprintf(title, lsUtf8("PLTitle"), m_modelName.c_str());
	fprintf(file, "<title>%s</title>\n", title);
	if (writeExternalCss())
	{
		fprintf(file, "<link href=\"%s\" title=\"%s\" "
			"rel=\"stylesheet\" type=\"text/css\">\n", sm_cssFilename,
			lsUtf8("PLStyleSheetTitle"));
	}
	else
	{
		fprintf(file, "<style type=\"text/css\" title=\"%s\"><!--\n",
			lsUtf8("PLStyleSheetTitle"));
		fprintf(file, "%s", sm_style);
		fprintf(file, "--></style>\n");
	}
	fprintf(file, "</head>\n");
	fprintf(file, "<body>\n");
	fprintf(file, "<div>\n");
}

void LDVHtmlInventory::writeFooter(FILE *file)
{
	fprintf(file, "</div>\n");
	fprintf(file, "</body>\n");
	fprintf(file, "</html>\n");
}

void LDVHtmlInventory::writePartHeaderCell(FILE *file)
{
	int colSpan = 2;

	if (!m_partImages)
	{
		colSpan = 1;
	}
    writeHeaderCell(file, LDVPLCPart, colSpan);
}

void LDVHtmlInventory::writeHeaderCell(
	FILE *file,
	LDVPartListColumn column,
	int colSpan)
{
	std::string utf8ColumnName;
	ucstringtoutf8(utf8ColumnName, getColumnNameUC(column));
	if (colSpan == 1)
	{
		fprintf(file, "			<th>%s</th>\n", utf8ColumnName.c_str());
	}
	else
	{
		fprintf(file, "			<th colspan=\"%d\">%s</th>\n", colSpan,
			utf8ColumnName.c_str());
	}
}

void LDVHtmlInventory::writeHeaderCell(FILE *file, LDVPartListColumn column)
{
	switch (column)
	{
    case LDVPLCPart:
		writePartHeaderCell(file);
		break;
    case LDVPLCDescription:
        writeHeaderCell(file, LDVPLCDescription, 1);
		break;
    case LDVPLCColor:
        writeHeaderCell(file, LDVPLCColor, 2);
		break;
    case LDVPLCElement:
        writeHeaderCell(file, LDVPLCElement, 1);
        break;
    case LDVPLCQuantity:
        writeHeaderCell(file, LDVPLCQuantity, 1);
		break;
	}
}

void LDVHtmlInventory::writePartCell(
	FILE *file,
	const LDPartCount &partCount,
	LDLPalette *palette,
	const LDLColorInfo &colorInfo,
	int colorNumber)
{
	int peeronColorNumber = colorNumber;
	std::string imgStyle;
	std::string partName = partCount.getFilename();
	size_t nDotSpot = partName.find('.');
	int r, g, b, a;
    bool bl = m_lookupSite == LookUp::Bricklink;

	palette->getRGBA(colorInfo, r, g, b, a);
	if (nDotSpot < partName.size())
	{
		partName = partName.substr(0, nDotSpot);
	}
	if (peeronColorNumber >= 512)
	{
		char bgColor[10];

		sprintf(bgColor, "%02X%02X%02X", r, g, b);
		peeronColorNumber = 7;
		imgStyle = (std::string)"style = \"padding: 4px; "
			"background-color: #" + bgColor + "\" ";
	}
	if (m_partImages)
	{
		std::string className;
		bool official = partCount.getModel()->isOfficial();

		//if (official)
		{
			className = " class=\"image\"";
		}

        // https://www.bricklink.com/v2/catalog/catalogitem.page?P=2780&idColor=7
        if (bl) {
            QStringList element = Annotations::getBLElement(
                QString::number(colorNumber),
                QString::fromStdString(partName),
                ElementSrc::BL).split("-");
            fprintf(file, "			<td%s>"
                "<a href=\"https://www.bricklink.com/v2/catalog/catalogitem.page?P=%s&idColor=%s\">",
                className.c_str(),
                element.at(0).toLatin1().constData(),
                element.at(1).toLatin1().constData());
        } else {
            fprintf(file, "			<td%s>"
                "<a href=\"http://peeron.com/inv/parts/%s\">",
                className.c_str(), partName.c_str());
        }

		if (official)
		{
			fprintf(file, "<img %salt=\"%s\" title=\"%s\" "
				"src=\"http://media.peeron.com/ldraw/images/%d/100/%s.png\">",
                imgStyle.c_str(), lsUtf8(bl ? "PLViewOnBricklink" : "PLViewOnPeeron"),
                lsUtf8(bl ? "PLViewOnBricklink" : "PLViewOnPeeron"),
                peeronColorNumber, partName.c_str());
		}
		else
		{
            fprintf(file, "%s", lsUtf8(bl ? "PLViewOnBricklink" : "PLViewOnPeeron"));
		}
		fprintf(file, "</a></td>\n");
	}
	fprintf(file, "			<td>%s</td>\n", partName.c_str());
}

void LDVHtmlInventory::writeDescriptionCell(
	FILE *file,
	const LDPartCount &partCount)
{
    if (m_columnMap[LDVPLCDescription])
	{
		const char *description = partCount.getModel()->getDescription();
		if (description)
		{
			fprintf(file, "			<td>%s</td>\n", description);
		}
		else
		{
			fprintf(file, "			<td>&lt;%s&gt;</td>\n",
				lsUtf8("PLNoDescription"));
		}
	}
}

void LDVHtmlInventory::writeColorCell(
	FILE *file,
	LDLPalette *palette,
	const LDLColorInfo &colorInfo,
	int colorNumber)
{
	int r, g, b, a;

	palette->getRGBA(colorInfo, r, g, b, a);
	if (strlen(colorInfo.name))
	{
		fprintf(file, "			<td class=\"colorNumber\">%d:</td>\n",
			colorNumber);
	}
	else
	{
		if (colorNumber < 512)
		{
			fprintf(file, "			<td class=\"colorNumber\">%d"
				"</td>\n", colorNumber);
		}
		else
		{
			fprintf(file, "			<td class=\"colorNumber\">#%X"
				"</td>\n", colorNumber);
		}
	}
	fprintf(file, "			<td>\n");
	fprintf(file, "				<table class=\"color\">\n");
	fprintf(file, "					<tr>\n");
	fprintf(file, "						<td rowspan=\"2\">"
		"%s<td>\n", colorInfo.name);
	fprintf(file, "						<td class=\"colorBox\"><table "
		"class=\"colorBox\"><tr><td style=\"background-color: #%02X%02X%02X\">"
		"&nbsp;</td></tr></table></td>\n", r, g, b);
	fprintf(file, "					</tr>\n");
	fprintf(file, "				</table>\n");
	fprintf(file, "			</td>\n");
}

void LDVHtmlInventory::writeElementCell(
    FILE *file,
    const LDPartCount &partCount,
    int colorNumber)
{
    bool bl = m_lookupSite == LookUp::Bricklink;
    std::string partName = partCount.getFilename();
    size_t nDotSpot = partName.find('.');
    if (nDotSpot < partName.size())
    {
        partName = partName.substr(0, nDotSpot);
    }

    QString element = Annotations::getBLElement(
        QString::number(colorNumber),
        QString::fromStdString(partName),
        bl ? ElementSrc::BL:ElementSrc::LEGO);

    fprintf(file, "			<td class=\"elementid\">%s</td>\n",
        element.toLatin1().constData());
}

void LDVHtmlInventory::writeQuantityCell(
	FILE *file,
	const LDPartCount &partCount,
	int colorNumber)
{
	fprintf(file, "			<td class=\"quantity\">%d</td>\n",
		partCount.getColorCount(colorNumber));
}

void LDVHtmlInventory::writeCell(
	FILE *file, LDVPartListColumn column,
	const LDPartCount &partCount,
	LDLPalette *palette,
	const LDLColorInfo &colorInfo,
	int colorNumber)
{
	switch (column)
	{
    case LDVPLCPart:
		writePartCell(file, partCount, palette, colorInfo, colorNumber);
		break;
    case LDVPLCDescription:
		writeDescriptionCell(file, partCount);
		break;
    case LDVPLCColor:
		writeColorCell(file, palette, colorInfo, colorNumber);
		break;
    case LDVPLCElement:
        writeElementCell(file, partCount, colorNumber);
        break;
    case LDVPLCQuantity:
		writeQuantityCell(file, partCount, colorNumber);
		break;
	}
}

const char *LDVHtmlInventory::getSnapshotPath(void) const
{
	m_snapshotPath = m_lastSavePath + "/" + getSnapshotFilename();
	return m_snapshotPath.c_str();
}

std::string LDVHtmlInventory::getSnapshotFilename(void) const
{
	size_t nSpot = m_lastFilename.find_last_of('.');
	std::string filename;

	if (nSpot < m_lastFilename.size())
	{
		filename = m_lastFilename.substr(0, nSpot);
	}
	else
	{
		filename = m_lastFilename;
	}
	filename += ".png";
	nSpot = filename.find_last_of("/\\");
	if (nSpot < filename.size())
	{
		filename = filename.substr(nSpot + 1);
	}
	return filename;
}

void LDVHtmlInventory::writeTableHeader(FILE *file, int totalParts)
{
	size_t i;

	fprintf(file, "<table class=\"titleImage\">\n");
	fprintf(file, "	<tbody>\n");
	if (m_showModel)
	{
		fprintf(file, "		<tr>\n");
		fprintf(file, "			<th class=\"titleImage\" "
			"colspan=\"%d\">\n", m_columns);
		fprintf(file, "				<img alt=\"&lt;%s&gt;\" "
			"title=\"&lt;%s&gt;\" src=\"%s\">\n",
			m_modelName.c_str(), m_modelName.c_str(),
			getSnapshotFilename().c_str());
		fprintf(file, "			</th>\n");
		fprintf(file, "		</tr>\n");
	}
	fprintf(file, "		<tr>\n");
	fprintf(file, "			<th class=\"title\" colspan=\"%d\">\n", m_columns);
	char title[1024];
	sprintf(title, lsUtf8("PLTitle"), m_modelName.c_str());
	fprintf(file, "				%s", title);
	if (m_showTotal)
	{
		fprintf(file, lsUtf8("PLTotalParts"), totalParts);
	}
	fprintf(file, "\n");
	fprintf(file, "			</th>\n");
	fprintf(file, "		</tr>\n");
	fprintf(file, "	</tbody>\n");
	fprintf(file, "</table>\n");
	fprintf(file, "<table>\n");
	fprintf(file, "	<thead>\n");
	fprintf(file, "		<tr>\n");
	for (i = 0; i < m_columnOrder.size(); i++)
	{
		writeHeaderCell(file, m_columnOrder[i]);
	}
	fprintf(file, "		</tr>\n");
	fprintf(file, "	</thead>\n");
	fprintf(file, "	<tbody>\n");
}

void LDVHtmlInventory::writeTableFooter(FILE *file)
{
	const char *ldviewCreditAlign = "left";

	//fprintf(file, "	<tfoot>\n");
	fprintf(file, "		<tr>\n");
	fprintf(file, "			<td colspan=\"%d\" class=\"credits\">\n",
			m_columns);
	fprintf(file, "				<table class=\"credits\">\n");
	fprintf(file, "					<tbody>\n");
	fprintf(file, "						<tr>\n");
	if (!m_partImages)
	{
		ldviewCreditAlign = "center";
	}
	fprintf(file, "							<td align=\"%s\">\n",
			ldviewCreditAlign);
	fprintf(file, "								%s\n",
            lsUtf8("PLVGeneratedBy"));
	fprintf(file, "							</td>\n");
	if (m_partImages)
	{
		fprintf(file, "							<td align=\"right\">\n");
		fprintf(file, "								%s\n",
                lsUtf8("PLProvidedBy"));
		fprintf(file, "							</td>\n");
	}
	fprintf(file, "						</tr>\n");
	fprintf(file, "					</tbody>\n");
	fprintf(file, "				</table>\n");
	fprintf(file, "			</td>\n");
	fprintf(file, "		</tr>\n");
	//fprintf(file, "	</tfoot>\n");
	fprintf(file, "	</tbody>\n");
	fprintf(file, "</table>\n");
}

void LDVHtmlInventory::writePartRow(
	FILE *file,
	const LDPartCount &partCount,
	LDLPalette *palette,
	const LDLColorInfo &colorInfo,
	int colorNumber)
{
	size_t i;

	fprintf(file, "		<tr>\n");
	for (i = 0; i < m_columnOrder.size(); i++)
	{
		writeCell(file, m_columnOrder[i], partCount, palette, colorInfo,
			colorNumber);
	}
	fprintf(file, "		</tr>\n");
}

void LDVHtmlInventory::setShowModelFlag(bool value)
{
	m_showModel = value;
	m_prefs->setInvShowModel(value);
}

void LDVHtmlInventory::setOverwriteSnapshotFlag(bool value)
{
	m_overwriteSnapshot = value;
	m_prefs->setInvOverwriteSnapshot(value);
}

void LDVHtmlInventory::setExternalCssFlag(bool value)
{
	m_externalCss = value;
	m_prefs->setInvExternalCss(value);
}

void LDVHtmlInventory::setPartImagesFlag(bool value)
{
	m_partImages = value;
	m_prefs->setInvPartImages(value);
}

void LDVHtmlInventory::setShowFileFlag(bool value)
{
	// Note: this class doesn't actually use the flag; it just keeps track of
	// the value and makes sure it gets saved and restored in the user defaults.
	m_showFile = value;
	m_prefs->setInvShowFile(value);
}

void LDVHtmlInventory::setShowTotalFlag(bool value)
{
	m_showTotal = value;
	m_prefs->setInvShowTotal(value);
}

void LDVHtmlInventory::setColumnOrder(const LDVPartListColumnVector &value)
{
	LongVector columnOrder;
	int i;

	m_columnOrder = value;
	for (i = 0; i < (int)value.size(); i++)
	{
		columnOrder.push_back(value[i]);
	}
	m_prefs->setInvColumnOrder(columnOrder);
}

bool LDVHtmlInventory::isColumnEnabled(LDVPartListColumn column)
{
	populateColumnMap();
	return m_columnMap[column];
}

CUCSTR LDVHtmlInventory::getColumnNameUC(LDVPartListColumn column)
{
	switch (column)
	{
    case LDVPLCPart:
		return ls(_UC("PLPartColName"));
    case LDVPLCDescription:
		return ls(_UC("PLDescriptionColName"));
    case LDVPLCColor:
		return ls(_UC("PLColorColName"));
    case LDVPLCElement:
        return ls(_UC("PLVElementColName"));
    case LDVPLCQuantity:
		return ls(_UC("PLQuantityColName"));
	}
	return _UC("<Unknown Column Name>");
}

const char *LDVHtmlInventory::getColumnName(LDVPartListColumn column)
{
	switch (column)
	{
    case LDVPLCPart:
		return ls("PLPartColName");
    case LDVPLCDescription:
		return ls("PLDescriptionColName");
    case LDVPLCColor:
		return ls("PLColorColName");
    case LDVPLCElement:
        return ls("PLVElementColName");
    case LDVPLCQuantity:
		return ls("PLQuantityColName");
	}
	return "<Unknown Column Name>";
}

void LDVHtmlInventory::prepForSnapshot(LDrawModelViewer *modelViewer)
{
	TCObject::release(m_viewPoint);
	m_viewPoint = modelViewer->saveViewPoint();
	modelViewer->resetView();
	modelViewer->setRotationSpeed(0);
	modelViewer->setXYPan(0, 0);
	modelViewer->setBackgroundRGB(0xEE, 0xEE, 0xEE);
	modelViewer->setStereoMode(LDVStereoNone);
}

void LDVHtmlInventory::restoreAfterSnapshot(LDrawModelViewer *modelViewer)
{
	if (m_viewPoint)
	{
		modelViewer->restoreViewPoint(m_viewPoint);
		m_viewPoint->release();
	}
}

bool LDVHtmlInventory::isSnapshotNeeded(void) const
{
	if (m_showModel)
	{
		if (m_overwriteSnapshot)
		{
			return true;
		}
		else
		{
			const char *snapshotPath = getSnapshotPath();
			FILE *pFile = ucfopen(snapshotPath, "rb");

			if (pFile)
			{
				fclose(pFile);
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

std::string LDVHtmlInventory::defaultFilename(const char *modelFilename)
{
	char *filePart = filenameFromPath(modelFilename);
	char *dirPart = directoryFromPath(modelFilename);
	std::string htmlFilename;
	
	if (dirPart)
	{
		htmlFilename = dirPart;
		delete[] dirPart;
		htmlFilename += '/';
	}
	char *findSpot = strrchr(filePart, '.');
	if (findSpot)
	{
		findSpot[0] = 0;
	}
	htmlFilename += filePart;
	delete[] filePart;
	htmlFilename += ".html";
	filePart = cleanedUpPath(htmlFilename.c_str());
	htmlFilename = filePart;
	delete[] filePart;
	return htmlFilename;
}

