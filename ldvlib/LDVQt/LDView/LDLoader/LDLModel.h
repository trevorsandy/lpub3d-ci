#ifndef __LDLMODEL_H__
#define __LDLMODEL_H__

#include <TCFoundation/TCAlertSender.h>
#include <TCFoundation/TCUnzipStream.h>
#include <LDLoader/LDLFileLine.h>
#include <LDLoader/LDLError.h>
#include <stdio.h>
#include <stdarg.h>
#include <fstream>

class TCDictionary;
class LDLMainModel;
class LDLCommentLine;
class LDLModelLine;
class TCImage;

typedef enum
{
	// WARNING: Increasing this list past 4 members requires more bits in flags.
	// Note that even now, it requires 3 bits, because the compiler treats it
	// as a signed integer.  When the value 2 is sign-extended, it becomes -2,
	// which doesn't match.
	BFCUnknownState,
	BFCOffState,
	BFCOnState,
	BFCForcedOnState
} BFCState;

typedef bool (*LDLFileCaseCallback)(char *filename);
typedef void (TCObject::*LDLScanPointCallback)(const TCVector &point,
	const LDLFileLine *pFileLine);
struct LDrawIniS;

class LDLModel : public TCAlertSender
{
public:
	LDLModel(void);
	LDLModel(const LDLModel &other);
	virtual TCObject *copy(void) const;
	// Color numbers might become dynamic, so not static.
	virtual void getRGBA(int colorNumber, int& r, int& g, int& b, int& a);
	virtual TCULong getPackedRGBA(int colorNumber);
	virtual int getEdgeColorNumber(int colorNumber);
	virtual bool hasSpecular(int colorNumber);
	virtual bool hasShininess(int colorNumber);
	virtual void getSpecular(int colorNumber, float *specular);
	virtual void getShininess(int colorNumber, float &shininess);
	virtual LDLModel *subModelNamed(const char *subModelName,
									bool lowRes = false, bool secondAttempt = false,
									const LDLModelLine *fileLine = NULL, bool knownPart = false);
	virtual const char *getFilename(void) const { return m_filename; }
	virtual void setFilename(const char *filename);
	virtual const char *getName(void) const { return m_name; }
	virtual const char *getDescription(void) const { return m_description; }
	virtual const char *getAuthor(void) const { return m_author; }
	virtual void setName(const char *name);
	bool load(std::ifstream &stream, TCUnzipStream *zipStream = NULL,
			  bool trackProgress = true);
	void print(int indent) const;
	virtual bool parse(void);
	virtual TCDictionary* getLoadedModels(void);
	virtual bool getLowResStuds(void) const;
	virtual LDLError *newError(LDLErrorType type, const LDLFileLine &fileLine,
							   CUCSTR format, va_list argPtr);
	virtual LDLError *newError(LDLErrorType type, const LDLFileLine &fileLine,
							   CUCSTR format, ...);
	virtual LDLError *newError(LDLErrorType type, CUCSTR format,
							   va_list argPtr);
	virtual LDLError *newError(LDLErrorType type, CUCSTR format, ...);
	virtual LDLFileLineArray *getFileLines(bool initialize = false);
	virtual const LDLFileLineArray *getFileLines(void) const
	{
		return m_fileLines;
	}
	virtual const SizeTList &getStepIndices(void) const { return m_stepIndices; }
	virtual size_t getActiveLineCount(void) const { return m_activeLineCount; }
	virtual void setActiveLineCount(size_t value) { m_activeLineCount = value; }
	virtual bool colorNumberIsTransparent(int colorNumber);
	virtual bool isMainModel(void) const { return false; }
	virtual void scanPoints(TCObject *scanner,
							LDLScanPointCallback scanPointCallback, const TCFloat *matrix,
							ptrdiff_t step = -1, bool watchBBoxIgnore = false,
							LDLStatistics *statistics = NULL) const;
	virtual void getBoundingBox(TCVector &min, TCVector &max) const;
	virtual TCFloat getMaxRadius(const TCVector &center, bool watchBBoxIgnore,
								 LDLStatistics *statistics = NULL);
	
	// Flags
	// Note that bit flags can cause odd results; thus returning the != false,
	// instead of returning the flag value directly.
	bool isPart(void) const { return m_flags.part != false; }
	bool isPrimitive(void) const { return m_flags.primitive != false; }
	bool isSubPart(void) const { return m_flags.subPart != false; }
	bool isMPD(void) const { return m_flags.mpd != false; }
	bool getNoShrinkFlag(void) const { return m_flags.noShrink != false; }
	void setNoShrinkFlag(bool value) { m_flags.noShrink = value; }
	bool isOfficial(void) const { return m_flags.official != false; }
	bool isUnOfficial(void) const { return m_flags.unofficial != false; }
	bool hasStuds(void) const { return m_flags.hasStuds != false; }
	bool hasBoundingBox(void) const;
	void copyPublicFlags(const LDLModel *src);
	void copyBoundingBox(const LDLModel *src);
	bool searchNext(const std::string &searchString, PtrDiffTVector& path,
					ptrdiff_t loopEnd, TCULong activeLineTypes) const;
	bool searchPrevious(const std::string &searchString, PtrDiffTVector& path,
						ptrdiff_t loopEnd, TCULong activeLineTypes) const;
	//bool hasBoundingBox(void) const { return m_flags.haveBoundingBox != false; }
	
	
	BFCState getBFCState(void) { return m_flags.bfcCertify; }
	
