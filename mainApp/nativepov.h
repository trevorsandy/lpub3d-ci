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

#include "project.h"
#include "tinyxml.h"
#include "render.h"

#define deg2rad(x) ((x) * LC_DTOR)

class Meta;
class lcVector3;
class PovVector3;
class lcDiskFile;
class NativeOptions;

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
	StringList OutputFileNames;
	std::string ior;
};

struct PovElement : public PovMapping
{
	float matrix[16];
};

typedef std::map<long, PovMapping> PovColorMap;
typedef std::map<std::string, PovElement> PovElementMap;

class NativePov
{
public:
  NativePov() {}
  ~NativePov(){}
  bool             CreateNativePovFile(const NativeOptions &);

protected:
  void             writePovDeclare(const QString &name, const QString &value,
                                   const QString &commentName = NULL);
  bool             writePovHeader(const Render::Mt &, const QString &);
  void             writePovBlankLine();
  void             writePovModelBounds();
  void             writePovCamera();
  void             writePovLights();
  void             writePovLight(float lat, float lon, int num);

  // Utility functions
  static void            multMatrix(const float* left, const float* right, float* result);

  lcDiskFile*      m_pPOVFile;
  char             m_Line[1024];

  // Options
  QString          m_InputFileName;
  QString          m_OutputFileName;
  QString          m_PovGenCommand;
  Render::Mt       m_ImageType;
  int              m_ImageWidth;
  int              m_ImageHeight;
  float            m_Latitude;
  float            m_Longitude;
  float            m_CameraDistance;
  bool             m_TransBackground;
  bool             m_Orthographic;

  // Parameters
  lcVector3        m_Min;
  lcVector3        m_Max;
  lcVector3        m_Center;
  float            m_Radius;

  lcVector3        m_Position;
  lcVector3        m_Target;
  lcVector3        m_Up;
  float            m_Fov;


  float	          m_ambient;
  QString	  m_bottomInclude;
  float	          m_chromeBril;
  float	          m_chromeRefl;
  float	          m_chromeRough;
  float	          m_chromeSpec;
  StringSet	  m_codes;
  VectorList	  m_condEdgePoints;
  float	          m_customAspectRatio;
  TiXmlElement	 *m_dependenciesElement;
  float	          m_diffuse;
  VectorList	  m_edgePoints;
  float	          m_edgeRadius;
  float	          m_fileVersion;
  bool	          m_findReplacements;
  bool	          m_floor;
  long	          m_floorAxis;
  bool	          m_hideStuds;
  StringSet	  m_includes;
  StringStringMap m_includeVersions;
  bool	          m_inlinePov;
  QString	  m_ldrawDir;
  StringSet	  m_macros;
  MatrixMap	  m_matrices;
  bool	          m_mesh2;
  float	          m_phong;
  float	          m_phongSize;
  bool	          m_primSubCheck;
  long	          m_quality;
  float	          m_refl;
  bool	          m_refls;
  float	          m_rubberPhong;
  float	          m_rubberPhongSize;
  float	          m_rubberRefl;
  QStringList	  m_searchPath;
  long	          m_selectedAspectRatio;
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

  friend class Project;
};

class PovVector3 : public lcVector3
{

public:
  PovVector3();
  PovVector3(const float _x, const float _y, const float _z);
  static void            initIdentityMatrix(float*);
  void                   transformPoint(const float *matrix, lcVector3 &newPoint) const;

  static float           sm_identityMatrix[16];
};

#endif // NATIVEPOV_H
