#ifndef __LDLCOMMENTLINE_H__
#define __LDLCOMMENTLINE_H__

#include <LDLoader/LDLFileLine.h>

class LDLCommentLine : public LDLFileLine
{
public:
	// OBI
	enum OBICommand
	{
		OBICERROR,
		OBICSet,
		OBICUnset,
		OBICNext,
		OBICStart,
		OBICEnd,
	};
	// /OBI

	virtual TCObject *copy(void) const;
	virtual bool parse(void);
	virtual LDLLineType getLineType(void) const { return LDLLineTypeComment; }
	virtual bool getMPDFilename(std::string* filename = NULL) const;
	virtual bool isPartMeta(void) const;
	virtual bool isOfficialPartMeta(bool partMeta) const;
	virtual bool isPrimitiveMeta(void) const;
	virtual bool isNoShrinkMeta(void) const;
	virtual bool isBFCMeta(void) const;
	virtual bool isLDViewMeta(void) const;
	virtual bool isBBoxIgnoreMeta(void) const;
	virtual bool isTexmapMeta(void) const;
	virtual bool isNewGeometryMeta(void) const;
	virtual bool isDataMeta(void) const;
	virtual bool isDataRowMeta(void) const;
	virtual bool isMeta(const std::string& metaName) const;
	virtual size_t getMetaOffset(const std::string& metaName) const;
	virtual bool containsBBoxIgnoreCommand(const char *command) const;
	virtual const char *getWord(size_t index) const;
	virtual size_t getNumWords(void) const;
	virtual LDLFileLineArray *getReplacementLines(void);
	virtual void setValid(bool value) { m_valid = value; }
	virtual void updateStatistics(LDLStatistics& statistics) const
	{
		++statistics.comments;
	}

	// OBI
	bool isOBIMeta(void) const;
	OBICommand getOBICommand(void) const;
	bool hasOBIConditional(void) const;
	bool getOBIConditional(void) const;
	bool hasOBIToken(void) const;
	const char *getOBIToken(void) const;
	int getOBIColorNumber(void) const;
	// /OBI

	virtual bool isMovedToMeta(void) const;
	virtual bool isStepMeta(void) const;
	virtual bool getAuthor(std::string& author) const;
	virtual bool containsBFCCommand(const char *command) const;
	virtual bool containsTexmapCommand(const char *command) const;
	virtual bool containsDataCommand(const char *command) const;
	virtual const char *getProcessedLine(void) const
	{
		return m_processedLine;
	}
protected:
	LDLCommentLine(LDLModel *parentModel, const char *line, size_t lineNumber,
		const char *originalLine = NULL);
	LDLCommentLine(const LDLCommentLine &other);
	~LDLCommentLine(void);
	virtual void dealloc(void);
	void setupProcessedLine(void);
	bool containsCommand(const char *command, ptrdiff_t startWord,
		bool caseSensitive = false, ptrdiff_t endWord = -1) const;

	char *m_processedLine;
	TCStringArray *m_words;

	friend class LDLFileLine; // Needed because constructors are protected.
};

#endif // __LDLCOMMENTLINE_H__
