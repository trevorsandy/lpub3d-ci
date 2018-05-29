/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef NATIVEPOV_H
#define NATIVEPOV_H

#include <map>
#include <list>
#include <set>
#include <math.h>

#include "project.h"
#include "tinyxml.h"
#include "render.h"

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif // WIN32

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

#define deg2rad(x) ((x) * LC_DTOR)  // M_PI / 180.0
#define rad2deg(x) ((x) * LC_RTOD)  // 180.0 / M_PI
#define EPSILON 0.0001
#define myabs(x) ((x) < 0 ? -(x) : (x))
#define fEq(x, y) (myabs((x)-(y)) < EPSILON)
#define fEq2(x, y, ep) (myabs((x)-(y)) < (ep))
#define sqr(x) ((x) * (x))

class PovVector3 : public lcVector3
{
public:
  PovVector3(void);
  PovVector3(const float _x, const float _y, const float _z);
  PovVector3(const float *);
  PovVector3(const PovVector3&);
  ~PovVector3(){}

  static void            initIdentityMatrix(float*);
  void                   transformPoint(const float *matrix, PovVector3 &newPoint) const;
  PovVector3             transformPoint(const float *matrix) const;
  float                  dot(const PovVector3&) const;
  static float           determinant(const float *matrix);
  static void            multMatrix(const float* left, const float* right, float* result);
  static float           invertMatrix(const float *matrix, float *inverseMatrix);
  void                   print(FILE* = stdout) const;
  PovVector3             mult(float* matrix) const;
  void                   upConvert(double *doubleVector);
  PovVector3&            normalize(void);
  static void            doubleNormalize(double *v);
  static void            doubleMultiply(const double *v1, double *v2, double n);
  static void            doubleAdd(const double *v1, const double *v2, double *v3);
  static double          doubleLength(const double *v);
  static void            calcRotationMatrix(float latitude, float longitude,
                                            float *rotationMatrix);

  float get(int i) const
  {
     return ((float*)this)[i];
  }

  // Overloaded Operators
  PovVector3             operator*(const PovVector3&) const;
  PovVector3             operator*(float) const;
  PovVector3             operator/(float) const;
  PovVector3&            operator=(const PovVector3&);
  PovVector3&            operator=(const lcVector3&);
  PovVector3&            operator*=(const PovVector3&);
  PovVector3&            operator*=(float);

  static float           sm_identityMatrix[16];
};

#include <string.h>
typedef enum Axis {XAxis, YAxis, ZAxis} SimAxis;

class LDLFacing: public PovVector3
{
private:
        static float glMatrix[16];
public:
        LDLFacing(void);
        LDLFacing(PovVector3& a, float phi);
        ~LDLFacing(void) {}

	// this creates a gl-compatible 4x4 matrix that you can use to
	// do rotations
	float* getMatrix(void);

	// this gets the inverse matrix -- it _should_ rotate away for
	//		the object.  (This might be used in camera translations)
	void getInverseMatrix(float *inverseMatrix);

	// setFacing sets a vector to be a rotation around vector v by phi
	//		radians.
	void setFacing(const PovVector3& v, float phi);

	// this multiplies two facings together.
	LDLFacing operator+(const LDLFacing& otherFacing);

	// normalize a facing.  Facings should automatically be
	//		normalized, but the user might want to call this every
	//		so often to get rid of floating point round off.
	LDLFacing& normalize(void);

	// getVector returns a vector pointing in the direction of the facing.
	//		< 0, 0, 1 > is asumed to be a rotation of 0.
	PovVector3 getVector() const;

	float getRotation(void) { return rotation; }

	// this gets the angle between two facings, ignoring the top of head.
	//		Not fully tested at this time.
	float angleBetween(const LDLFacing &f2);

	// this returns the inverse facing.
	LDLFacing inverse(void);

	// mult multiplies two facings together.  You get the same
	//		result as if you multiplied the two rotation matricies
	//		together.
	LDLFacing mult(const LDLFacing& f2);

