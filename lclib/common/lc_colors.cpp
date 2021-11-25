#include "lc_global.h"
#include "lc_colors.h"
#include "lc_file.h"
#include "lc_library.h"
#include "lc_application.h"
#include <float.h>

std::vector<lcColor> gColorList;
lcColorGroup gColorGroups[LC_NUM_COLORGROUPS];
int gEdgeColor;
int gDefaultColor;

lcVector4 gInterfaceColors[] = // todo: make the colors configurable and include the grid and other hardcoded colors here as well.
{
	lcVector4(0.898f, 0.298f, 0.400f, 1.000f), // lcInterfaceColor::Selected
/*** LPub3D Mod - Selected Parts ***/	
	lcVector4(0.474f, 0.847f, 0.474f, 1.000f), // lcInterfaceColor::BMSelected
/*** LPub3D Mod end ***/	
	lcVector4(0.400f, 0.298f, 0.898f, 1.000f), // lcInterfaceColor::Focused
	lcVector4(0.500f, 0.800f, 0.500f, 1.000f), // lcInterfaceColor::Camera
	lcVector4(0.500f, 0.800f, 0.500f, 1.000f), // lcInterfaceColor::Light
	lcVector4(0.500f, 0.800f, 0.500f, 0.500f), // lcInterfaceColor::ControlPoint
	lcVector4(0.400f, 0.298f, 0.898f, 0.500f), // lcInterfaceColor::ControlPointFocused
};

LC_ARRAY_SIZE_CHECK(gInterfaceColors, lcInterfaceColor::Count);

static void GetToken(char*& Ptr, char* Token)
{
	while (*Ptr && *Ptr <= 32)
		Ptr++;

	while (*Ptr > 32)
		*Token++ = *Ptr++;

	*Token = 0;
}

static void lcAdjustStudStyleColors(std::vector<lcColor>& Colors, lcStudStyle StudStyle)
{
	const lcPreferences& Preferences = lcGetPreferences();

	if (!Preferences.mAutomateEdgeColor && !lcIsHighContrast(StudStyle))
		return;

	const lcVector4 Edge = lcVector4FromColor(Preferences.mPartEdgeColor);
	const lcVector4 DarkEdge = lcVector4FromColor(Preferences.mDarkEdgeColor);
	const lcVector4 BlackEdge = lcVector4FromColor(Preferences.mBlackEdgeColor);

	const float ContrastControl = Preferences.mPartEdgeContrast;
	const float LightDarkControl = Preferences.mAutomateEdgeColor ? Preferences.mPartColorValueLDIndex : LC_SRGB_TO_LINEAR(Preferences.mPartColorValueLDIndex);

	for (lcColor& Color : Colors)
	{
		const lcVector3 LinearColor = lcSRGBToLinear(lcVector3(Color.Value));
		const float ValueLuminescence = lcLuminescence(LinearColor);

		if (Preferences.mAutomateEdgeColor)
		{
			if (Color.Adjusted)
				continue;

			const float EdgeLuminescence = lcLuminescence(lcSRGBToLinear(lcVector3(Color.Edge)));

			Color.Edge = lcAlgorithmicEdgeColor(LinearColor, ValueLuminescence, EdgeLuminescence, ContrastControl, LightDarkControl);
			Color.Adjusted = true;
		}
		else
		{
			if (Color.Code == 4242)
				continue;
			else if (Color.Code == 0)
				Color.Edge = BlackEdge;
			else if (ValueLuminescence < LightDarkControl)
				Color.Edge = DarkEdge;
			else
				Color.Edge = Edge;
		}
	}
}