	bool getBFCOn(void)
	{
		return m_flags.bfcCertify == BFCOnState ||
		m_flags.bfcCertify == BFCForcedOnState;
	}
	
	virtual void cancelLoad(void);
	virtual bool getLoadCanceled(void);
	LDLMainModel *getMainModel(void) { return m_mainModel; }
	void setMainModel(LDLMainModel *value) { m_mainModel = value; }
	const LDLMainModel *getMainModel(void) const { return m_mainModel; }
	virtual TCObject *getAlertSender(void);
	virtual int loadMpdTexmaps(void);
	
	static const char *lDrawDir(bool defaultValue = false);
	static std::string unofficialDir(void);
	static void setLDrawDir(const char *value);
	static const std::string& ldrawZipPath(void);
	static bool setLDrawZipPath(const std::string& value);
	static bool checkLDrawZipPath(const std::string& value);
	static void setVerifyLDrawSubDirs(bool value) { sm_verifyLDrawSubDirs = value; }
	static void setFileCaseCallback(LDLFileCaseCallback value);
	static LDLFileCaseCallback getFileCaseCallback(void)
	{
		return fileCaseCallback;
	}
	// LPub3D Mod - Enable LPub3d to use LDrawIni
	static LDrawIniS *getlDrawIni(void)
	{
		return sm_lDrawIni;
	}
	// LPub3D Mod End
	static bool openFile(std::string &filename, std::ifstream &modelStream,
		TCUnzipStream *zipStream = NULL, bool loadingFoundFile = false);
	static bool openStream(const char *filename, std::ifstream &stream);
	static void combinePathParts(std::string &path, const std::string &left,
		const std::string& middle, const std::string &right = std::string());
	static bool verifyLDrawDir(const char* value);
	static void closeZips(void);
	static void ldrawZipUpdated(void);
	static bool fileExists(const std::string &filename);
protected:
	static std::string getLastModifiedKey(const std::string& lfilename);
	static time_t getLocalTimestamp(const std::string& lfilename);
	static void setSystemLDrawDir(char* value);
	static bool isInLDrawDir(const std::string& filename);
	virtual void dealloc(void);
	bool openTexmap(const char *filename, std::ifstream &texmapStream,
		TCUnzipStream *zipStream, std::string &path);
	virtual bool openSubModelNamed(const char* subModelName,
		std::string &subModelPath, std::ifstream &fileStream,
		TCUnzipStream *zipStream, bool knownPart, bool *pLoop = NULL,
		bool isText = true);
	virtual bool initializeNewSubModel(LDLModel* subModel,
		const char *dictName, std::ifstream &subModelStream,
		TCUnzipStream *zipStream);
	virtual bool initializeNewSubModel(LDLModel* subModel,
		const char *dictName);
	virtual bool read(std::ifstream &stream, TCUnzipStream *zipStream);
	virtual void processLine(std::string& line, size_t& lineNumber);
	virtual ptrdiff_t parseComment(size_t index, LDLCommentLine *commentLine);
	virtual ptrdiff_t parseMPDMeta(size_t index, const char *filename);
	virtual ptrdiff_t parseBFCMeta(LDLCommentLine *commentLine);
	virtual ptrdiff_t parseTexmapMeta(LDLCommentLine *commentLine);
	virtual ptrdiff_t parseBBoxIgnoreMeta(LDLCommentLine *commentLine);
	virtual void readComment(LDLCommentLine *commentLine);
	virtual void sendAlert(LDLError *alert);
	virtual void sendAlert(LDLErrorType type, LDLAlertLevel level,
		CUCSTR format, va_list argPtr);
	virtual void sendAlert(LDLErrorType type, LDLAlertLevel level,
		const LDLFileLine &fileLine, CUCSTR format, va_list argPtr);
	virtual void reportError(LDLErrorType type, const LDLFileLine &fileLine,
		CUCSTR format, ...);
	virtual void reportWarning(LDLErrorType type, const LDLFileLine &fileLine,
		CUCSTR format, ...);
	virtual void reportError(LDLErrorType type, CUCSTR format, ...);
	virtual void reportWarning(LDLErrorType type, CUCSTR format, ...);
	virtual void reportProgress(const char *message, float progress,
		bool mainOnly = true);
	virtual void reportProgress(const wchar_t *message, float progress,
		bool mainOnly = true);
	virtual bool isSubPart(const char *subModelName);
	virtual bool isAbsolutePath(const char *path);
//	virtual void processModelLine(LDLModelLine *modelLine);
	virtual bool openModelFile(std::string &filename, std::ifstream &modelStream,
		TCUnzipStream *zipStream, bool isText, bool knownPart = false);
	virtual void calcBoundingBox(void) const;
	virtual void calcMaxRadius(const TCVector &center, bool watchBBoxIgnore,
		LDLStatistics *statistics);
	void scanBoundingBoxPoint(const TCVector &point, LDLFileLine *pFileLine);
	void scanRadiusSquaredPoint(const TCVector &point, LDLFileLine *pFileLine);
	void sendUnofficialWarningIfPart(const LDLModel *subModel,
		const LDLModelLine *fileLine, const char *subModelName);
	void endTexmap(void);
	void extractData();
	std::basic_istream<char, std::char_traits<char>>& getLine(
		std::ifstream &stream, TCUnzipStream *zipStream, std::string& line);
	static time_t getFileTimestamp(const std::string& path);

