#include "lc_global.h"
#include "lc_file.h"
#include "lc_library.h"
#include "lc_mainwindow.h"
#include "pieceinf.h"
/*** LPub3D Mod - use LPub3D exportable BrickLink codes table ***/
#include "annotations.h"
/*** LPub3D Mod end ***/

/*** LPub3D Mod - use LPub3D exportable BrickLink tables ***/
/***
static QJsonDocument lcLoadBrickLinkMapping()
{
	std::map<std::string, std::string> Remapping;

	QFile File(lcGetPiecesLibrary()->mLibraryDir.absoluteFilePath(QLatin1String("ldraw2bl.txt")));
	QByteArray Data;

	if (File.open(QIODevice::ReadOnly))
		Data = File.readAll();
	else
	{
		QFile DefaultFile(":/resources/ldraw2bl.txt");

		if (DefaultFile.open(QIODevice::ReadOnly))
			Data = DefaultFile.readAll();
	}

	return QJsonDocument::fromJson(Data);
}

static std::string lcGetBrickLinkPart(const QJsonObject& PartMapping, const QString& LDrawPart)
{
	QJsonObject::const_iterator PartIt = PartMapping.find(LDrawPart);

	return (PartIt != PartMapping.end()) ? PartIt.value().toString().toStdString() : LDrawPart.toStdString();
}

static int lcGetBrickLinkColor(const QJsonObject& ColorMapping, int ColorIndex)
{
	int ColorCode = gColorList[ColorIndex].Code;
	QJsonObject::const_iterator ColorIt = ColorMapping.find(QString::number(ColorCode));

	return (ColorIt != ColorMapping.end()) ? ColorIt.value().toString().toInt() : 0;
}

***/
/*** LPub3D Mod end ***/

class lcBrickLinkItem
{
public:
	lcBrickLinkItem(const std::string& id, int color, int count)
		: mId(id), mColor(color), mCount(count)
	{
	}

	void AddToCount(int count)
	{
		mCount += count;
	}

	std::string mId;
	int mColor;
	int mCount;
};

bool lcExportBrickLink(const QString& SaveFileName, const lcPartsList& PartsList)
{
/*** LPub3D Mod - use LPub3D exportable BrickLink tables ***/
/***
	QJsonDocument Document = lcLoadBrickLinkMapping();
	QJsonObject Root = Document.object();
	QJsonObject PartMapping = Root["Parts"].toObject();
	QJsonObject ColorMapping = Root["Colors"].toObject();
***/
/*** LPub3D Mod end ***/

	lcDiskFile BrickLinkFile(SaveFileName);
	char Line[1024];

	if (!BrickLinkFile.Open(QIODevice::WriteOnly))
	{
/*** LPub3D Mod - export ***/
        QMessageBox::warning(gMainWindow, QObject::tr("Visual Editor"), QObject::tr("Could not open file '%1' for writing.").arg(SaveFileName));
		return false;
/*** LPub3D Mod end ***/
	}

	std::map<std::string, lcBrickLinkItem> Inventory;

	BrickLinkFile.WriteLine("<INVENTORY>\n");

	for (const auto& PartIt : PartsList)
	{
		const PieceInfo* Info = PartIt.first;

		for (const auto& ColorIt : PartIt.second)
		{
			char FileName[LC_PIECE_NAME_LEN];
			strcpy(FileName, Info->mFileName);
			char* Ext = strchr(FileName, '.');
			if (Ext)
				*Ext = 0;

/*** LPub3D Mod - use LPub3D exportable BrickLink tables ***/
			std::string Remapped = Annotations::getBrickLinkPartId(FileName).toStdString();
			int ColorCode = gColorList[ColorIt.first].Code;
			int Color = Annotations::getBrickLinkColor(ColorCode);
/*** LPub3D Mod end ***/
			std::string key(Remapped);
			key += "-" + std::to_string(Color);
			auto iter = Inventory.find(key);
			if (iter == Inventory.end())
			{
				Inventory.emplace(std::make_pair(key, lcBrickLinkItem(Remapped, Color, ColorIt.second)));
			}
			else
			{
				iter->second.AddToCount(ColorIt.second);
			}
		}
	}

	for (const auto& Item : Inventory)
	{
		BrickLinkFile.WriteLine("  <ITEM>\n");
		BrickLinkFile.WriteLine("    <ITEMTYPE>P</ITEMTYPE>\n");
		sprintf(Line, "    <ITEMID>%s</ITEMID>\n", Item.second.mId.c_str());
		BrickLinkFile.WriteLine(Line);

		sprintf(Line, "    <MINQTY>%d</MINQTY>\n", Item.second.mCount);
		BrickLinkFile.WriteLine(Line);

		if (Item.second.mColor)
		{
			sprintf(Line, "    <COLOR>%d</COLOR>\n", Item.second.mColor);
			BrickLinkFile.WriteLine(Line);
		}

		BrickLinkFile.WriteLine("  </ITEM>\n");
	}

	BrickLinkFile.WriteLine("</INVENTORY>\n");

/*** LPub3D Mod - export ***/
	return true;
/*** LPub3D Mod end ***/
}
