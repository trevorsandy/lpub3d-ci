#include "LDLPalette.h"
#include <string.h>
#include <stdio.h>
// LPub3D Mod - stud style
#include <math.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <TCFoundation/TCVector.h>
#include <TCFoundation/TCUserDefaults.h>
// LPub3D Mod End
#include <TCFoundation/mystring.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

#define GOLD_SCALE (255.0f / 240.0f * 2.0f)

static const TCByte transA = 110;

// LPub3D Mod - stud style
#define TC_SRGB_TO_LINEAR(v) (powf(v, 2.2f))
#define TC_LINEAR_TO_SRGB(v) (powf(v, 1.0f / 2.2f))
#define TC_LUM_FROM_SRGB(r,g,b) ((0.2126f * TC_SRGB_TO_LINEAR(r)) + (0.7152f * TC_SRGB_TO_LINEAR(g)) + (0.0722f * TC_SRGB_TO_LINEAR(b)))
#define TC_LUM_FROM_RGB(r,g,b) ((0.2126f * r) + (0.7152f * g) + (0.0722f * b))
#define TC_RGB_EPSILON (0.5f / 255.0f)
#define TC_RGB_TO_DEC(v) (v / 255.0f)

#define LPUB3D_COLOUR_HIGHLIGHT_PREFIX "110"
#define LPUB3D_COLOUR_NAME_PREFIX      "LPub3D_"

LDLColor LDLPalette::sm_studCylinderColor{ 27,42,52,255 };
LDLColor LDLPalette::sm_partEdgeColor{ 0,0,0,255 };
LDLColor LDLPalette::sm_blackEdgeColor{ 255,255,255,255 };
LDLColor LDLPalette::sm_darkEdgeColor{ 27,42,52,255 };
TCFloat  LDLPalette::sm_partEdgeContrast = 0.5f;
TCFloat  LDLPalette::sm_partColorValueLDIndex = 0.5f;
bool     LDLPalette::sm_partEdgeColorEnabled = true;
bool     LDLPalette::sm_blackEdgeColorEnabled = true;
bool     LDLPalette::sm_darkEdgeColorEnabled = true;
bool     LDLPalette::sm_automateEdgeColor = false;
int      LDLPalette::sm_studStyle = 0;

static TCVector TC_RGB2HSL(TCVector rgb)
{
	int Mi;
	TCFloat M, m, C, h, S, L; // h is H/60

	Mi = (rgb[0] >= rgb[1]) ? 0 : 1;
	Mi = (rgb[Mi] >= rgb[2]) ? Mi : 2;
	M = rgb[Mi];

	m = (rgb[0] < rgb[1]) ? rgb[0] : rgb[1];
	m = (m < rgb[2]) ? m : rgb[2];

	C = M - m;
	L = (M + m) / 2.0f;

	if (C < TC_RGB_EPSILON)  // C == 0.0
		h = 0.0f;
	else if (Mi == 0)        // M == R
		h = 0.0f + (rgb[1] - rgb[2]) / C;
	else if (Mi == 1)        // M == G
		h = 2.0f + (rgb[2] - rgb[0]) / C;
	else                     // M = B
		h = 4.0f + (rgb[0] - rgb[1]) / C;

	h = (h < 0.0) ? h + 6.0f : h;
	h = (h >= 6.0) ? h - 6.0f : h;

	S = ((L < (TC_RGB_EPSILON / 2.0f)) || (L > (1.0f - (TC_RGB_EPSILON / 2.0f))))
		? 0.0f : (2.0f * (M - L)) / (1.0f - fabs((2.0f * L) - 1.0f));

	return TCVector(h, S, L);
}

static TCVector TC_HSL2RGB(TCVector hSL)
{
	TCVector rgb;
	float h, S, L, C, X, m;

	h = hSL[0];
	S = hSL[1];
	L = hSL[2];

	C = (1.0f - fabs(2.0f * L - 1.0f)) * S;
	X = C * (1.0f - fabs(fmodf(h, 2.0f) - 1.0f));

	if (h < 1.0f)
		rgb = TCVector(C, X, 0.0f);
	else if (h < 2.0f)
		rgb = TCVector(X, C, 0.0f);
	else if (h < 3.0f)
		rgb = TCVector(0.0f, C, X);
	else if (h < 4.0f)
		rgb = TCVector(0.0f, X, C);
	else if (h < 5.0f)
		rgb = TCVector(X, 0.0f, C);
	else
		rgb = TCVector(C, 0.0f, X);

	m = L - C / 2.0f;

	rgb[0] += m;
	rgb[1] += m;
	rgb[2] += m;

	return rgb;
}