	// I dont' know why dot returns a facing -- this was in
	//		the code I grabbed from graphicsGems.  Don't try this
	//		before checking the answer.
	LDLFacing dot(LDLFacing& f2);

	// This negates a facing.  I don't know the difference geometrically
	//		between a negate and an inverse -- but there is a difference.
	LDLFacing negate(void);

	// this THEORETICALLY works.  Give it a vector, and this SHOULD
	//		make the current rotation point at the vector, maintaining
	//		the top of head.  Note, SHOULD.  I tried testing, and
	//		this seemed to work.
	void pointAt(const PovVector3 &v2);

	// this function returns the difference between two facings
	PovVector3 difference(const LDLFacing &from);

	// this prints a facing's data.  Use mostly for bug testing.
	void print(FILE* = stdout);

	// Note: NOT virtual.  PovVector3 doesn't have any virtual member
	// functions, and adding one would create a vtable, which we don't
	// want.  Right now, PovVector3 looks the same in memory as an array of
	// 3 floats.
	float& operator[](int i);

protected:
        float* invertMatrix(float*);
        void swapMatrixRows(float*, int, int);
        // this is the 4th number in the 4 float array of a quaternion.
        // (the other 3 are in the private vector superclass.
        float rotation;
};

class LDLCamera
{
public:
        LDLCamera(void);
        LDLCamera(const LDLCamera& other);
        ~LDLCamera(void);

	PovVector3 getPosition(void) const { return position; }
	void setPosition(const PovVector3 &pos) { position = pos; }
	LDLFacing getFacing(void) const { return facing; }
	void setFacing(const LDLFacing &newFacing) { facing = newFacing; }
	const char* getName(void) const { return name; }
	void setName(char* n);
	LDLCamera& operator=(const LDLCamera& right) {return duplicate(right);}
	void rotate(const PovVector3 &rotation);
	void move(const PovVector3 &distance);
protected:
	LDLCamera& duplicate(const LDLCamera& copyFrom);

	PovVector3 position;
	LDLFacing facing;
	char* name;
};

class POVObject
{
public:
  POVObject(void){}
  virtual void release(void) { dealloc(); }
protected:
  virtual ~POVObject(void) {}
  virtual void dealloc(void) { delete this; }
};

// This is conversion of Lars Hassing's auto camera code from L3P.  It computes
// the correct distance and pan amount for the camera so that the viewing
// pyramid will be positioned in the closest possible position, such that the
// model just touches the edges of the view on the top and bottom, the left and
// right, or all four.
// After processing all the model data for the current camera angle, it ends up
// with 6 equations with 6 unknowns.  It uses a matrix solving routine to solve
// these.  The 6 values seem to be the X, Y, and Z coordinates of two points.
// Once it has the values, it decides which point is the correct point, and then
// uses that as the camera location.

#define L3ORDERM 6
#define L3ORDERN 6
class POVObject;
class LDLAutoCamera : public POVObject
{
public:
	LDLAutoCamera(void);
//	void setModel(LDLModel *value);                           // rem this
	void setModelCenter(const PovVector3 &value);
	void setRotationMatrix(const float *value);
	void setCamera(const LDLCamera &value);
	void setCameraGlobe(const char *value);
	void setDistanceMultiplier(float value);
	void setWidth(float value);
	void setHeight(float value);
	void setMargin(float value);
	void setFov(float value);
	void setStep(int value) { m_step = value; }
	void setScanConditionalControlPoints(bool value)
	{
		m_scanConditionalControlPoints = value;
	}

	void zoomToFit(void);
	const LDLCamera &getCamera(void) const { return m_camera; }
protected:
	~LDLAutoCamera(void);
	void dealloc(void);