static std::vector<lcColor> lcParseColorFile(lcFile& File)
{
	char Line[1024], Token[1024];
	std::vector<lcColor> Colors;
	lcColor Color;

	while (File.ReadLine(Line, sizeof(Line)))
	{
		char* Ptr = Line;

		GetToken(Ptr, Token);
		if (strcmp(Token, "0"))
			continue;

		GetToken(Ptr, Token);
		strupr(Token);
		if (strcmp(Token, "!COLOUR"))
			continue;

		Color.Code = ~0U;
		Color.Translucent = false;
		Color.Group = LC_COLORGROUP_SOLID;
		Color.Value[0] = FLT_MAX;
		Color.Value[1] = FLT_MAX;
		Color.Value[2] = FLT_MAX;
		Color.Value[3] = 1.0f;
		Color.Edge[0] = FLT_MAX;
		Color.Edge[1] = FLT_MAX;
		Color.Edge[2] = FLT_MAX;
		Color.Edge[3] = 1.0f;
/*** LPub3D Mod - use Visual Editor colors ***/
		Color.CValue = ~0U;
		Color.EValue = ~0U;
		Color.Alpha = 255;
/*** LPub3D Mod end ***/

		GetToken(Ptr, Token);
		strncpy(Color.Name, Token, sizeof(Color.Name));
		Color.Name[LC_MAX_COLOR_NAME - 1] = 0;
		strncpy(Color.SafeName, Color.Name, sizeof(Color.SafeName));

/*** LPub3D Mod - load color entry ***/
		char NameCheck[LC_MAX_COLOR_NAME];
		strncpy (NameCheck, Token, 7);
		NameCheck[7] = '\0';
		strupr(NameCheck);
/*** LPub3D Mod end ***/

		for (char* Underscore = strchr((char*)Color.Name, '_'); Underscore; Underscore = strchr(Underscore, '_'))
			*Underscore = ' ';

		for (GetToken(Ptr, Token); Token[0]; GetToken(Ptr, Token))
		{
			strupr(Token);

			if (!strcmp(Token, "CODE"))
			{
				GetToken(Ptr, Token);
				Color.Code = atoi(Token);
			}
			else if (!strcmp(Token, "VALUE"))
			{
				GetToken(Ptr, Token);
				if (Token[0] == '#')
					Token[0] = ' ';

				int Value;
				if (sscanf(Token, "%x", &Value) != 1)
					Value = 0;
/*** LPub3D Mod - use Visual Editor colors ***/
				Color.CValue = Value;
/*** LPub3D Mod end ***/
				Color.Value[2] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Value[1] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Value[0] = (float)(Value & 0xff) / 255.0f;
			}
			else if (!strcmp(Token, "EDGE"))
			{
				GetToken(Ptr, Token);
				if (Token[0] == '#')
					Token[0] = ' ';

				int Value;
				if (sscanf(Token, "%x", &Value) != 1)
					Value = 0;
/*** LPub3D Mod - use Visual Editor colors ***/
				Color.EValue = Value;
/*** LPub3D Mod end ***/

				Color.Edge[2] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Edge[1] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Edge[0] = (float)(Value & 0xff) / 255.0f;
			}
			else if (!strcmp(Token, "ALPHA"))
			{
				GetToken(Ptr, Token);
				int Value = atoi(Token);
				Color.Value[3] = (float)(Value & 0xff) / 255.0f;
				if (Value != 255)
					Color.Translucent = true;
/*** LPub3D Mod - use Visual Editor colors ***/
				Color.Alpha = Value;
/*** LPub3D Mod end ***/
				if (Value == 128)
					Color.Group = LC_COLORGROUP_TRANSLUCENT;
				else if (Value != 0)
					Color.Group = LC_COLORGROUP_SPECIAL;
			}
			else if (!strcmp(Token, "CHROME") || !strcmp(Token, "PEARLESCENT") || !strcmp(Token, "RUBBER") ||
					 !strcmp(Token, "MATTE_METALIC") || !strcmp(Token, "METAL") || !strcmp(Token, "LUMINANCE"))
			{
				Color.Group = LC_COLORGROUP_SPECIAL;
			}
			else if (!strcmp(Token, "MATERIAL"))
			{
				Color.Group = LC_COLORGROUP_SPECIAL;
				break; // Material is always last so ignore it and the rest of the line.
			}
		}

/*** LPub3D Mod - load color entry ***/
		if (!strcmp(NameCheck, "LPUB3D_"))
			Color.Group = LC_COLORGROUP_LPUB3D;
/*** LPub3D Mod end ***/

		if (Color.Code == ~0U || Color.Value[0] == FLT_MAX)
			continue;

		if (Color.Edge[0] == FLT_MAX)
		{
			Color.Edge[0] = 33.0f / 255.0f;
			Color.Edge[1] = 33.0f / 255.0f;
			Color.Edge[2] = 33.0f / 255.0f;
		}

		bool Duplicate = false;

		for (lcColor& ExistingColor : Colors)
		{
			if (ExistingColor.Code == Color.Code)
			{
				ExistingColor = Color;
				Duplicate = true;
				break;
			}
		}

		if (!Duplicate)
			Colors.push_back(Color);
	}

	return Colors;
}