static LDLColor getAlgorithmicEdgeColor(const TCVector& Value, const float ValueLum, const float EdgeLum, const float Contrast, const float Saturation)
{
	float y1, yt;
	float y0 = ValueLum;
	float ye = EdgeLum;
	float cont = Contrast;
	float sat = Saturation;
	TCVector hSL, rgb1, rgbf;

	// Determine luma target
	if (ye < y0)
	{
		// Light base color
		yt = y0 - cont * y0;
	}
	else
	{
		// Dark base color
		yt = y0 + cont * (1.0f - y0);
	}

	// Get base color in hSL
	hSL = TC_RGB2HSL(Value);

	// Desaturate
	hSL[1] *= sat;

	// Adjusted color to RGB
	rgb1 = TC_HSL2RGB(TCVector(hSL[0], hSL[1], 0.5f));

	// Fix adjusted color luma to target value
	y1 = TC_LUM_FROM_RGB(rgb1[0], rgb1[1], rgb1[2]);
	if (yt < y1)
	{
		// Make darker via scaling
		rgbf = (yt / y1) * rgb1;
	}
	else
	{
		// Make lighter via scaling anti-color
		rgbf = TCVector(1.0f, 1.0f, 1.0f) - rgb1;
		rgbf *= (1.0f - yt) / (1.0f - y1);
		rgbf = TCVector(1.0f, 1.0f, 1.0f) - rgbf;
	}

	TCVector rgb = TCVector(TC_LINEAR_TO_SRGB(rgbf[0]), TC_LINEAR_TO_SRGB(rgbf[1]), TC_LINEAR_TO_SRGB(rgbf[2])) *= 255;

	return LDLColor{ (TCByte)rgb[0], (TCByte)rgb[1], (TCByte)rgb[2], 255};
}
// LPub3D Mod End

LDLPalette *LDLPalette::sm_defaultPalette = NULL;
TCByte LDLPalette::sm_transA = transA;
LDLPalette::LDLPaletteCleanup LDLPalette::sm_cleanup;

LDLPalette::LDLPaletteCleanup::~LDLPaletteCleanup(void)
{
	TCObject::release(LDLPalette::sm_defaultPalette);
	LDLPalette::sm_defaultPalette = NULL;
}

static const int standardColorSize = 5;

static const char standardColorNames[][64] =
{
	"Black",
	"Blue",
	"Green",
	"Teal",
	"Red",
	"Dark Pink",
	"Brown",
	"Gray",
	"Dark Gray",
	"Light Blue",
	"Light Green",
	"Turquoise",
	"Light Red",
	"Pink",
	"Yellow",
	"White",
	"",
	"Mint Green",
	"Light Yellow",
	"Tan",
	"Light Purple",
	"Glow in the dark",
	"Purple",
	"Violet Blue",
	"",
	"Orange",
	"Magenta",
	"Yellow Green",
	"Dark Tan",
	"",
	"",
	"",
	"Trans Black",
	"Trans Blue",
	"Trans Green",
	"Trans Dark Cyan",
	"Trans Red",
	"Trans Purple",
	"Trans Brown",
	"Trans Light Gray",
	"Trans Gray",
	"Trans Light Cyan",
	"Trans Yellow Green",
	"Trans Cyan",
	"Trans Light Red",
	"Trans Pink",
	"Trans Yellow",
	"Clear",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Trans Orange"
};

static const TCULong standardColors[] =
{
//	R		G		B		A		Edge	   #	Description
	51,		51,		51,		255,	8,		// 0	Black
	0,		51,		178,	255,	9,		// 1	Blue
	0,		127,	51,		255,	10,		// 2	Green
	0,		181,	166,	255,	11,		// 3	Teal
	204,	0,		0,		255,	12,		// 4	Red
	255,	51,		153,	255,	13,		// 5	Dark Pink
	102,	51,		0,		255,	8,		// 6	Brown
	153,	153,	153,	255,	8,		// 7	Gray
	102,	102,	88,		255,	0,		// 8	Dark Gray
	0,		128,	255,	255,	1,		// 9	Light Blue
	51,		255,	102,	255,	2,		// 10	Light Green
	171,	253,	249,	255,	3,		// 11	Turquoise
	255,	0,		0,		255,	4,		// 12	Light Red
	255,	176,	204,	255,	5,		// 13	Pink
	255,	229,	0,		255,	8,		// 14	Yellow
	255,	255,	255,	255,	8,		// 15	White
	0,		0,		0,		0,		255,	// 16	***UNDEFINED***
	102,	240,	153,	255,	0,		// 17	Mint Green
	255,	255,	128,	255,	0,		// 18	Light Yellow
	204,	170,	102,	255,	0,		// 19	Tan
	224,	204,	240,	255,	0,		// 20	Light Purple
	224,	255,	176,	255,	0x47c07c0,	// 21	Glow in the dark
	153,	51,		153,	255,	0,		// 22	Purple
	76,		0,		204,	255,	0,		// 23	Violet Blue
	0,		0,		0,		0,		255,	// 24	***UNDEFINED***
	255,	102,	0,		255,	0x4000000,	// 25	Orange
	255,	51,		153,	255,	0x4000000,	// 26	Magenta
	173,	221,	80,		255,	0,		// 27	Yellow Green
	197,	151,	80,		255,	0,		// 28	Dark Tan
	0,		0,		0,		0,		255,	// 29	***UNDEFINED***
	0,		0,		0,		0,		255,	// 30	***UNDEFINED***
	0,		0,		0,		0,		255,	// 31	***UNDEFINED***
	102,	102,	102,	transA,	40,		// 32	Trans Black
	0,		0,		153,	transA,	41,		// 33	Trans Blue
//	0,		0,		153,	transA,	0x4026026,	// 33	Trans Blue
	0,		80,		24,		transA,	42,		// 34	Trans Green
//	0,		80,		24,		transA,	0x4042042,	// 34	Trans Green
	0,		181,	166,	transA,	43,		// 35	Trans Dark Cyan
	204,	0,		0,		transA,	44,		// 36	Trans Red
//	204,	0,		0,		transA,	0x4601601,	// 36	Trans Red
	255,	51,		153,	transA,	45,		// 37	Trans Purple
	102,	51,		0,		transA,	32,		// 38	Trans Brown
	153,	153,	153,	transA,	40,		// 39	Trans Light Gray
	102,	102,	88,		transA,	32,		// 40	Trans Gray
	153,	192,	240,	transA,	33,		// 41	Trans Light Cyan
	204,	255,	0,		transA,	34,		// 42	Trans Yellow Green
	171,	253,	249,	transA,	35,		// 43	Trans Cyan
	255,	0,		0,		transA,	36,		// 44	Trans Light Red
	255,	176,	204,	transA,	37,		// 45	Trans Pink
	240,	196,	0,		transA,	40,		// 46	Trans Yellow
	255,	255,	255,	transA,	40,		// 47	Clear
	0,		0,		0,		0,		255,	// 48	***UNDEFINED***
	0,		0,		0,		0,		255,	// 49	***UNDEFINED***
	0,		0,		0,		0,		255,	// 50	***UNDEFINED***
	0,		0,		0,		0,		255,	// 51	***UNDEFINED***
	0,		0,		0,		0,		255,	// 52	***UNDEFINED***
	0,		0,		0,		0,		255,	// 53	***UNDEFINED***
	0,		0,		0,		0,		255,	// 54	***UNDEFINED***
	0,		0,		0,		0,		255,	// 55	***UNDEFINED***
	0,		0,		0,		0,		255,	// 56	***UNDEFINED***
	255,	102,	0,		transA,	40,		// 57	Trans Orange
};

