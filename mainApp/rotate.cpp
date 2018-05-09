 
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

#include "lpub.h"
#include <QtWidgets>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <math.h>

#include "render.h"
#include "ldrawfiles.h"


/*****************************************************************************
 * Rotation routines
 ****************************************************************************/

void
matrixMakeRot(
  double rm[3][3],
  double rots[3])
{
  double pi = 2*atan2(1.0,0.0);

  double s1 = sin(2*pi*rots[0]/360.0);
  double c1 = cos(2*pi*rots[0]/360.0);
  double s2 = sin(2*pi*rots[1]/360.0);
  double c2 = cos(2*pi*rots[1]/360.0);
  double s3 = sin(2*pi*rots[2]/360.0);
  double c3 = cos(2*pi*rots[2]/360.0);

  rm[0][0] = c2*c3;
  rm[0][1] = -c2*s3;
  rm[0][2] = s2;

  rm[1][0] = c1 * s3 + s1 * s2 * c3;
  rm[1][1] = c1 * c3 - s1 * s2 * s3;
  rm[1][2] = -s1 * c2;

  rm[2][0] = s1 * s3 - c1 * s2 * c3;
  rm[2][1] = s1 * c3 + c1 * s2 * s3;
  rm[2][2] = c1 * c2;
}

void
matrixCp(
  double dst[3][3],
  double src[3][3])
{
  int i,j;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      dst[i][j] = src[i][j];
    }
  }
}

void
matrixMult3(
  double res[3][3],
  double lft[3][3],
  double rht[3][3])
{
  int i,j,k;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      res[i][j] = 0.0;
    }
  }

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      for (k = 0; k < 3; k++) {
        res[i][j] += lft[i][k] * rht[k][j];
      }
    }
  }
}

void
matrixMult(
  double res[3][3],
  double src[3][3])
{
  double t[3][3];

  matrixCp(t,res);
  matrixMult3(res,t,src);
}

void rotatePoint(
  double p[3],
  double rm[3][3])
{
  double X = rm[0][0]*p[0] + rm[0][1]*p[1] + rm[0][2]*p[2];
  double Y = rm[1][0]*p[0] + rm[1][1]*p[1] + rm[1][2]*p[2];
  double Z = rm[2][0]*p[0] + rm[2][1]*p[1] + rm[2][2]*p[2];

  p[0] = X;
  p[1] = Y;
  p[2] = Z;
}

void rotateMatrix(
  double pm[3][3],
  double rm[3][3])
{

  double res[3][3];

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
       res[i][j] = 0.0;
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        res[i][j] += rm[i][k] * pm[k][j];
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      pm[i][j] = res[i][j];
    }
  }
}

int Render::rotateParts(
    const QString     &addLine,
          RotStepMeta &rotStep,
    const QStringList &parts,
          QString     &ldrName)
{
  QStringList rotatedParts = parts;
  rotateParts(addLine,rotStep,rotatedParts);

  QFile file(ldrName);
  if ( ! file.open(QFile::WriteOnly | QFile::Text)) {
    emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Cannot open file %1 for writing:\n%2")
                         .arg(ldrName) .arg(file.errorString()));
    return -1;
  }

  QTextStream out(&file);

  RotStepData rotStepData = rotStep.value();
  QString rotsComment = QString("0 // ROTSTEP %1 %2 %3 %4")
                                .arg(rotStepData.type)
                                .arg(rotStepData.rots[0])
                                .arg(rotStepData.rots[1])
                                .arg(rotStepData.rots[2]);
                                
  out << rotsComment << endl;

  for (int i = 0; i < rotatedParts.size(); i++) {
    QString line = rotatedParts[i];
    out << line << endl;
  }

  file.close();

  return 0;
}