/*** LPub3D Mod - load color entry ***/
bool lcLoadColorFile(lcFile& File, lcStudStyle StudStyle, bool Update)
{
	std::vector<lcColor> Colors = lcParseColorFile(File);
	const bool Valid = !Colors.empty();

	if (Valid)
		lcAdjustStudStyleColors(Colors, StudStyle);

/*** LPub3D Mod - load color entry ***/
	bool Found = Update ? true : false;
	bool FoundMain = Found, FoundEdge = Found, FoundStud = Found, FoundNoColor = Found;;
/*** LPub3D Mod end ***/

	for (const lcColor& Color : Colors)
	{
		switch (Color.Code)
		{
			case 16:
				FoundMain = true;
				break;

			case 24:
				FoundEdge = true;
				break;

			case 4242:
				FoundStud = true;
				break;

			case LC_COLOR_NOCOLOR:
				FoundNoColor = true;
				break;
		}
	}

	if (!FoundMain)
	{
		lcColor MainColor;

		MainColor.Code = 16;
		MainColor.Translucent = false;
		MainColor.Group = LC_COLORGROUP_SOLID;
		MainColor.Value[0] = 1.0f;
		MainColor.Value[1] = 1.0f;
		MainColor.Value[2] = 0.5f;
		MainColor.Value[3] = 1.0f;
		MainColor.Edge[0] = 0.2f;
		MainColor.Edge[1] = 0.2f;
		MainColor.Edge[2] = 0.2f;
		MainColor.Edge[3] = 1.0f;
/*** LPub3D Mod - use Visual Editor colors ***/
		MainColor.CValue = lcColorFromVector3(lcVector3(MainColor.Value));
		MainColor.EValue = lcColorFromVector3(lcVector3(MainColor.Edge));
		MainColor.Alpha = 255;
/*** LPub3D Mod end ***/
		strcpy(MainColor.Name, "Main Color");
		strcpy(MainColor.SafeName, "Main_Color");

		Colors.push_back(MainColor);
	}

	if (!FoundEdge)
	{
		lcColor EdgeColor;

		EdgeColor.Code = 24;
		EdgeColor.Translucent = false;
		EdgeColor.Group = LC_NUM_COLORGROUPS;
		EdgeColor.Value[0] = 0.5f;
		EdgeColor.Value[1] = 0.5f;
		EdgeColor.Value[2] = 0.5f;
		EdgeColor.Value[3] = 1.0f;
		EdgeColor.Edge[0] = 0.2f;
		EdgeColor.Edge[1] = 0.2f;
		EdgeColor.Edge[2] = 0.2f;
		EdgeColor.Edge[3] = 1.0f;
/*** LPub3D Mod - use Visual Editor colors ***/
		EdgeColor.CValue = lcColorFromVector3(lcVector3(EdgeColor.Value));
		EdgeColor.EValue = lcColorFromVector3(lcVector3(EdgeColor.Edge));
		EdgeColor.Alpha = 255;
/*** LPub3D Mod end ***/
		strcpy(EdgeColor.Name, "Edge Color");
		strcpy(EdgeColor.SafeName, "Edge_Color");

		Colors.push_back(EdgeColor);
	}

	if (!FoundStud)
	{
		const lcPreferences& Preferences = lcGetPreferences();
		lcColor StudCylinderColor;

		StudCylinderColor.Code = 4242;
		StudCylinderColor.Translucent = false;
		StudCylinderColor.Group = LC_NUM_COLORGROUPS;
		StudCylinderColor.Value = lcVector4FromColor(Preferences.mStudCylinderColor);
		StudCylinderColor.Edge = lcVector4FromColor(Preferences.mPartEdgeColor);
/*** LPub3D Mod - use Visual Editor colors ***/
		StudCylinderColor.CValue = lcColorFromVector3(lcVector3(StudCylinderColor.Value));
		StudCylinderColor.EValue = lcColorFromVector3(lcVector3(StudCylinderColor.Edge));
		StudCylinderColor.Alpha = 255;
/*** LPub3D Mod end ***/
		strcpy(StudCylinderColor.Name, "Stud Cylinder Color");
		strcpy(StudCylinderColor.SafeName, "Stud_Cylinder_Color");

		Colors.push_back(StudCylinderColor);
	}

	if (!FoundNoColor)
	{
		lcColor NoColor;

		NoColor.Code = LC_COLOR_NOCOLOR;
		NoColor.Translucent = false;
		NoColor.Group = LC_NUM_COLORGROUPS;
		NoColor.Value[0] = 0.5f;
		NoColor.Value[1] = 0.5f;
		NoColor.Value[2] = 0.5f;
		NoColor.Value[3] = 1.0f;
		NoColor.Edge[0] = 0.2f;
		NoColor.Edge[1] = 0.2f;
		NoColor.Edge[2] = 0.2f;
		NoColor.Edge[3] = 1.0f;
/*** LPub3D Mod - use Visual Editor colors ***/
		NoColor.CValue = lcColorFromVector3(lcVector3(NoColor.Value));
		NoColor.EValue = lcColorFromVector3(lcVector3(NoColor.Edge));
		NoColor.Alpha = 255;
/*** LPub3D Mod end ***/
		strcpy(NoColor.Name, "No Color");
		strcpy(NoColor.SafeName, "No_Color");

		Colors.push_back(NoColor);
	}

/*** LPub3D Mod - load color entry ***/
	if (!Update)
	{
		for (lcColor& Color : gColorList)
			Color.Group = LC_NUM_COLORGROUPS;

		for (int GroupIdx = 0; GroupIdx < LC_NUM_COLORGROUPS; GroupIdx++)
			gColorGroups[GroupIdx].Colors.clear();

		gColorGroups[0].Name = QApplication::tr("Solid", "Colors");
		gColorGroups[1].Name = QApplication::tr("Translucent", "Colors");
		gColorGroups[2].Name = QApplication::tr("Special", "Colors");
		gColorGroups[3].Name = QApplication::tr("LPub3D", "Colors");
	}
/*** LPub3D Mod end ***/

	for (lcColor& Color : Colors)
	{
		int ColorIndex;

		for (ColorIndex = 0; ColorIndex < static_cast<int>(gColorList.size()); ColorIndex++)
			if (gColorList[ColorIndex].Code == Color.Code)
				break;

		if (ColorIndex == static_cast<int>(gColorList.size()))
			gColorList.push_back(Color);
		else
			gColorList[ColorIndex] = Color;

		if (Color.Group != LC_NUM_COLORGROUPS)
			gColorGroups[Color.Group].Colors.push_back(ColorIndex);

		if (Color.Code == 16)
			gDefaultColor = ColorIndex;
		else if (Color.Code == 24)
			gEdgeColor = ColorIndex;
	}

	return Valid;
}

