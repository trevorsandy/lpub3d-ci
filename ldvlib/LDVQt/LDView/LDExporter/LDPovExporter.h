#ifndef __LDPOVEXPORTER_H__
#define __LDPOVEXPORTER_H__

#include "LDExporter.h"
#include <map>
#include <list>
#include <TCFoundation/TCDefines.h>
#include <TCFoundation/TCVector.h>

// LPub3D Mod - default lights
/*
Elements         Light
00 type          All
01 shadowless    All
02 latitude      All
03 longitude     All
04 target        All
05 color         All
06 intensity     All
07 fadeDistance  All
08 fadePower     All
09 spotRadius    Spot
00 spotFalloff   Spot
01 spotTightness Spot
12 circle        Area
13 areaWidth     Area
14 areaHeight    Area
15 areaRows      Area
16 areaColumns   Area
*/
#define POV_LIGHT_01 "1 0 45 0 <0,0,0> <0.8,0.8,0.8> 1 0 0 0 0 0 0 250 250 4 4"    // Area Light
#define POV_LIGHT_02 "0 0 30 120 <0,0,0> <0.8,0.8,0.8> 0 0 0 0 0 0 0 0 0 0 0"      // Point Light
#define POV_LIGHT_03 "1 0 60 -120 <0,0,0> <0.8,0.8,0.8> 1 0 0 0 0 0 0 250 250 3 3" // Area Light
#define EXPORT_POV_LIGHTS_DEFAULT POV_LIGHT_01 ";" POV_LIGHT_02 ";" POV_LIGHT_03
// LPub3D Mod End

class LDLModel;
class LDLFileLine;
class LDLModelLine;
class LDLTriangleLine;
class LDLQuadLine;
class LDLLineLine;
class LDLCommentLine;
class LDLShapeLine;
class TiXmlElement;

typedef std::list<TCVector> VectorList;
typedef std::map<std::string, const TCFloat *> MatrixMap;

struct PovName
{
	std::string name;
	StringStringMap attributes;
};

typedef std::list<PovName> PovNameList;

struct PovMapping
{
	PovNameList names;
	StringList povCodes;
	StringList povFilenames;
	std::string ior;
};

struct PovElement : public PovMapping
{
#ifdef USE_CPP11
	float matrix[16] = { 0.0f };
#else // USE_CPP11
	float matrix[16];
	PovElement()
	{
		memset(matrix, 0, sizeof(matrix));
	}
#endif // USE_CPP11
};

typedef std::map<TCULong, PovMapping> PovColorMap;
typedef std::map<std::string, PovElement> PovElementMap;

