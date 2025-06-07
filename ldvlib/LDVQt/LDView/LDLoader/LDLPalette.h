#ifndef __LDLPALETTE_H__
#define __LDLPALETTE_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <TCFoundation/TCStlIncludes.h>

typedef std::map<std::string, int, less_no_case> CIStringIntMap;

typedef struct
{
	TCByte r, g, b, a;
} LDLColor;

typedef struct
{
	char name[64];
	LDLColor color;
	LDLColor ditherColor;
	int edgeColorNumber;
	float specular[4];
	float shininess;
	float luminance;
	bool chrome;
	bool rubber;
	// LPub3D Mod - stud style
	bool adjusted;
	// LPub3D Mod End
} LDLColorInfo;

class LDLPalette : public TCObject
{
public:
	LDLPalette(void);
	LDLPalette(const LDLPalette &other);
	void reset(void);
	void getRGBA(int colorNumber, int &r, int &g, int &b, int &a);
	void getRGBA(const LDLColorInfo &colorInfo, int &r, int &g, int &b, int &a);
	bool hasSpecular(int colorNumber);
	bool hasShininess(int colorNumber);
	bool hasLuminance(int colorNumber);
	// LPub3D Mod - stud style
	int getEdgeColorNumber(int colorNumber, bool = false);
	// LPub3D Mod End
	bool isColorComment(const char *comment);
	bool parseColorComment(const char *comment);
	LDLColorInfo getAnyColorInfo(int colorNumber);
	LDLColorInfo &getColorInfo(int index) { return m_colors[index]; }
	virtual int getColorNumberForRGB(TCByte r, TCByte g, TCByte b,
		bool transparent);
	int getColorNumberForName(const char *name) const;

	static void getDefaultRGBA(int colorNumber, int &r, int &g, int &b, int &a);
	static LDLPalette *getDefaultPalette(void);
	static TCULong colorForRGBA(int r, int g, int b, int a);
	static int colorNumberForRGBA(int r, int g, int b, int a);
	static int colorNumberForPackedRGBA(TCULong color);
	static TCByte getTransA(void) { return sm_transA; }
protected:
	virtual ~LDLPalette(void);
	virtual void dealloc(void);
	void init(void);
	void initStandardColors(void);
	void initDitherColors(void);
	void initColorInfo(LDLColorInfo &colorInfo, int r, int g, int b, int a);
	void initSpecular(int index, float sr, float sg, float sb, float sa,
		float shininess);
	void initRubber(int index);
	void initSpecular(LDLColorInfo &colorInfo, float sr, float sg, float sb,
		float sa, float shininess);
	void initOtherColors(void);
	void initOtherColor(int index, TCByte r, TCByte g, TCByte b,
		TCByte a = 255);
	bool parseLDLiteColorComment(const char *comment);
	bool parseLDrawOrgColorComment(const char *comment);
	void initSpecularAndShininess(LDLColorInfo &color);
	bool getCustomColorRGBA(int colorNumber, int &r, int &g, int &b, int &a);
	bool getCustomColorInfo(int colorNumber, LDLColorInfo &colorInfo);
	int getBlendedColorComponent(TCULong c1, TCULong c2, TCULong a1,
		TCULong a2);
	virtual bool isColorNumberRGB(int colorNumber, TCByte r, TCByte g,
		TCByte b);
	virtual LDLColorInfo *updateColor(int colorNumber, const LDLColor &color,
		const LDLColor &ditherColor, int edgeColorNumber,
		float luminance = 1.0f);

	class CustomColor : public TCObject
	{
	public:
		int colorNumber;
		LDLColorInfo colorInfo;
		CustomColor() : colorNumber(0)
		{
			memset(&colorInfo, 0, sizeof(colorInfo));
		}
	};

	LDLColorInfo m_colors[512];
	TCTypedObjectArray<CustomColor> *m_customColors;
	CIStringIntMap m_namesMap;

	// LPub3D Mod - stud style
	void initStudStyleSettings(void);
	int getStudStyleOrAutoEdgeColor(int colorNumber);
	int getEdgeColorNumberFromRGB(const LDLColor& color);

	static LDLColor sm_studCylinderColor;
	static LDLColor sm_partEdgeColor;
	static LDLColor sm_blackEdgeColor;
	static LDLColor sm_darkEdgeColor;
	static TCFloat sm_partEdgeContrast;
	static TCFloat sm_partColorValueLDIndex;
	static bool sm_partEdgeColorEnabled;
	static bool sm_blackEdgeColorEnabled;
	static bool sm_darkEdgeColorEnabled;
	static bool sm_automateEdgeColor;
	static int  sm_studStyle;
	// LPub3D Mod End

	static LDLPalette *sm_defaultPalette;
	static TCByte sm_transA;
	static class LDLPaletteCleanup
	{
	public:
		~LDLPaletteCleanup(void);
	} sm_cleanup;
	friend class LDLPaletteCleanup;
};

#endif // __LDLPALETTE_H__