	void preCalcCamera(void);
//	void scanCameraPoint(const PovVector3 &point, const LDLFileLine *pFileLine); // rem this
	int L3Solve6(float x[L3ORDERN], const float A[L3ORDERM][L3ORDERN],
		const float b[L3ORDERM]);

//	LDLModel *m_model;            // rem this
	LDLCamera m_camera;
	PovVector3 m_modelCenter;
	float m_rotationMatrix[16];
	float m_globeRadius;
	bool m_haveGlobeRadius;
	bool m_scanConditionalControlPoints;
	float m_distanceMultiplier;
	float m_width;
	float m_height;
	float m_margin;
	float m_fov;
	int m_step;
#ifdef _DEBUG
	int m_numPoints;
#endif // _DEBUG
	struct CameraData
	{
		CameraData(void)
			:direction(0.0f, 0.0f, -1.0f),
			horizontal(1.0f, 0.0f, 0.0f),
			vertical(0.0f, -1.0f, 0.0f),
			horMin(1e6),
			horMax(-1e6),
			verMin(1e6),
			verMax(-1e6)
		{
			int i;

			for (i = 0; i < 4; i++)
			{
				dMin[i] = 1e6;
			}
		}
		PovVector3 direction;
		PovVector3 horizontal;
		PovVector3 vertical;
		PovVector3 normal[4];
		float dMin[4];
		float horMin;
		float horMax;
		float verMin;
		float verMax;
		float fov;
	} *m_cameraData;
};


class Meta;
class lcVector3;
class PovVector3;
class lcDiskFile;
class NativeOptions;

class LDLFacing;
class LDLCamera;

typedef std::map<std::string, bool> StringBoolMap;
typedef std::map<std::string, std::string> StringStringMap;
typedef std::map<char, std::string> CharStringMap;
typedef std::map<int, bool> IntBoolMap;
typedef std::list<std::string> StringList;
typedef std::list<PovVector3> VectorList;
typedef std::set<std::string> StringSet;
typedef std::map<std::string, const float *> MatrixMap;

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
	float matrix[16];
};

typedef std::map<long, PovMapping> PovColorMap;
typedef std::map<std::string, PovElement> PovElementMap;
typedef void (POVObject::*LDLScanPointCallback)(const PovVector3 &point,
        bool conditionalLine);

class NativePov : POVObject
{
public:
  NativePov() {}
  ~NativePov(){}
  bool             CreateNativePovFile(const NativeOptions &);

protected:
  void             getPovModelBoundingBox();
  void             getPovModelCenter();
  void             getPovModelRadius();
  QString          getPovAspectRatio();
  float            getPovModelFov();
  void             getPovModelBounds();

  bool             writePovHeader();
  void             writePovBlankLine();
  void             writePovModelBounds();
  void             writePovCameraSettings();
  void             writePovCamera();
  bool             writePovLights();
  void             writePovLight(float lat, float lon, int num);
  void             writeSeamMacro(void);

  void             loadPovSettings();
  void             loadPovLDrawPovXml();
  void             loadXmlElements(TiXmlElement *elements);
  void             loadXmlMatrices(TiXmlElement *matrices);
  void             loadXmlColors(TiXmlElement *matrices);
  std::string      loadPovMapping(TiXmlElement *element,
                                  const char *ldrawElementName, PovMapping &mapping);
  void             loadPovDependencies(TiXmlElement *element, PovMapping &mapping);
  void             loadPovDependency(TiXmlElement *element, PovMapping &mapping);
  void             loadPovFilenames(TiXmlElement *element, PovMapping &mapping,
                                    const std::string &povVersion = std::string());
  void             loadPovCodes(TiXmlElement *element, PovMapping &mapping);

  static void      cleanupFloats(float *array, int count = 16);
  void             getCameraStrings(std::string &locationString,
                                    std::string &lookAtString, std::string &skyString, std::string &fovString);

  void             scanPoints(POVObject *scanner,
                              LDLScanPointCallback scanPointCallback, const float *matrix) const;
  void             calcMaxRadius(const PovVector3 &center);
  void             scanRadiusSquaredPoint(const PovVector3 &point, bool conditionalLine = false);