class LDPovExporter : public LDExporter
{
	enum ColorType
	{
		CTOpaque,
		CTTransparent,
		CTRubber,
		CTChrome,
	};
public:
	LDPovExporter(void);
	int doExport(LDLModel *pTopModel);
	virtual std::string getExtension(void) const { return "pov"; }
	virtual ucstring getTypeDescription(void) const;
	static bool shouldFlipNormal(const TCVector &normal1,
		const TCVector &normal2);
	struct LineKey
	{
		LineKey(void);
		LineKey(const TCVector &point0, const TCVector &point1);
		LineKey(const LineKey &other);
		LineKey &operator=(const LineKey &other);
		bool operator<(const LineKey &other) const;
		bool operator==(const LineKey &other) const;
		TCVector direction;
		TCVector intercept;
	};
protected:
	typedef std::map<TCVector, size_t> VectorSizeTMap;
	typedef std::map<size_t, TCVector> SizeTVectorMap;
	typedef std::map<TCVector, TCVector> VectorVectorMap;
	typedef std::vector<TCVector> TCVectorVector;
	struct Shape
	{
		Shape() {}
		Shape(const TCVector *pts, size_t count, const TCFloat *matrix);
		Shape(const TCVector &p1, const TCVector &p2);
		Shape(const TCVector &p1, const TCVector &p2, const TCVector &p3);
		Shape(const TCVector &p1, const TCVector &p2, const TCVector &p3,
			const TCVector &p4);
		TCVectorVector points;
	};
	struct SmoothTriangle
	{
		bool initLineKeys(const SizeTVectorMap &indexToVert);
		void setNormal(const TCVector &point, const TCVector &normal);
		LineKey lineKeys[3];
		TCVector edgeNormals[3];
		VectorVectorMap normals;
#ifdef USE_CPP11
		int colorNumber = 0;
		int vertexIndices[3] = { 0 };
		int normalIndices[3] = { 0 };
		bool hardEdges[3] = { false };
		size_t smoothPass = 0;
#else // USE_CPP11
		int colorNumber;
		int vertexIndices[3];
		int normalIndices[3];
		bool hardEdges[3];
		size_t smoothPass;
		SmoothTriangle()
			: colorNumber(0)
			, smoothPass(0)
		{
			memset(vertexIndices, 0, sizeof(vertexIndices));
			memset(normalIndices, 0, sizeof(normalIndices));
			memset(hardEdges, 0, sizeof(hardEdges));
		}
#endif // USE_CPP11
	};
	// LPub3D Mod - lights
	struct Light
	{
		int   type;
		int   shadowless;
		float latitude;
		float longitude;
		std::string target;
		std::string color;
		float intensity;
		float fadeDistance;
		float fadePower;
		float radius;
		float falloff;
		float tightness;
		int   circle;
		int   width;
		int   height;
		int   rows;
		int   columns;
	};

	typedef std::list<Light> LightList;
	// LPub3D Mod End
	typedef std::list<Shape> ShapeList;
	typedef std::map<int, ShapeList> IntShapeListMap;
	typedef std::vector<SmoothTriangle> SmoothTriangleVector;
	typedef std::list<SmoothTriangle> SmoothTriangleList;
	typedef std::list<SmoothTriangle*> SmoothTrianglePList;
	typedef std::vector<SmoothTriangle*> SmoothTrianglePVector;
	typedef std::set<SmoothTriangle*> SmoothTrianglePSet;
	typedef std::map<LineKey, SmoothTrianglePSet> TriangleEdgesMap;
	typedef std::map<TCVector, SmoothTrianglePList> TrianglePPointsMap;
	typedef std::pair<TCVector, TCVector> LinePair;
	typedef std::list<LinePair> LineList;
	typedef std::map<LineKey, LineList> EdgeMap;
	typedef std::set<LineKey> LineKeySet;