LDLPalette::LDLPalette(void)
	:m_customColors(new TCTypedObjectArray<CustomColor>)
{
	init();
}

LDLPalette::LDLPalette(const LDLPalette &other)
	:m_customColors(new TCTypedObjectArray<CustomColor>)
{
	memcpy(m_colors, other.m_colors, sizeof(m_colors));
}

LDLPalette::~LDLPalette(void)
{
}

void LDLPalette::dealloc(void)
{
	TCObject::release(m_customColors);
	TCObject::dealloc();
}

void LDLPalette::initSpecularAndShininess(LDLColorInfo &color)
{
	color.specular[0] = -100.0;
	color.specular[1] = -100.0;
	color.specular[2] = -100.0;
	color.specular[3] = -100.0;
	color.shininess = -100.0;
}

void LDLPalette::init(void)
{
	int i;

	for (i = 0; i < 512; i++)
	{
		initColorInfo(m_colors[i], 0, 0, 0, 0);
	}
	initStandardColors();
	initDitherColors();
	initOtherColors();
	// LPub3D Mod - stud style
	initStudStyleSettings();
	// LPub3D Mod End
}

void LDLPalette::reset(void)
{
	m_customColors->removeAll();
	init();
}

void LDLPalette::initStandardColors(void)
{
	int i;
	int count = sizeof(standardColors) / sizeof(standardColors[0]) /
		standardColorSize;

	for (i = 0; i < count; i++)
	{
		int ofs = i * standardColorSize;

		strcpy(m_colors[i].name, standardColorNames[i]);
		m_namesMap[m_colors[i].name] = i;
		m_colors[i].color.r = (TCByte)standardColors[ofs];
		m_colors[i].color.g = (TCByte)standardColors[ofs + 1];
		m_colors[i].color.b = (TCByte)standardColors[ofs + 2];
		m_colors[i].color.a = (TCByte)standardColors[ofs + 3];
		m_colors[i].ditherColor = m_colors[i].color;
		m_colors[i].edgeColorNumber = (int)standardColors[ofs + 4];
	}
	for (i = 0; i < 256; i++)
	{
		LDLColorInfo &colorInfo = m_colors[i];

		if (colorInfo.color.r == 0 && colorInfo.color.g == 0 &&
			colorInfo.color.b == 0 && colorInfo.color.a == 0 &&
			colorInfo.edgeColorNumber == 255)
		{
			if (i & 0x20)
			{
				colorInfo = m_colors[i % 16 + 32];
			}
			else
			{
				colorInfo = m_colors[i % 16];
			}
		}
	}
}

void LDLPalette::initDitherColors(void)
{
	int i;

	for (i = 256; i < 512; i++)
	{
		int colorNumber1 = (i - 256) / 16;
		int colorNumber2 = (i - 256) % 16;
		int ofs1 = colorNumber1 * standardColorSize;
		int ofs2 = colorNumber2 * standardColorSize;

		m_colors[i].color.r = (TCByte)standardColors[ofs1];
		m_colors[i].color.g = (TCByte)standardColors[ofs1 + 1];
		m_colors[i].color.b = (TCByte)standardColors[ofs1 + 2];
		m_colors[i].color.a = (TCByte)standardColors[ofs1 + 3];
		m_colors[i].ditherColor.r = (TCByte)standardColors[ofs2];
		m_colors[i].ditherColor.g = (TCByte)standardColors[ofs2 + 1];
		m_colors[i].ditherColor.b = (TCByte)standardColors[ofs2 + 2];
		m_colors[i].ditherColor.a = (TCByte)standardColors[ofs2 + 3];
		m_colors[i].edgeColorNumber = 0;
	}
}