	static void initCheckDirs();

	// LPub3D Mod - stud style
	static int isStudStylePrimitive(const char* FileName, int studStyle);
	static char *setCylinderColor(char *input);
	static int getStudStyleFile(LDLModel* subModel, const char* dictName,
		int studStyle, bool openStud);
	// LPub3D Mod End

	char *m_filename;
	char *m_name;
	char *m_author;
	char *m_description;
	LDLFileLineArray *m_fileLines;
	LDLModelArray *m_mpdTexmapModels;
	LDLCommentLineArray *m_mpdTexmapLines;
	TCImageArray *m_mpdTexmapImages;
	LDLMainModel *m_mainModel;
	SizeTList m_stepIndices;
	size_t m_activeLineCount;
	LDLModel *m_activeMPDModel;

	TCVector m_boundingMin;
	TCVector m_boundingMax;
	TCVector m_center;
	TCFloat m_maxRadius;
	TCFloat m_maxFullRadius;
	std::string m_texmapFilename;
	TCImage *m_texmapImage;
	LDLFileLine::TexmapType m_texmapType;
	TCVector m_texmapPoints[3];
#ifdef USE_CPP11
	TCFloat m_texmapExtra[2] = { 0.0f };
#else // USE_CPP11
	TCFloat m_texmapExtra[2];
#endif // USE_CPP11
	std::vector<TCByte> m_data;
	LDLCommentLine *m_dataLine;
	struct
	{
		// Private flags
		bool loadingPart:1;			// Temporal
		bool loadingSubPart:1;		// Temporal
		bool loadingPrimitive:1;	// Temporal
		bool loadingUnoffic:1;		// Temporal
		bool loadingFoundFile:1;	// Temporal
		bool mainModelLoaded:1;		// Temporal
		bool mainModelParsed:1;		// Temporal
		bool started:1;				// Temporal
		bool bfcClip:1;				// Temporal
		bool bfcWindingCCW:1;		// Temporal
		bool bfcInvertNext:1;		// Temporal
		bool haveBoundingBox:1;		// Temporal
		bool haveMaxRadius:1;		// Temporal
		bool haveMaxFullRadius:1;	// Temporal
		bool fullRadius:1;			// Temporal
		bool texmapStarted:1;		// Temporal
		bool texmapFallback:1;		// Temporal
		bool texmapNext:1;			// Temporal
		bool texmapValid:1;			// Temporal
		// Public flags
		bool part:1;
		bool subPart:1;
		bool primitive:1;
		bool mpd:1;
		bool noShrink:1;
		bool official:1;
		bool unofficial:1;
		bool hasStuds:1;
		BFCState bfcCertify:3;
		bool bboxIgnoreOn:1;
		bool bboxIgnoreBegun:1;
	} m_flags;

	static StringList sm_checkDirs;
	static char *sm_systemLDrawDir;
#ifdef WIN32
	static std::string sm_systemLDrawDirSlashes;
#endif // WIN32
	static char *sm_defaultLDrawDir;
	static LDrawIniS *sm_lDrawIni;
	static int sm_modelCount;
	static bool sm_studCylinderColorEnabled;
	static LDLFileCaseCallback fileCaseCallback;
	static std::string sm_ldrawZipPath;
	static std::string sm_unoffZipPath;
	static bool sm_verifyLDrawSubDirs;
#ifdef HAVE_MINIZIP
	static unzFile sm_ldrawZip;
	static unzFile sm_unoffZip;
#endif // HAVE_MINIZIP
	static class LDLModelCleanup
	{
	public:
		~LDLModelCleanup(void);
	} sm_cleanup;
	friend class LDLModelCleanup;
};

#endif // __LDLMODEL_H__