	~LDPovExporter(void);
	void dealloc(void);
	bool writeHeader(void);
	// LPub3D Mod - additional POV-Ray export settings
	void writeLgQuality(void);
	void writeGlobalSettings(void);
	void writeLightSourceMacro(void);
	// LPub3D Mod End
	void writeMainModel(void);
	void writeFloor(void);
	bool writeModel(LDLModel *pModel, const TCFloat *matrix, bool inPart);
	bool writeCamera(void);
	bool writeLights(void);
	// LPub3D Mod - lights
	// Replaced with writeLightSourceMacro()
	// void writeLight(TCFloat lat, TCFloat lon, int num);
	// LPub3D Mod End
	bool writeModelObject(LDLModel *pModel, bool mirrored,
		const TCFloat *matrix, bool inPart);
	void writeGeometry(IntShapeListMap &colorGeometryMap);
	bool scanModelColors(LDLModel *pModel, bool inPart);
	bool writeModelColors(void);
	bool writeEdges(void);
	bool writeXmlMatrix(const char *filename);
	void writeMatrix(const TCFloat *matrix);
	void writeSeamMacro(void);
	void writeSeamMatrix(LDLModelLine *pModelLine);
	bool writeColor(int colorNumber, bool slope = false);
	void writeColorDeclaration(int colorNumber);
	void writeInnerColorDeclaration(int colorNumber, bool slope);
	void writeRGBA(int r, int g, int b, int a);
	bool writeModelLine(LDLModelLine *pModelLine, bool &studsStarted,
		bool mirrored, const TCFloat *matrix, bool inPart);
	void indentStud(bool studsStarted);
	void writeInnerModelLine(const std::string &declareName,
		LDLModelLine *pModelLine, bool mirrored, bool slope, bool studsStarted,
		bool inPart);
	void writeCommentLine(LDLCommentLine *pCommentLine, bool &ifStarted,
		bool &elseStarted, bool &povMode);
	void writeTriangleLine(LDLTriangleLine *pTriangleLine);
	void writeQuadLine(LDLQuadLine *pQuadLine);
	void writeTriangleLineVertices(LDLTriangleLine *pTriangleLine, int &total);
	void writeQuadLineVertices(LDLQuadLine *pQuadLine, int &total);
	void writeMesh2Vertices(const TCVector *pVertices, size_t count, int &total);
	void writeTriangleLineIndices(LDLTriangleLine *pTriangleLine, int &current,
		int &total);
	void writeQuadLineIndices(LDLQuadLine *pQuadLine, int &current, int &total);
	void writeMesh2Indices(int i0, int i1, int i2, int &total);
	void writeEdgeLineMacro(void);
	void writeEdgeColor(void);
	void endMesh(void);
	bool onEdge(const LinePair &edge, const LineList &edges);
	bool normalsCheck(const TCVector &normal1, TCVector normal2);
	bool edgesOverlap(const LinePair &edge1, const LinePair &edge2);
	int findEdge(const SmoothTriangle &triangle, const LineKey &lineKey);
	int findPoint(const SmoothTriangle &triangle, const TCVector &point,
		const SizeTVectorMap &points);
	void startMesh(void);
	void startMesh2(void);
	void startMesh2Section(const char *sectionName);
	void writeMesh(int colorNumber, const ShapeList &list);
	void writeMesh2(int colorNumber, const ShapeList &list);
	void writeMesh2(int colorNumber, const VectorSizeTMap &vertices,
		const VectorSizeTMap &normals, const SmoothTriangleVector &triangles);
	void smoothGeometry(int colorNumber, const ShapeList &list,
		const ShapeList &edges, VectorSizeTMap &vertices,
		VectorSizeTMap &normals, SmoothTriangleVector &triangles);
	bool initSmoothTriangle(SmoothTriangle &triangle, VectorSizeTMap &vertices,
		TrianglePPointsMap &trianglePoints, SizeTVectorMap &indexToVert,
		const TCVector &point1, const TCVector &point2, const TCVector &point3);
	bool trySmooth(const TCVector &normal1, TCVector &normal2);
	bool shouldSmooth(const TCVector &normal1, const TCVector &normal2);
	void endMesh2Section(void);
	void startStuds(bool &started);
	void endStuds(bool &started);
	void writePoints(const TCVector *points, int count, int size = -1,
		int start = 0);
	void writeTriangle(const TCVector *points, int size = -1, int start = 0);
	void writePoint(const TCVector &point);
	std::string getDeclareName(LDLModel *pModel, bool mirrored,
		bool inPart = false);
	std::string getDeclareName(const std::string &modelFilename, bool mirrored,
		bool inPart = false, bool isPart = false);
	std::string getModelFilename(const LDLModel *pModel);
	std::string findInclude(const std::string &filename);
	bool findModelInclude(const LDLModel *pModel);
	std::string findMainPovName(const PovMapping &mapping);
	const PovName *findPovName(const PovMapping &mapping, const char *attrName,
		const char *attrValue);
	bool findXmlModelInclude(const LDLModel *pModel);
	void writeDescriptionComment(const LDLModel *pModel);
	bool findModelGeometry(LDLModel *pModel, IntShapeListMap &colorGeometryMap,
		bool mirrored, const TCFloat *matrix, bool inPart,
		int colorNumber = 16);
	bool isStud(LDLModel *pModel);
	void getCameraStrings(std::string &locationString,
		std::string &lookAtString, std::string &skyString);
	void scanEdgePoint(const TCVector &point, const LDLFileLine *pFileLine);
	void loadLDrawPovXml(void);
	void loadXmlColors(TiXmlElement *matrices);
	std::string loadPovMapping(TiXmlElement *element,
		const char *ldrawElementName, PovMapping &mapping);
	// LPub3D Mod - lights
	void loadLights(const char* povLights);
	std::string getLightsString(void) const;
	// LPub3D Mod End
	void loadPovDependency(TiXmlElement *element, PovMapping &mapping);
	void loadPovFilenames(TiXmlElement *element, PovMapping &mapping,
		const std::string &povVersion = std::string());
	void loadPovCodes(TiXmlElement *element, PovMapping &mapping);
	void loadPovDependencies(TiXmlElement *element, PovMapping &mapping);
	void loadXmlMatrices(TiXmlElement *matrices);
	void loadXmlElements(TiXmlElement *elements);
	void loadXmlMovedTos(TiXmlElement *movedTos);
	void loadXmlMovedTo(const std::string& newName, TCFloat *matrix,
		const PovElement& oldElement);
	void loadXmlMatrix(TiXmlElement *element, TCFloat *matrix);
	bool writeCode(const std::string &code, bool lineFeed = true);
	bool writeInclude(const std::string &filename, bool lineFeed = true,
		const LDLModel *pModel = NULL);
	void writeLogo(void);
	virtual void initSettings(void) const;
	virtual LDExporterSetting *addEdgesSettings(LDExporterSetting *pGroup)
		const;
	virtual LDExporterSetting *addGeometrySettings(void) const;
	std::string getAspectRatio(void);
	std::string replaceSpecialChacters(const char *string);
	void writeLDXOpaqueColor(void);
	void writeLDXTransColor(void);
	void writeLDXChromeColor(void);
	void writeLDXRubberColor(void);
	ColorType getColorType(int colorNumber);
	void writeDeclare(const char *name, const std::string &value,
		const char *commentName = NULL);
	void writeDeclare(const char *name, const char *value,
		const char *commentName = NULL);
	void writeDeclare(const char *name, double value,
		const char *commentName = NULL);
	void writeDeclare(const char *name, float value,
		const char *commentName = NULL);
	void writeDeclare(const char *name, long value,
		const char *commentName = NULL);
	void writeDeclare(const char *name, bool value,
		const char *commentName = NULL);
	bool shouldDrawConditional(const TCVector &p1, const TCVector &p2,
		const TCVector &p3, const TCVector &p4, const TCFloat *matrix);