void LDLPalette::initOtherColor(int index, TCByte r, TCByte g, TCByte b,
								TCByte a)
{
	m_colors[index].color.r = r;
	m_colors[index].color.g = g;
	m_colors[index].color.b = b;
	m_colors[index].color.a = a;
	m_colors[index].ditherColor = m_colors[index].color;
	m_colors[index].edgeColorNumber = 0;
}

void LDLPalette::initSpecular(LDLColorInfo &colorInfo, float sr, float sg,
							  float sb, float sa, float shininess)
{
	colorInfo.specular[0] = sr;
	colorInfo.specular[1] = sg;
	colorInfo.specular[2] = sb;
	colorInfo.specular[3] = sa;
	colorInfo.shininess = shininess;
}

void LDLPalette::initSpecular(int index, float sr, float sg, float sb, float sa,
							  float shininess)
{
	initSpecular(m_colors[index], sr, sg, sb, sa, shininess);
}

void LDLPalette::initRubber(int index)
{
	initSpecular(index, 0.075f, 0.075f, 0.075f, 1.0f, 15.0f);
	m_colors[index].rubber = true;
}

void LDLPalette::initOtherColors(void)
{
	initOtherColor(382,	204,	170,	102);	// Tan
	initOtherColor(334,	240,	176,	51);	// Gold
	initOtherColor(383,	204,	204,	204);	// Chrome
	initOtherColor(494,	204,	204,	204);	// Electrical Contacts

	// Black rubber
	initRubber(256);
	m_colors[256].edgeColorNumber = 8;

	// Blue rubber
	initRubber(273);
	// Red rubber
	initRubber(324);
	// Gray rubber
	initRubber(375);
	// White rubber
	initRubber(511);

	// Gold
	initSpecular(334,
		240.0f / 255.0f * GOLD_SCALE,
		176.0f / 255.0f * GOLD_SCALE,
		51.0f / 255.0f * GOLD_SCALE,
		1.0f, 5.0f);
	m_colors[334].chrome = true;

	// Chrome
	initSpecular(383, 0.9f, 1.2f, 1.5f, 1.0f, 5.0f);
	m_colors[383].chrome = true;

	// Electrical contacts
	initSpecular(494, 0.9f, 0.9f, 1.5f, 1.0f, 5.0f);
}

// LPub3D Mod - stud style
int LDLPalette::getEdgeColorNumberFromRGB(const LDLColor& color)
{
	char hexColor[16];
	int edgeColorNumber;
	snprintf(hexColor, sizeof hexColor, "%02x%02x%02x", (int)color.r, (int)color.g, (int)color.b);
	if (sscanf(hexColor, "%x", &edgeColorNumber) == 1)
	{
		edgeColorNumber &= 0xFFFFFF;
		edgeColorNumber |= 0x2000000; // Encode EDGE as extended RGB color.
		return edgeColorNumber;
	}
	debugPrintf("Failed to get extended HEX from RGB %s\n", hexColor);
	return 0;
}