int Render::rotateParts(
        const QString &addLine,
        RotStepMeta   &rotStep,
        QStringList   &parts)
{
  double min[3], max[3];

  min[0] = 1e23, max[0] = -1e23,
  min[1] = 1e23, max[1] = -1e23,
  min[2] = 1e23, max[2] = -1e23;

  double defaultViewMatrix[3][3], defaultViewRots[3];

  // TODO - set/get from Preferences
  defaultViewRots[0] = 0;
  defaultViewRots[1] = 0;
  defaultViewRots[2] = 0;

  matrixMakeRot(defaultViewMatrix,defaultViewRots);

  RotStepData rotStepData = rotStep.value();

  double rm[3][3];

  if (rotStepData.type.size() == 0) {
    matrixCp(rm,defaultViewMatrix);
  } else {
    double rotStepMatrix[3][3];
    matrixMakeRot(rotStepMatrix,rotStepData.rots);
    if (rotStepData.type == "ABS") {
      matrixCp(rm,rotStepMatrix);
    } else {
      matrixMult3(rm,defaultViewMatrix,rotStepMatrix);
    }
  }

  QStringList tokens;

  split(addLine,tokens);

  if (addLine.size() && tokens.size() == 15 && tokens[0] == "1") {
    if (LDrawFile::mirrored(tokens) /* || ! defaultRot */) {

      double alm[3][3];

      for (int token = 5; token < 14; token++) {
        double value = tokens[token].toFloat();
        alm[(token-5) / 3][(token-5) % 3] = value;
      }
      matrixMult(rm,alm);
    }
  }

  // rotate all the parts

  QString processed_parts;

  for (int i = 0; i < parts.size(); i++) {

    QString line = parts[i];
    QStringList tokens;

    split(line,tokens);

    if (tokens.size() < 2) {
      continue;
    }

    double v[4][3];

    if (tokens[0] == "1") {
      v[0][0] = tokens[2].toFloat();
      v[0][1] = tokens[3].toFloat();
      v[0][2] = tokens[4].toFloat();

      rotatePoint(v[0],rm);

      for (int d = 0; d < 3; d++) {
        if (v[0][d] < min[d]) {
          min[d] = v[0][d];
        }
        if (v[0][d] > max[d]) {
          max[d] = v[0][d];
        }
      }
    } else if (tokens[0] == "2") {
      int c = 2;
      for (int j = 0; j < 2; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    } else if (tokens[0] == "3") {
      int c = 2;
      for (int j = 0; j < 3; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    } else if (tokens[0] == "4") {
      int c = 2;
      for (int j = 0; j < 4; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    } else if (tokens[0] == "5") {
      int c = 2;
      for (int j = 0; j < 4; j++) {
        v[j][0] = tokens[c].toDouble();
        v[j][1] = tokens[c+1].toDouble();
        v[j][2] = tokens[c+2].toDouble();
        c += 3;
        rotatePoint(v[j],rm);

        for (int d = 0; d < 3; d++) {
          if (v[j][d] < min[d]) {
            min[d] = v[j][d];
          }
          if (v[j][d] > max[d]) {
            max[d] = v[j][d];
          }
        }
      }
    }
  }

  // center the design at the LDraw origin

  double center[3];
  for (int d = 0; d < 3; d++) {
    center[d] = (min[d] + max[d])/2;
  }

  for (int i = 0; i < parts.size(); i++) {
    QString line = parts[i];
    QStringList tokens;

    split(line,tokens);

    if (tokens.size() < 2) {
      continue;
    }

    double v[4][3];
    double pm[3][3];
    QString t1;

    if (tokens[0] == "1") {
      int c = 2;
      v[0][0] = tokens[c].toFloat();
      v[0][1] = tokens[c+1].toFloat();
      v[0][2] = tokens[c+2].toFloat();
      c += 3;
      for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
          pm[y][x] = tokens[c++].toDouble();
        }
      }
      rotatePoint(v[0],rm);
      v[0][0] -= center[0];
      v[0][1] -= center[1];
      v[0][2] -= center[2];
      rotateMatrix(pm,rm);

      t1 = QString("1 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7 "
                   "%8 %9 %10 "
                   "%11 %12 %13 "
                   "%14")

                   .arg(tokens[1])
                   .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                   .arg(pm[0][0]) .arg(pm[0][1]) .arg(pm[0][2])
                   .arg(pm[1][0]) .arg(pm[1][1]) .arg(pm[1][2])
                   .arg(pm[2][0]) .arg(pm[2][1]) .arg(pm[2][2])
                   .arg(tokens[tokens.size()-1]);

      parts[i] = t1;
    } else if (tokens[0] == "2") {

      int c = 2;
      for (int n = 0; n < 2; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("2 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7")
                   .arg(tokens[1])
                   .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                   .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2]);
      parts[i] = t1;
    } else if (tokens[0] == "3") {

      int c = 2;
      for (int n = 0; n < 3; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("3 %1 "
                   "%2 %3 %4  "
                   "%5 %6 %7  "
                   "%8 %9 %10")
                     .arg(tokens[1])
                     .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                     .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2])
                     .arg( v[2][0]) .arg( v[2][1]) .arg( v[2][2]);
      parts[i] = t1;
    } else if (tokens[0] == "4") {

      int c = 2;
      for (int n = 0; n < 4; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("4 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7 "
                   "%8 %9 %10 "
                   "%11 %12 %13")
                     .arg(tokens[1])
                     .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                     .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2])
                     .arg( v[2][0]) .arg( v[2][1]) .arg( v[2][2])
                     .arg( v[3][0]) .arg( v[3][1]) .arg( v[3][2]);
      parts[i] = t1;
    } else if (tokens[0] == "5") {

      int c = 2;
      for (int n = 0; n < 4; n++) {
        for (int d = 0; d < 3; d++) {
          v[n][d] = tokens[c++].toDouble();
        }
        rotatePoint(v[n],rm);
        for (int d = 0; d < 3; d++) {
          v[n][d] -= center[d];
        }
      }

      t1 = QString("5 %1 "
                   "%2 %3 %4 "
                   "%5 %6 %7 "
                   "%8 %9 %10 "
                   "%11 %12 %13")
                     .arg(tokens[1])
                     .arg( v[0][0]) .arg( v[0][1]) .arg( v[0][2])
                     .arg( v[1][0]) .arg( v[1][1]) .arg( v[1][2])
                     .arg( v[2][0]) .arg( v[2][1]) .arg( v[2][2])
                     .arg( v[3][0]) .arg( v[3][1]) .arg( v[3][2]);
      parts[i] = t1;
    }
  }
  return 0;
}