	bool writePrimitive(const char *format, ...);
	bool writeRoundClipRegion(TCFloat fraction, bool closeOff = true);
	bool writeNdisClipRegion(TCFloat fraction);
	bool writeTNdisClipRegion(TCFloat fraction);
	virtual bool substituteEighthSphere(bool bfc, bool is48 = false);
	virtual bool substituteEighthSphereCorner(bool bfc, bool is48 = false);
	virtual bool substituteCylinder(TCFloat fraction, bool bfc, bool inPart,
		bool is48);
	virtual bool substituteCylinder(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteSlopedCylinder(TCFloat fraction, bool bfc,
		bool inPart, bool is48);
	virtual bool substituteSlopedCylinder(TCFloat fraction, bool bfc,
		bool is48 = false, SlopeType slopeType = STStandard);
	virtual bool substituteSlopedCylinder2(TCFloat fraction, bool bfc,
		bool inPart, bool is48);
	virtual bool substituteSlopedCylinder2(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteDisc(TCFloat fraction, bool bfc, bool inPart,
		bool is48);
	virtual bool substituteDisc(TCFloat fraction, bool bfc, bool is48 = false);
	virtual bool substituteNotDisc(TCFloat fraction, bool bfc, bool inPart,
		bool is48);
	virtual bool substituteNotDisc(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteTNotDisc(TCFloat fraction, bool bfc, bool inPart,
		bool is48);
	virtual bool substituteTNotDisc(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteTangent(TCFloat fraction, bool bfc, bool inPart,
		bool is48);
	virtual bool substituteTangent(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteCone(TCFloat fraction, int size, bool bfc,
		bool inPart, bool is48);
	virtual bool substituteCone(TCFloat fraction, int size, bool bfc,
		bool is48 = false);
	virtual bool substituteRing(TCFloat fraction, int size, bool bfc,
		bool inPart, bool is48, bool isOld);
	virtual bool substituteRing(TCFloat fraction, int size, bool bfc,
		bool is48 = false, bool isOld = false);
	virtual bool allowRTori(void) const { return false; }
	virtual bool substituteTorusQ(TCFloat fraction, int size, bool bfc,
		bool inPart, bool isMixed, bool is48);
	virtual bool substituteTorusQ(TCFloat fraction, int size, bool bfc,
		bool isMixed, bool is48 = false);
	virtual bool substituteTorusIO(bool inner, TCFloat fraction, int size,
		bool bfc, bool inPart, bool isMixed, bool is48);
	virtual bool substituteTorusIO(bool inner, TCFloat fraction, int size,
		bool bfc, bool isMixed, bool is48 = false);
	virtual bool substituteChrd(TCFloat fraction, bool bfc, bool inPart,
		bool is48);
	virtual bool substituteChrd(TCFloat fraction, bool bfc, bool is48 = false);
	virtual bool substituteStudInPart(bool inPart);
	virtual bool substituteStud(void);

	std::string getPrimName(const std::string &base, bool is48, bool inPart,
		int num = -1, int den = -1);

	// NOTE: loadSettings is NOT virtual: it's called from the constructor.
	void loadSettings(void);

	double alphaMod(int color);
	static std::string filterForAlpha(int alpha);
	static void cleanupFloats(TCFloat *array, int count = 16);
	static void cleanupDoubles(double *array, int count = 16);
	static const char *get48Prefix(bool is48);		

	StringBoolMap m_processedModels;
	StringSet m_writtenModels;
	StringBoolMap m_emptyModels;
	IntBoolMap m_colorsUsed;
	LDLModel *m_pTopModel;
	StringStringMap m_declareNames;
	FILE *m_pPovFile;
	StringList m_searchPath;
	bool m_findReplacements;
	bool m_xmlMap;
	std::string m_xmlMapPath;
	std::string m_topInclude;
	std::string m_bottomInclude;
	// LPub3D Mod - lights
	LightList m_povLightList;
	// LPub3D Mod End
	bool m_inlinePov;
	bool m_hideStuds;
	bool m_smoothCurves;
	bool m_unmirrorStuds;
	long m_quality;
	bool m_background;
	bool m_floor;
	long m_floorAxis;
	bool m_refls;
	bool m_shads;
	bool m_mesh2;
	long m_selectedAspectRatio;
	float m_customAspectRatio;
	TCFloat m_edgeRadius;
	TCFloat m_ambient;
	TCFloat m_diffuse;
	TCFloat m_refl;
	TCFloat m_phong;
	TCFloat m_phongSize;
	TCFloat m_transRefl;
	TCFloat m_transFilter;
	TCFloat m_transIoR;
	TCFloat m_rubberRefl;
	TCFloat m_rubberPhong;
	TCFloat m_rubberPhongSize;
	TCFloat m_chromeRefl;
	TCFloat m_chromeBril;
	TCFloat m_chromeSpec;
	TCFloat m_chromeRough;
	TCFloat m_fileVersion;
	VectorList m_edgePoints;
	VectorList m_condEdgePoints;
	PovColorMap m_xmlColors;
	PovElementMap m_xmlElements;
	StringStringMap m_includeVersions;
	StringStringMap m_xmlMatrices;
	TiXmlElement *m_dependenciesElement;
	MatrixMap m_matrices;
	std::string m_ldrawDir;
	StringSet m_includes;
	StringSet m_codes;
	StringSet m_macros;
	bool m_primSubCheck;

	static CharStringMap sm_replacementChars;
};

#endif // __LDPOVEXPORTER_H__