void LDLPalette::initStudStyleSettings()
{
	sm_studStyle = (int)TCUserDefaults::longForKey(STUD_STYLE_KEY, sm_studStyle);
	sm_automateEdgeColor = TCUserDefaults::boolForKey(AUTOMATE_EDGE_COLOR_KEY, sm_automateEdgeColor);
	sm_partEdgeContrast = TCUserDefaults::floatForKey(PART_EDGE_CONTRAST_KEY, sm_partEdgeContrast);
	sm_partColorValueLDIndex = TCUserDefaults::floatForKey(PART_COLOR_VALUE_LD_INDEX_KEY, sm_partColorValueLDIndex);
	sm_partEdgeColorEnabled = TCUserDefaults::boolForKey(PART_EDGE_COLOR_ENABLED_KEY, sm_partEdgeColorEnabled);
	sm_blackEdgeColorEnabled = TCUserDefaults::boolForKey(BLACK_EDGE_COLOR_ENABLED_KEY, sm_blackEdgeColorEnabled);
	sm_darkEdgeColorEnabled = TCUserDefaults::boolForKey(DARK_EDGE_COLOR_ENABLED_KEY, sm_darkEdgeColorEnabled);

	if (sm_studStyle < 6 && !sm_automateEdgeColor)
		return;
	
	int r, g, b, a;
	std::string rgbaString = TCUserDefaults::stringForKey(STUD_CYLINDER_COLOR_KEY, "27,42,52,255");
	if (sscanf(rgbaString.c_str(), "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
		sm_studCylinderColor = LDLColor{ (TCByte)r, (TCByte)g, (TCByte)b, (TCByte)a };
	rgbaString = TCUserDefaults::stringForKey(PART_EDGE_COLOR_KEY, "0,0,0,255");
	if (sscanf(rgbaString.c_str(), "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
		sm_partEdgeColor = LDLColor{ (TCByte)r, (TCByte)g, (TCByte)b, (TCByte)a };
	rgbaString = TCUserDefaults::stringForKey(BLACK_EDGE_COLOR_KEY, "255,255,255,255");
	if (sscanf(rgbaString.c_str(), "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
		sm_blackEdgeColor = LDLColor{ (TCByte)r, (TCByte)g, (TCByte)b, (TCByte)a };
	rgbaString = TCUserDefaults::stringForKey(DARK_EDGE_COLOR_KEY, "27,42,52,255");
	if (sscanf(rgbaString.c_str(), "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
		sm_darkEdgeColor = LDLColor{ (TCByte)r, (TCByte)g, (TCByte)b, (TCByte)a };

	int edgeColorNumber = getEdgeColorNumberFromRGB(sm_partEdgeColor);
	LDLColorInfo* colorInfo = updateColor(4242, sm_studCylinderColor, sm_studCylinderColor,
		edgeColorNumber, (-25500.0f / 255.0f));
	if (colorInfo)
	{
		const char *name = "Stud Cylinder Colour";
		strncpy(colorInfo->name, name, sizeof(colorInfo->name));
		colorInfo->name[sizeof(colorInfo->name) - 1] = 0;
		m_namesMap[name] = 4242;
		colorInfo->rubber = false;
		colorInfo->chrome = false;
	}
	else
		debugPrintf("Error creating Stud Cylinder Colour (%d)\n", 4242);
}

int LDLPalette::getStudStyleOrAutoEdgeColor(int colorNumber)
{
	LDLColorInfo colorInfo = getAnyColorInfo(colorNumber);

	char colorNumberStr[32];
	snprintf(colorNumberStr, sizeof(colorNumberStr), "%d", colorNumber);
	bool isLPubHighlightColor = strncmp(LPUB3D_COLOUR_HIGHLIGHT_PREFIX, colorNumberStr, 3) == 0;
	isLPubHighlightColor &= strncmp(LPUB3D_COLOUR_NAME_PREFIX, colorInfo.name, 7) == 0;

	if (isLPubHighlightColor)
		return getEdgeColorNumber(colorNumber, isLPubHighlightColor);

	TCVector value(TC_RGB_TO_DEC((int)colorInfo.color.r), TC_RGB_TO_DEC((int)colorInfo.color.g), TC_RGB_TO_DEC((int)colorInfo.color.b));

	const float valueLuminescence = TC_LUM_FROM_SRGB(value[0], value[1], value[2]);
	const float lightDarkControl = sm_automateEdgeColor ? sm_partColorValueLDIndex : TC_SRGB_TO_LINEAR(sm_partColorValueLDIndex);

	if (sm_automateEdgeColor)
	{
		if (colorInfo.adjusted)
		{
			return colorInfo.edgeColorNumber;
		} 
		else
		{
			int r, g, b, a;
			getRGBA(colorInfo.edgeColorNumber, r, g, b, a);
			const float edgeLuminescence = TC_LUM_FROM_SRGB(TC_RGB_TO_DEC(r), TC_RGB_TO_DEC(g), TC_RGB_TO_DEC(b));

			LDLColor algEdgeColor = getAlgorithmicEdgeColor(value, valueLuminescence, edgeLuminescence, sm_partEdgeContrast, lightDarkControl);
			int edgeColorNumber = getEdgeColorNumberFromRGB(algEdgeColor);

			LDLColorInfo* adjustColorInfo;
			adjustColorInfo = updateColor(colorNumber, colorInfo.color, colorInfo.ditherColor, edgeColorNumber);
			adjustColorInfo->adjusted = true;
			return edgeColorNumber;
		}
	}
	else
	{
		if (sm_blackEdgeColorEnabled && colorNumber == 0)
			return getEdgeColorNumberFromRGB(sm_blackEdgeColor);
		else if (sm_darkEdgeColorEnabled && colorNumber != 4242 &&
			valueLuminescence < lightDarkControl)
			return getEdgeColorNumberFromRGB(sm_darkEdgeColor);
		else if (sm_partEdgeColorEnabled)
			return getEdgeColorNumberFromRGB(sm_partEdgeColor);
		else
			return getEdgeColorNumber(colorNumber, isLPubHighlightColor);
	}

	debugPrintf("Error creating edge color number for color: %d\n",
		colorNumber);

	return 0;
}

int LDLPalette::getEdgeColorNumber(int colorNumber, bool isHighlightColor)
{

	if (!isHighlightColor && sm_studStyle > 5 || sm_automateEdgeColor)
	{
		return getStudStyleOrAutoEdgeColor(colorNumber);
	}
	else
	{
		if (colorNumber < 512 && colorNumber >= 0)
		{
			return m_colors[colorNumber].edgeColorNumber;
		}
		else
		{
			LDLColorInfo colorInfo;

			if (getCustomColorInfo(colorNumber, colorInfo))
			{
				return colorInfo.edgeColorNumber;
			}
			else
			{
				return 0;
			}
		}
	}
}
// LPub3D Mod End

bool LDLPalette::getCustomColorInfo(int colorNumber, LDLColorInfo &colorInfo)
{
	size_t i;
	size_t count = m_customColors->getCount();

	for (i = 0; i < count; i++)
	{
		CustomColor *customColor = (*m_customColors)[i];

		if (customColor->colorNumber == colorNumber)
		{
			colorInfo = customColor->colorInfo;
			return true;
		}
	}
	return false;
}

bool LDLPalette::getCustomColorRGBA(int colorNumber, int &r, int &g, int &b,
									int &a)
{
	LDLColorInfo colorInfo;

	if (getCustomColorInfo(colorNumber, colorInfo))
	{
		getRGBA(colorInfo, r, g, b, a);
		return true;
	}
	return false;
}

int LDLPalette::getBlendedColorComponent(TCULong c1, TCULong c2, TCULong a1,
										 TCULong a2)
{
	if (a1 == 0 && a2 == 0)
	{
		return 0;
	}
	else if (c1 == c2)
	{
		return c1;
	}
	else
	{
		float af1 = (float)a1 / 255.0f;
		float af2 = (float)a2 / 255.0f;
		float cf1 = (float)c1 * af1;
		float cf2 = (float)c2 * af2;

		return (int)((cf1 + cf2) / (af1 + af2));
	}
}

void LDLPalette::getRGBA(const LDLColorInfo &colorInfo, int &r, int &g, int &b,
						 int &a)
{
	r = getBlendedColorComponent(colorInfo.color.r, colorInfo.ditherColor.r,
		colorInfo.color.a, colorInfo.ditherColor.a);
	g = getBlendedColorComponent(colorInfo.color.g, colorInfo.ditherColor.g,
		colorInfo.color.a, colorInfo.ditherColor.a);
	b = getBlendedColorComponent(colorInfo.color.b, colorInfo.ditherColor.b,
		colorInfo.color.a, colorInfo.ditherColor.a);
	a = ((int)colorInfo.color.a + (int)colorInfo.ditherColor.a) / 2;
}

void LDLPalette::initColorInfo(LDLColorInfo &colorInfo, int r, int g, int b,
							   int a)
{
	colorInfo.name[0] = 0;
	colorInfo.color.r = (TCByte)r;
	colorInfo.color.g = (TCByte)g;
	colorInfo.color.b = (TCByte)b;
	colorInfo.color.a = (TCByte)a;
	colorInfo.ditherColor.r = (TCByte)r;
	colorInfo.ditherColor.g = (TCByte)g;
	colorInfo.ditherColor.b = (TCByte)b;
	colorInfo.ditherColor.a = (TCByte)a;
	colorInfo.edgeColorNumber = 255;
	colorInfo.luminance = -100.0f;
	colorInfo.chrome = false;
	colorInfo.rubber = false;
	// LPub3D Mod - stud style
	colorInfo.adjusted = false;
	// LPub3D Mod End
	initSpecularAndShininess(colorInfo);
}

bool LDLPalette::hasSpecular(int colorNumber)
{
	LDLColorInfo colorInfo = getAnyColorInfo(colorNumber);

	if (colorInfo.specular[0] != -100.0f || colorInfo.specular[1] != -100.0f ||
		colorInfo.specular[2] != -100.0f || colorInfo.specular[3] != -100.0f)
	{
		return true;
	}
	return false;
}

bool LDLPalette::hasShininess(int colorNumber)
{
	return getAnyColorInfo(colorNumber).shininess != -100.0f;
}

bool LDLPalette::hasLuminance(int colorNumber)
{
	return getAnyColorInfo(colorNumber).luminance != -100.0f;
}

LDLColorInfo LDLPalette::getAnyColorInfo(int colorNumber)
{
	LDLColorInfo colorInfo;
	int r, g, b, a;

	// Default color is orange-ish opaque.
	r = 255;
	g = 128;
	b = 0;
	a = 255;
	if (colorNumber < 512 && colorNumber >= 0)
	{
		colorInfo = m_colors[colorNumber];
		if (colorInfo.color.r != 0 || colorInfo.color.g != 0 ||
			colorInfo.color.b != 0 || colorInfo.color.a != 0 ||
			colorInfo.edgeColorNumber != 255)
		{
			return colorInfo;
		}
		else if (colorNumber != 16 && colorNumber != 24)
		{
			debugPrintf("Unknown color: %d\n", colorNumber);
		}
	}
	else if (getCustomColorInfo(colorNumber, colorInfo))
	{
		return colorInfo;
	}
	else
	{
		if (colorNumber >= 0x2000000 && colorNumber < 0x4000000)
		{
			// 0x2RRGGBB = opaque RGB
			// 0x3RRGGBB = transparent RGB
			r = (colorNumber & 0xFF0000) >> 16;
			g = (colorNumber & 0xFF00) >> 8;
			b = (colorNumber & 0xFF);
			if (colorNumber >= 0x3000000)
			{
				a = transA;
			}
		}
		else if (colorNumber >= 0x4000000 && colorNumber < 0x5000000)
		{
			// 0x4RGBRGB = opaque dither
			r = (((colorNumber & 0xF00000) >> 20) * 17 +
				((colorNumber & 0xF00) >> 8) * 17) / 2;
			g = (((colorNumber & 0xF0000) >> 16) * 17 +
				((colorNumber & 0xF0) >> 4) * 17) / 2;
			b = (((colorNumber & 0xF000) >> 12) * 17 +
				(colorNumber & 0xF) * 17) / 2;
		}
		else if (colorNumber >= 0x5000000 && colorNumber < 0x6000000)
		{
			// 0x5RGBxxx = transparent dither (xxx is ignored)
			r = ((colorNumber & 0xF00000) >> 20) * 17;
			g = ((colorNumber & 0xF0000) >> 16) * 17;
			b = ((colorNumber & 0xF000) >> 12) * 17;
			if (colorNumber >= 0x6000000 && colorNumber < 0x7000000)
			{
				a = transA;
			}
		}
		else if (colorNumber >= 0x6000000 && colorNumber < 0x7000000)
		{
			// 0x6xxxRGB = transparent dither (xxx is ignored)
			r = ((colorNumber & 0xF00) >> 8) * 17;
			g = ((colorNumber & 0xF0) >> 4) * 17;
			b = (colorNumber & 0xF) * 17;
			a = transA;
		}
		else if (colorNumber >= 0x7000000 && colorNumber < 0x8000000)
		{
			// 0x7xxxxxx = invisible
			r = g = b = a = 0;
		}
	}
	initColorInfo(colorInfo, r, g, b, a);
	return colorInfo;
}

void LDLPalette::getRGBA(int colorNumber, int& r, int& g, int& b, int& a)
{
	LDLColorInfo colorInfo = getAnyColorInfo(colorNumber);
	getRGBA(colorInfo, r, g, b, a);
}

bool LDLPalette::isColorComment(const char *comment)
{
	if (stringHasCaseInsensitivePrefix(comment, "0 color ") ||
		stringHasCaseInsensitivePrefix(comment, "0 !colour "))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LDLPalette::parseColorComment(const char *comment)
{
	if (stringHasCaseInsensitivePrefix(comment, "0 color "))
	{
		return parseLDLiteColorComment(comment);
	}
	else if (stringHasCaseInsensitivePrefix(comment, "0 !colour "))
	{
		return parseLDrawOrgColorComment(comment);
	}
	else
	{
		return false;
	}
}

bool LDLPalette::parseLDrawOrgColorComment(const char *comment)
{
	const char *itemSpot = strcasestr(comment, "CODE ");
	int colorNumber;
	int colorValue;
	int edgeColorNumber;
	int alpha = 255;
	int luminance = -25500;
	LDLColor color;
	LDLColor ditherColor;
	LDLColorInfo *colorInfo;
	bool chrome = false;
	bool rubber = false;
	bool metal = false;
	char name[1024];

	strncpy(name, &comment[10], sizeof(name));
	name[sizeof(name) - 1] = 0;
	stripLeadingWhitespace(name);
	if (strchr(name, ' '))
	{
		*strchr(name, ' ') = 0;
	}
	if (strchr(name, '\t'))
	{
		*strchr(name, '\t') = 0;
	}
	replaceStringCharacter(name, '_', ' ');
	if (!itemSpot)
	{
		debugPrintf("Couldn't find color CODE in color meta-comment:\n%s\n",
			comment);
		return false;
	}
	if (sscanf(itemSpot + 5, "%d", &colorNumber) != 1)
	{
		debugPrintf("Error parsing color CODE in color meta-comment:\n%s\n",
			comment);
		return false;
	}
	itemSpot = strcasestr(comment, "VALUE ");
	if (!itemSpot)
	{
		debugPrintf("Couldn't find color VALUE in color meta-comment:\n%s\n",
			comment);
		return false;
	}
	if (sscanf(itemSpot + 6, "#%x", &colorValue) != 1 &&
		sscanf(itemSpot + 6, "0x%x", &colorValue) != 1)
	{
		debugPrintf("Error parsing color VALUE in color meta-comment:\n%s\n",
			comment);
		return false;
	}
	itemSpot = strcasestr(&comment[10 + strlen(name)], "EDGE ");
	if (!itemSpot)
	{
		debugPrintf("Couldn't find color EDGE in color meta-comment:\n%s\n",
			comment);
		return false;
	}
	if (sscanf(itemSpot + 5, "#%x", &edgeColorNumber) != 1 &&
		sscanf(itemSpot + 5, "0x%x", &edgeColorNumber) != 1)
	{
		if (sscanf(itemSpot + 5, "%d", &edgeColorNumber) != 1)
		{
			debugPrintf("Error parsing color VALUE in color "
				"meta-comment:\n%s\n", comment);
			return false;
		}
	}
	else
	{
		edgeColorNumber &= 0xFFFFFF;
		edgeColorNumber |= 0x2000000;
	}
	itemSpot = strcasestr(comment, "ALPHA ");
	if (itemSpot)
	{
		if (sscanf(itemSpot + 6, "%d", &alpha) == 1)
		{
			// This is a little odd, but we want 128 to map to our standard
			// transparent alpha (transA), anything below transA to smoothly map
			// from 0 to transA, and anything above transA to smoothly map from
			// transA to 255.
			if (alpha == 128)
			{
				alpha = transA;
			}
			else if (alpha < 128)
			{
				alpha = alpha * transA / 128;
			}
			else
			{
				alpha = transA + (alpha - 128) * (255 - transA) / 127;
			}
		}
		else
		{
			debugPrintf("Error parsing color ALPHA in color meta-comment:\n"
				"%s\n", comment);
			return false;
		}
	}
	itemSpot = strcasestr(comment, "LUMINANCE ");
	if (itemSpot)
	{
		if (sscanf(itemSpot + 10, "%d", &luminance) != 1)
		{
			debugPrintf("Error parsing color LUMINANCE in color meta-comment:\n"
				"%s\n", comment);
			return false;
		}
	}
	if (strcasestr(comment, "CHROME"))
	{
		chrome = true;
	}
	if (strcasestr(comment, "RUBBER"))
	{
		rubber = true;
	}
	if (strcasestr(comment, "METAL"))
	{
		metal = true;
	}
	color.r = (TCByte)(colorValue >> 16);
	color.g = (TCByte)(colorValue >> 8);
	color.b = (TCByte)colorValue;
	color.a = (TCByte)alpha;
	ditherColor = color;
	colorInfo = updateColor(colorNumber, color, ditherColor, edgeColorNumber,
		(float)luminance / 255.0f);
	if (colorInfo)
	{
		strncpy(colorInfo->name, name, sizeof(colorInfo->name));
		colorInfo->name[sizeof(colorInfo->name) - 1] = 0;
		m_namesMap[name] = colorNumber;
		if (rubber)
		{
			initSpecular(*colorInfo, 0.075f, 0.075f, 0.075f, -100.0f, 15.0f);
			colorInfo->rubber = true;
		}
		else if (chrome)
		{
			initSpecular(*colorInfo, 0.9f, 1.2f, 1.5f, -100.0f, 5.0f);
			colorInfo->chrome = true;
		}
		else if (metal)
		{
			initSpecular(*colorInfo, 0.9f, 0.9f, 1.5f, -100.0f, 5.0f);
		}
	}
	return true;
}

bool LDLPalette::parseLDLiteColorComment(const char *comment)
{
	int colorNumber;
	int edgeColorNumber;
	int r, g, b, a;
	int dr, dg, db, da;

	if (sscanf(comment + 8, "%i %*s %i %i %i %i %i %i %i %i %i", &colorNumber,
		&edgeColorNumber, &r, &g, &b, &a, &dr, &dg, &db, &da) == 10)
	{
		LDLColor color;
		LDLColor ditherColor;

		color.r = (TCByte)r;
		color.g = (TCByte)g;
		color.b = (TCByte)b;
		color.a = (TCByte)a;
		ditherColor.r = (TCByte)dr;
		ditherColor.g = (TCByte)dg;
		ditherColor.b = (TCByte)db;
		ditherColor.a = (TCByte)da;
		updateColor(colorNumber, color, ditherColor, edgeColorNumber);
		return true;
	}
	else
	{
		return false;
	}
}

LDLColorInfo *LDLPalette::updateColor(int colorNumber, const LDLColor &color,
									  const LDLColor &ditherColor,
									  int edgeColorNumber, float luminance)
{
	LDLColorInfo *colorInfo;

	if (colorNumber < 512 && colorNumber >= 0)
	{
		colorInfo = m_colors + colorNumber;
	}
	else
	{
		// A custom color was requested, but it won't fit into our main
		// color array.  So add a new item in the m_customColors array, and
		// it will be noticed during look-up.
		CustomColor *customColor = new CustomColor;

		customColor->colorNumber = colorNumber;
		colorInfo = &customColor->colorInfo;
		// LPub3D Mod - stud style
		colorInfo->adjusted = false;
		// LPub3D Mod End
		m_customColors->addObject(customColor);
		customColor->release();
	}
	colorInfo->color = color;
	colorInfo->ditherColor = ditherColor;
	colorInfo->edgeColorNumber = edgeColorNumber;
	colorInfo->luminance = luminance;
	initSpecularAndShininess(*colorInfo);
	return colorInfo;
}

int LDLPalette::getColorNumberForName(const char *name) const
{
	std::string mangledName = name;
	replaceStringCharacter(&mangledName[0], '_', ' ');
	CIStringIntMap::const_iterator it = m_namesMap.find(mangledName);

	if (it == m_namesMap.end())
	{
		return -1;
	}
	else
	{
		return it->second;
	}
}

int LDLPalette::getColorNumberForRGB(TCByte r, TCByte g, TCByte b,
									 bool transparent)
{
	int color;

	if (r != 255 || g != 128 || b != 0)
	{
		int i;

		for (i = 0; i <= 27; i++)
		{
			if (isColorNumberRGB(i, r, g, b))
			{
				if (transparent)
				{
					return i + 32;
				}
				else
				{
					return i;
				}
			}
		}
	}
	color = ((int)r << 16) | ((int)g << 8) | (int)b;
	if (transparent)
	{
		return 0x3000000 | color;
	}
	else
	{
		return 0x2000000 | color;
	}
}

bool LDLPalette::isColorNumberRGB(int colorNumber, TCByte r, TCByte g, TCByte b)
{
	int pr, pg, pb, pa;

	if (colorNumber == 16 || colorNumber == 24)
	{
		return false;
	}
	getRGBA(colorNumber, pr, pg, pb, pa);
	if (pr == r && pg == g && pb == b)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// NOTE: static function.
LDLPalette *LDLPalette::getDefaultPalette(void)
{
	if (!sm_defaultPalette)
	{
		sm_defaultPalette = new LDLPalette;
	}
	return sm_defaultPalette;
}

// NOTE: static function.
void LDLPalette::getDefaultRGBA(int colorNumber, int &r, int &g, int &b, int &a)
{
	getDefaultPalette()->getRGBA(colorNumber, r, g, b, a);
}

// NOTE: static function.
TCULong LDLPalette::colorForRGBA(int r, int g, int b, int a)
{
	return (r & 0xFF) << 24 | (g & 0xFF) << 16 | (b & 0xFF) << 8 | (a & 0xFF);
}

// NOTE: static function.
int LDLPalette::colorNumberForPackedRGBA(TCULong color)
{
	return colorNumberForRGBA(color >> 24, (color >> 16) & 0xFF,
		(color >> 8) & 0xFF, color & 0xFF);
}

// NOTE: static function.
int LDLPalette::colorNumberForRGBA(int r, int g, int b, int a)
{
	int color = ((int)r << 16) | ((int)g << 8) | (int)b;

	if (a == 255)
	{
		return 0x2000000 | color;
	}
	else
	{
		return 0x3000000 | color;
	}
}