  void             writePovDeclare(const QString &name, const QString &value,
                                   const QString &commentName = NULL);
  void             writePovDeclare(const char *name, const std::string &value,
                      const char *commentName = NULL);
  void             writePovDeclare(const char *name, const char *value,
                      const char *commentName = NULL);
  void             writePovDeclare(const char *name, double value,
                      const char *commentName = NULL);
  void             writePovDeclare(const char *name, float value,
                      const char *commentName = NULL);
  void             writePovDeclare(const char *name, long value,
                      const char *commentName = NULL);
  void             writePovDeclare(const char *name, bool value,
                      const char *commentName = NULL);

  //bool             scanModelColors(LDLModel *pModel, bool inPart);

  lcArray<lcModelPartsEntry> ModelParts;
  lcDiskFile*      m_pPOVFile;
  char             m_Line[1024];

  std::string     m_udPrefix;
  std::string     m_filename;
  std::string     m_appName;
  std::string     m_renderer;
  std::string     m_appVersion;
  std::string     m_appUrl;
  std::string     m_appCopyright;

  QString          m_InputFileName;
  QString          m_OutputFileName;
  QString          m_PovGenCommand;
  Render::Mt       m_ImageType;
  float            m_Latitude;
  float            m_Longitude;
  float            m_CameraDistance;
  bool             m_TransBackground;
  bool             m_Orthographic;

  // Parameters
//  lcVector3        m_Min;
//  lcVector3        m_Max;
//  lcVector3        m_center;
//  float            m_radius;

  PovVector3      m_Position;
  PovVector3      m_Target;
  PovVector3      m_Up;

  LDLCamera       m_camera;
  float           m_rotationMatrix[16];
  float           m_width;
  float           m_height;
  float           m_fov;
  float           m_xPan;
  float           m_yPan;
  float           m_radius;
  PovVector3      m_boundingMin;
  PovVector3      m_boundingMax;
  PovVector3      m_center;

  float	          m_ambient;
  QString	  m_bottomInclude;
  float	          m_chromeBril;
  float	          m_chromeRefl;
  float	          m_chromeRough;
  float	          m_chromeSpec;
  StringSet	  m_codes;
  VectorList	  m_condEdgePoints;
  bool            m_conditionalEdges;
  float	          m_customAspectRatio;
  TiXmlElement	 *m_dependenciesElement;
  float	          m_diffuse;
  bool            m_edges;
  VectorList	  m_edgePoints;
  float	          m_edgeRadius;
  int	          m_fileVersion;
  bool	          m_findReplacements;
  bool	          m_floor;
  long	          m_floorAxis;
  bool	          m_hideStuds;
  StringSet	  m_includes;
  StringStringMap m_includeVersions;
  bool	          m_inlinePov;
  QString	  m_ldrawDir;
  QString         m_lights;
  StringSet	  m_macros;
  MatrixMap	  m_matrices;
  bool	          m_mesh2;
  float	          m_phong;
  float	          m_phongSize;
  bool            m_primSub;
  bool	          m_primSubCheck;
  long	          m_quality;
  float	          m_refl;
  bool	          m_refls;
  float	          m_rubberPhong;
  float	          m_rubberPhongSize;
  float	          m_rubberRefl;
  float           m_seamWidth;
  QStringList	  m_searchPath;
  int	          m_selectedAspectRatio;
  bool	          m_shads;
  bool	          m_smoothCurves;
  QString	  m_topInclude;
  float	          m_transFilter;
  float	          m_transIoR;
  float	          m_transRefl;
  bool	          m_unmirrorStuds;
  PovColorMap	  m_xmlColors;
  PovElementMap	  m_xmlElements;
  bool	          m_xmlMap;
  QString	  m_xmlMapPath;
  StringStringMap m_xmlMatrices;

  StringBoolMap   m_processedModels;
  StringSet       m_writtenModels;
  StringBoolMap   m_emptyModels;
  IntBoolMap      m_colorsUsed;
  // LDLModel *m_pTopModel;    // Have to improvise this
  StringStringMap m_declareNames;

  friend class Project;
};

#endif // NATIVEPOV_H