// Align the viewer camera with LPub3D default view
QVector<lcVector3> Render::viewerCameraSettings(
      AssemMeta &assemMeta,
      const float &cd)
{

    // Initialization
    double camera_latitude    = 0.0;
    double camera_longitude   = 0.0;
    double camera_distance    = 577.0;

    double projection_fromx   = -250.0;  // LeoCAd home value
    double projection_fromy   = -250.0;  // LeoCAd home value
    double projection_fromz   = 75.0;    // LeoCAd home value

    double projection_towardx = 0.0;     // LeoCAd home value
    double projection_towardy = 0.0;     // LeoCAd home value
    double projection_towardz = 0.0;     // LeoCAd home value

    double projection_upx     = -0.2357; // LeoCAd home value
    double projection_upy     = -0.2357; // LeoCAd home value
    double projection_upz     = 0.94281; // LeoCAd home value

    double projection_fov     = 30.00;   // LeoCAD home value
    double projection_znear   = 25.0;    // LeoCAD home value
    double projection_zfar    = 50000.0; // LeoCAD home value

    //double adjustment         = -37.9519;

    projection_fov   = assemMeta.v_fov.value();    // sets the camera FOV angle in degrees,(default = 30.0f)
    projection_znear = assemMeta.v_znear.value();  // sets the near clipping plane.        (default = 25.0f)
    projection_zfar  = assemMeta.v_zfar.value();   // sets the far clipping plane.         (default = 50000.0f)

    camera_latitude  = assemMeta.v_angle.value(0); // sets the camera globe latitude in degrees  (default = 23)
    camera_longitude = assemMeta.v_angle.value(1); // sets the camera globe longitude in degrees (default = 45)
    camera_distance  = cd;                         // sets the camera globe radius in units      (calculated)

    // Oblique transform.
    double la, lo;
    la = 3.1415927 * camera_latitude  / 180.0;
    lo = 3.1415927 * camera_longitude / 180.0;

    projection_fromx = camera_distance * sin(lo);
    projection_fromy = camera_distance * sin(la);
    projection_fromz = camera_distance * cos(lo);

    projection_fromx *= cos(la);
    projection_fromz *= cos(la);

    QVector<lcVector3> viewMatrix(0);
    float fx, fy, fz, tx, ty, tz, ux, uy, uz;

    // Position (camera globe)
    fx = projection_fromx;
    fy = -projection_fromz;             // Switch Y and Z coordinates to match LeoCAD. Set Y negative to match LDraw Y axis vertical with negative value in the up direction
    fz = projection_fromy; // + adjustment; // adjust (-37.9519) to reach LeoCAD default value of 187.5 //TODO see if can do better

    // Target
    tx = projection_towardx;
    ty = projection_towardy;
    tz = projection_towardz;

    // Up vector
    ux = projection_upx;
    uy = projection_upy;
    uz = projection_upz;

    // return camara position, target, up vector, fov, znear and zfar
    viewMatrix.append(lcVector3(fx,fy,fz));         // camera position
    viewMatrix.append(lcVector3(tx,ty,tz));         // camera target
    viewMatrix.append(lcVector3(ux,uy,uz));         // camera up vector
    viewMatrix.append(lcVector3(projection_fov,     // fov
                                projection_znear,   // znear
                                projection_zfar));  // zfar

    /* ______________DEBUG_____________________________________ /

    //-caN sets the camera FOV angle in degrees, default = 30.0f.
    logTrace() << QString("[-caN] FOV = %1").arg(projection_fov,0,'f',1);
    // -zN sets the near clipping plane.  (default = 25.0f)
    // -ZN sets the far clipping plane.  (default = 12500.0f)
    logTrace() << QString("[-zN],[-ZN] ZCLIP = %1 znear, %2 zfar").arg(projection_znear,0,'f',1).arg(projection_zfar,0,'f',1);
    // -cg<la>,<lo>,<r> sets the camera location on globe.                         FROM
    logTrace() << QString("[-cg<la>,<lo>,<r>] CAMERA GLOBE = %1, %2, %3").arg(assemMeta.angle.value(0)).arg(assemMeta.angle.value(1)).arg(cd > 0 ? cd : camera_distance,0,'f',1);
    logTrace() << QString("CAMERA POS = %1, %2, %3").arg(projection_fromx,0,'f',4).arg(projection_fromy,0,'f',4).arg(projection_fromz,0,'f',4);
    // -coX,Y,Z sets the model origin for the camera to look at.  (default = 0,0,0) TO
    logTrace() << QString("LOOK AT    = %1, %2, %3").arg(projection_towardx,0,'f',4).arg(projection_towardy,0,'f',4).arg(projection_towardz,0,'f',4);
    // -cuX,Y,Z sets the camera up vector.  (default = 0,0,1)						UP
    logTrace() << QString("UP VECTOR  = %1, %2, %3").arg(projection_upx,0,'f',4).arg(projection_upy,0,'f',4).arg(projection_upz,0,'f',4);
    // viewMatrix
    logTrace() << QString("viewMatrix = fx %1, fy %2, fz %3, tx %4, ty %5, tz %6, ux %7, uy %8, uz %9")
                  .arg(fx,0,'f',4)
                  .arg(fy,0,'f',4)
                  .arg(fz,0,'f',4)
                  .arg(tx,0,'f',4)
                  .arg(ty,0,'f',4)
                  .arg(tz,0,'f',4)
                  .arg(ux,0,'f',4)
                  .arg(uy,0,'f',4)
                  .arg(uz,0,'f',4);
   / __________________________________________________________ */
    return viewMatrix;
}