/*** LPub3D Mod - load color entry ***/
bool lcLoadColorEntry(const char* ColorEntry, lcStudStyle StudStyle)
{
	QByteArray ColorData;
	ColorData.append(ColorEntry);
	lcMemFile ColorMemFile;
	ColorMemFile.WriteBuffer(ColorData.constData(), ColorData.size());
	ColorMemFile.Seek(0, SEEK_SET);

	return lcLoadColorFile(ColorMemFile, StudStyle, true/*Update*/);
}
/*** LPub3D Mod end ***/

void lcLoadDefaultColors(lcStudStyle StudStyle)
{
	lcDiskFile ConfigFile(":/resources/ldconfig.ldr");

	if (ConfigFile.Open(QIODevice::ReadOnly))
		lcLoadColorFile(ConfigFile, StudStyle);
}

int lcGetColorIndex(quint32 ColorCode)
{
	for (size_t ColorIdx = 0; ColorIdx < gColorList.size(); ColorIdx++)
		if (gColorList[ColorIdx].Code == ColorCode)
			return (int)ColorIdx;

	lcColor Color;

	Color.Code = ColorCode;
	Color.Translucent = false;
	Color.Edge[0] = 0.2f;
	Color.Edge[1] = 0.2f;
	Color.Edge[2] = 0.2f;
	Color.Edge[3] = 1.0f;

	if (ColorCode & LC_COLOR_DIRECT)
	{
		Color.Value[0] = (float)((ColorCode & 0xff0000) >> 16) / 255.0f;
		Color.Value[1] = (float)((ColorCode & 0x00ff00) >> 8) / 255.0f;
		Color.Value[2] = (float)((ColorCode & 0x0000ff) >> 0) / 255.0f;
		Color.Value[3] = 1.0f;
		sprintf(Color.Name, "Color %06X", ColorCode & 0xffffff);
		sprintf(Color.SafeName, "Color_%06X", ColorCode & 0xffffff);
	}
	else
	{
		Color.Value[0] = 0.5f;
		Color.Value[1] = 0.5f;
		Color.Value[2] = 0.5f;
		Color.Value[3] = 1.0f;
		sprintf(Color.Name, "Color %03d", ColorCode);
		sprintf(Color.SafeName, "Color_%03d", ColorCode);
	}

	gColorList.push_back(Color);
	return (int)gColorList.size() - 1;
}
