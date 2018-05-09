 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class encapsulates the external renderers.  For now, this means
 * only ldglite.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include "imagematting.h"
#include "QsLog.h"

class QString;
class QStringList;
class Meta;
class AssemMeta;
class LPubMeta;
class UnitsMeta;
class RotStepMeta;
class lcVector3;

enum Mt { PLI, CSI };

class Render
{
public:
  Render() {}
  virtual ~Render() {}
  static QString const   getRenderer();
  static void            setRenderer(QString const &name);
  static QString const   getRotstepMeta(RotStepMeta &);
  bool                   useLDViewSCall(bool override = false);
  virtual int 		 renderCsi(const QString &,
                                   const QStringList &,
                                   const QStringList &,
                                   const QString &,
                                   Meta &) = 0;
  virtual int 		 renderPli(const QStringList &,
                                   const QString &,
                                   Meta &,
                                   bool bom) = 0;
  int                    rotateParts(const QString &addLine,
                                      RotStepMeta &rotStep,
                                      const QStringList &parts,
                                      QString &ldrName);
// TODO - REMOVE
//  int                    renderLDViewSCallCsi(
//                                     const QStringList &,
//                                     const QStringList &,
//                                     Meta &);
// TODO - REMOVE
//  int                    renderLDViewSCallPli(const QStringList &,
//                                         Meta &,
//                                         bool bom);
  static int             rotateParts(const QString &addLine,
                                     RotStepMeta &rotStep,
                                     QStringList &parts);
  QVector<lcVector3>     viewerCameraSettings(AssemMeta &assemMeta,
                                        const float &cd = 0.0f);
  QVector<lcVector3>     nativeCameraSettings(LPubMeta &,
                                        const int &height,
                                        const float &cd = 0.0f,
                                        const Mt &type = CSI);
// TODO - REMOVE
//  int                    render3DCsi(const QString &,
//                                    const QString &,
//                                    const QStringList &,
//                                    Meta &,
//                                    bool cisExists,
//                                    bool outOfDate);
// TODO - REMOVE
//  int                    render3DCsiSubModels(QStringList &,
//                                             QStringList &,
//                                             bool doFadeStep = false,
//                                             bool doHighlightStep = false);
// TODO - REMOVE
//  int                    load3DCsiImage(QString &);

  ImageMatting           imageMatting;
protected:
  virtual float          cameraDistance(Meta &meta, float) = 0;
};

extern Render *renderer;

class POVRay : public Render
{
public:
  POVRay() {}
  virtual ~POVRay() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class LDGLite : public Render
{
public:
  LDGLite() {}
  virtual ~LDGLite() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &,const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class LDView : public Render
{
public:
  LDView() {}
  virtual ~LDView() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

class Native : public Render
{
public:
  Native() {}
  virtual ~Native() {}
  virtual int renderCsi(const QString &,  const QStringList &, const QStringList &, const QString &, Meta &);
  virtual int renderPli(                  const QStringList &, const QString &, Meta &, bool bom);
  virtual float cameraDistance(Meta &meta, float);
};

#endif