// Align the viewer camera with LPub3D default view
QVector<lcVector3> Render::nativeCameraSettings(
      LPubMeta    &lpubMeta,
      const int   &height,
      const float &cd,
      const Mt    &type)
{

    // Initialization
    double camera_latitude    = 0.0;
    double camera_longitude   = 0.0;
    double camera_distance    = 0.0;

    double projection_fromx   = 0.0;          // LPub3D default value
    double projection_fromy   = 0.0;          // LPub3D default value
    double projection_fromz   = 1000.0;       // LPub3D default value

    double projection_towardx = 0.0;          // LPub3D default value
    double projection_towardy = 1.0;          // LPub3D default value
    double projection_towardz = 0.0;          // LPub3D default value

    double projection_upx     = -0.2357;      // LPub3D default value
    double projection_upy     = -0.2357;      // LPub3D default value
    double projection_upz     = 0.94281;      // LPub3D default value

    double projection_fov     = 0.01;         // LPub3D default value
    double projection_znear   = 10.0;         // LPub3D default value
    double projection_zfar    = 4000.0;       // LPub3D default value

    if (type == PLI) {
        projection_fov   = lpubMeta.pli.fov.value();      // sets the camera FOV angle in degrees (default = 0.01f)
        projection_znear = lpubMeta.pli.znear.value();    // sets the near clipping plane.        (default = 10.0f)
        projection_zfar  = lpubMeta.pli.zfar.value();     // sets the far clipping plane.         (default = 4000.0f)

        camera_latitude  = lpubMeta.pli.angle.value(0);   // sets the camera globe latitude in degrees  (default = 0.0f) old 23
        camera_longitude = lpubMeta.pli.angle.value(1);   // sets the camera globe longitude in degrees (default = 0.0f) old -45
        camera_distance  = cd;
    }
    else
    if (type == CSI) {
        projection_fov   = lpubMeta.assem.fov.value();    // sets the camera FOV angle in degrees (default = 0.01f)
        projection_znear = lpubMeta.assem.znear.value();  // sets the near clipping plane.        (default = 10.0f)
        projection_zfar  = lpubMeta.assem.zfar.value();   // sets the far clipping plane.         (default = 4000.0f)

        camera_latitude  = lpubMeta.assem.angle.value(0); // sets the camera globe latitude in degrees  (default = 0.0f) old 23
        camera_longitude = lpubMeta.assem.angle.value(1); // sets the camera globe longitude in degrees (default = 0.0f) old -45
        camera_distance  = cd;                            // sets the camera globe radius in units      (calculated)
    }

    // Adjust camera distance
    if (camera_distance <= 0.0)
    {
      camera_distance = projection_fromz;
    }
    else // Adjust clip planes for camera_distance.
    {
      if (camera_distance > projection_fromz)
          projection_zfar = camera_distance + 3000;
      if (camera_distance > 3000)
          projection_znear = 100;
      if (camera_distance > 10000)
          projection_znear = camera_distance - 3000;
    }

    // Oblique transform.
    double la, lo;
    la = 3.1415927 * camera_latitude  / 180.0;
    lo = 3.1415927 * camera_longitude / 180.0;

    projection_fromx = camera_distance * sin(lo);
    projection_fromy = camera_distance * sin(la);
    projection_fromz = camera_distance * cos(lo);

    projection_fromx *= cos(la);
    projection_fromz *= cos(la);

    QVector<lcVector3> viewMatrix(0);
    float fx, fy, fz, tx, ty, tz, ux, uy, uz;

    // Position (camera globe)
    fx = projection_fromx;
    // Switch Y and Z coordinates to match LeoCAD. Set Y negative to match LDraw Y axis vertical with negative value in the up direction
    fy = -projection_fromz + (height/6.0);  // Move origin 2/3 way down the screen
    fz = projection_fromy;

    // Target
    tx = projection_towardx;
    ty = projection_towardy + (height/6.0); // Move origin 2/3 way down the screen
    tz = projection_towardz;

    // Up vector
    ux = projection_upx;
    uy = projection_upy;
    uz = projection_upz;

    // return camara position, target, up vector, fov, znear and zfar
    viewMatrix.append(lcVector3(fx,fy,fz));         // camera position
    viewMatrix.append(lcVector3(tx,ty,tz));         // camera target
    viewMatrix.append(lcVector3(ux,uy,uz));         // camera up vector
    viewMatrix.append(lcVector3(projection_fov,     // fov
                                projection_znear,   // znear
                                projection_zfar));  // zfar

    /* ______________DEBUG_____________________________________ /

    //-caN sets the camera FOV angle in degrees, default = 30.0f.
    logTrace() << QString("[-caN] FOV = %1").arg(projection_fov,0,'f',1);
    // -zN sets the near clipping plane.  (default = 25.0f)
    // -ZN sets the far clipping plane.  (default = 12500.0f)
    logTrace() << QString("[-zN],[-ZN] ZCLIP = %1 znear, %2 zfar").arg(projection_znear,0,'f',1).arg(projection_zfar,0,'f',1);
    // -cg<la>,<lo>,<r> sets the camera location on globe.                         FROM
    logTrace() << QString("[-cg<la>,<lo>,<r>] CAMERA GLOBE = %1, %2, %3").arg(pliMeta.angle.value(0)).arg(pliMeta.angle.value(1)).arg(cd > 0 ? cd : camera_distance,0,'f',1);
    logTrace() << QString("CAMERA POS = %1, %2, %3").arg(projection_fromx,0,'f',4).arg(projection_fromy,0,'f',4).arg(projection_fromz,0,'f',4);
    // -coX,Y,Z sets the model origin for the camera to look at.  (default = 0,0,0) TO
    logTrace() << QString("LOOK AT    = %1, %2, %3").arg(projection_towardx,0,'f',4).arg(projection_towardy,0,'f',4).arg(projection_towardz,0,'f',4);
    // -cuX,Y,Z sets the camera up vector.  (default = 0,0,1)						UP
    logTrace() << QString("UP VECTOR  = %1, %2, %3").arg(projection_upx,0,'f',4).arg(projection_upy,0,'f',4).arg(projection_upz,0,'f',4);
    // viewMatrix
    logTrace() << QString("viewMatrix = fx %1, fy %2, fz %3, tx %4, ty %5, tz %6, ux %7, uy %8, uz %9")
                  .arg(fx,0,'f',4)
                  .arg(fy,0,'f',4)
                  .arg(fz,0,'f',4)
                  .arg(tx,0,'f',4)
                  .arg(ty,0,'f',4)
                  .arg(tz,0,'f',4)
                  .arg(ux,0,'f',4)
                  .arg(uy,0,'f',4)
                  .arg(uz,0,'f',4);
   / __________________________________________________________ */

    // viewMatrix.at(0) = Position
    // viewMatrix.at(1) = TargetPosition
    // viewMatrix.at(2) = UpVector

    return viewMatrix;
}
