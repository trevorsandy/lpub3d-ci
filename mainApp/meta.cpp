
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
 * This file describes a set of classes that implement a parse tree for
 * all the meta-commands that LPub supports.  Action metas such as STEP,
 * ROTSTEP, CALLOUT BEGIN, etc. return special return codes.  Configuration
 * metas that imply no action, but specify data for later use, retain
 * the configuration information, and return a generic OK return code.
 *
 * The top of tree is the Meta class that is the interface to the traverse
 * function that walks the LDraw model hierarchy.  Meta also tracks
 * locations in files like topOfModel, bottomOfModel, bottomOfSteps,topOfRange,
 * bottomOfRange, topOfStep, bottomOfStep, etc.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "version.h"
#include <QtWidgets>
#include <QStringList>

#include "meta.h"
#include "lpub.h"
#include "lc_profile.h"

#include "pagesizes.h"

/* The token map translates known keywords to values
 * used by LPub to identify things like placement and such
 */

QHash<QString, int> tokenMap;
QHash<Rc, QRegularExpression> groupRegExMap;

bool AbstractMeta::reportErrors = false;

void AbstractMeta::init(BranchMeta *parent, QString name)
{
  preamble = parent->preamble + name + " ";
  parent->list[name] = this;
}

void AbstractMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
}

QString LeafMeta::format(
    bool local,
    bool global,
    QString suffix)
{
  QString result;

  if (local) {
    result = "LOCAL ";
  } else if (global) {
    result = "GLOBAL ";
  }

  return preamble + result + suffix;
}

BranchMeta::BranchMeta (const BranchMeta &rhs) : AbstractMeta(rhs)
{
  const QStringList keys = list.keys();
  Q_FOREACH (const QString &key, keys) {
    *list[key] = *rhs.list[key];
  }
}

BranchMeta::~BranchMeta()
{
  list.clear();
}

Rc BranchMeta::parse(QStringList &argv, int index, Where &here)
{
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    QStringList debugLine;
    qDebug() << "CAPTURE LINE CONTENTS"
    for(int i=0;i<argv.size();i++) {
        debugLine << argv[i];
        int size = argv.size();
        int incr = i;
        int result = size - incr;
        qDebug() << QString("LINE ARGV Pos:(%1), PosIndex:(%2) [%3 - %4 = %5], Value:(%6)")
                            .arg(i+1).arg(i).arg(size).arg(incr).arg(result).arg(argv[i])
                 ;
    }
    debugLine << QString(", Index (%7)[%8], LineNum (%9), ModelName (%10)")
                         .arg(index).arg(argv[index]).arg(here.modelName).arg(here.lineNumber);
    qDebug() << debugLine.join(" ");
#endif
//*/

  Rc rc;
  int offset;
  int size = argv.size();

  if (index < size) {

    /* Find out if the current argv explicitly matches any of the
   * keywords known to be valid at this point in the meta command */

    QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);

    if (i != list.end()) {
      /* We found a match */
      offset = 1;
      rc = OkRc;

      if (size - index > 1) {
        if (i.value()) {
          if (argv[index+offset] == "LOCAL") {
            i.value()->pushed = true;
            offset++;
          } else if (argv[index+offset] == "GLOBAL") {
            i.value()->global = true;
            offset++;
          }
          if (index + offset >= size) {
            rc = FailureRc;
          }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
          QString iVal = QString("argv[index+offset] (%1) [Index: %2, Offset: 1]")
                                 .arg(argv[index+1]).arg(index);
          qDebug() << "\nMETA.CPP: BRANCHMETA::PARSE -"
                   << "\nPREAMBLE:" << QString(i.value()->preamble).replace("0 !LPUB ","")
                   << "\nDATA -"
                   << "\n ARGV:      " << argv.join(" ")
                   << "\n I.VALUE(): " << iVal
                   << "\nOTHER DATA -"
                   << "\n Pushed:   [" << i.value()->pushed << "]"
                   << "\n Global:    " << (i.value()->global ? "True" : "False")
                   << "\n Index:     " << index
                   << "\n Offset:    1"
                        ;
#endif
//*/
        }
      }
      /* Now parse the rest of the argvs */
      if (rc == OkRc) {
        return i.value()->parse(argv,index+offset,here);
      }
    } else if (size - index > 1) {
      /* Failed an explicit match.  Lets try to see if the value
       * matches any of the keywords through regular expressions */
      bool local  = argv[index] == "LOCAL";
      bool global = argv[index] == "GLOBAL";

      // LPUB CALLOUT Vertical
      // LPUB CALLOUT LOCAL Vertical

      offset = local || global;

      if (index + offset < size) {
        static QRegularExpression rx;
        for (i = list.begin(); i != list.end(); i++) {
          rx.setPattern(i.key());
          if (argv[index + offset].contains(rx)) {
            /* Now parse the rest of the argvs */
            i.value()->pushed = local;
            i.value()->global = global;
            return i.value()->parse(argv,index+offset,here);
          }
        }
      }
    }
  }

  // syntax error
  return FailureRc;
}

/*
 * Find out if the match string matches the syntax graph up until this
 * point
 */

bool BranchMeta::preambleMatch(QStringList &argv, int index, QString &match)
{
  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    return false;
  } else {
    return i.value()->preambleMatch(argv,index,match);
  }
}

/*
 * Output documentation information for this node in the syntax
 * graph
 */

void BranchMeta::doc(QStringList &out, QString preamble)
{
  QString key;
  QStringList keys = list.keys();
  keys.sort();
  Q_FOREACH (key, keys) {
    list[key]->doc(out, preamble + " " + key);
  }
}

/*
 * Output command information for this node in the syntax
 * graph
 */
void BranchMeta::metaKeywords(QStringList &out, QString preamble)
{
  QStringList keys = list.keys();
  keys.sort();
  Q_FOREACH (const QString &key, keys) {
    list[key]->metaKeywords(out, preamble + " " + key);
  }
}

void BranchMeta::pop()
{
  const QStringList keys = list.keys();
  Q_FOREACH (const QString &key, keys) {
    list[key]->pop();
  }
}

/* ------------------ */
void RcMeta::init(BranchMeta *parent, const QString name, Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc RcMeta::parse(QStringList &argv, int index, Where &here)
{
  Q_UNUSED(argv)
  Q_UNUSED(index)
  _here[pushed] = here;
  return rc;
}

void RcMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
}

void RcMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

void IntMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc IntMeta::parse(QStringList &argv, int index,Where &here)
{
  int size = argv.size();
  if ((size - 1) >= index) { // changed from index == (size-1) to (size-1) >= index for LightMeta
    bool ok;
    int v;
    v = argv[index].toInt(&ok);
    if (ok) {
      if (v < _min || v > _max) {
        return RangeErrorRc;
      }
      _value[pushed] = v;
      _here[pushed] = here;
      return rc;
    }
  }

  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected a whole number but got \"%1\" %2") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString IntMeta::format(bool local, bool global)
{
  QString foo = QString("%1").arg(_value[pushed],0,base);

  return LeafMeta::format(local,global,foo);
}

void IntMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <integer>";
}

/* ------------------ */

void FloatMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}

Rc FloatMeta::parse(QStringList &argv, int index,Where &here)
{
  int size = argv.size();
  if ((size - 1) >= index) { // changed from index == (size-1) to (size-1) >= index for LightMeta
    bool ok;
    float v = argv[index].toFloat(&ok);
    if (ok) {
      if (v < _min || v > _max) {
        return RangeErrorRc;
      }
      _value[pushed] = v;
      _here[pushed] = here;
      return rc;
    }
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected a floating point number but got \"%1\" %2") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString FloatMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString("%1") .arg(double(value()),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo.trimmed());
}
void FloatMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <decimal>";
}

/* ------------------ */

QString UnitMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString("%1") .arg(double(valueInches()),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo.trimmed());
}

int UnitMeta::valuePixels()
{
  float t = _value[pushed];
  return int(t*resolution());
}


/* ------------------ */
int UnitsMeta::valuePixels(int which)
{
  float t = _value[pushed][which];
  float r = resolution();

  return int(t*r);
}

void UnitsMeta::setValuePixels(int which, int pixels)
{
  float r = resolution();
  float value = float(pixels/r);

  _value[pushed][which] = value;
}

QString UnitsMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString("%1 %2")
      .arg(double(valueInches(0)),_fieldWidth,'f',_precision)
      .arg(double(valueInches(1)),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo.trimmed());
}

/* ------------------ */

void FloatPairMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}

Rc FloatPairMeta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() - index == 2) {
    bool ok[2];
    float v0 = argv[index  ].toFloat(&ok[0]);
    float v1 = argv[index+1].toFloat(&ok[1]);
    if (ok[0] && ok[1]) {
      if (v0 < _min || v0 > _max ||
        v1 < _min || v1 > _max) {
        return RangeErrorRc;
      }
      _value[pushed][0] = v0;
      _value[pushed][1] = v1;
      _here[pushed] = here;
      return rc;
    }
  }

  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected two floating point numbers but got \"%1\" %2") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString FloatPairMeta::format(bool local, bool global)
{
  QString foo = QString("%1 %2")
      .arg(double(_value[pushed][0]),_fieldWidth,'f',_precision)
      .arg(double(_value[pushed][1]),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo.trimmed());
}
void FloatPairMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <decimal> <decimal>";
}

/* ------------------ */

void Vector3Meta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc Vector3Meta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() - index == 3) {
    bool ok[3];
    float x = argv[index  ].toFloat(&ok[0]);
    float y = argv[index+1].toFloat(&ok[1]);
    float z = argv[index+2].toFloat(&ok[2]);
    if (ok[0] && ok[1] && ok[2]) {
      if (x < _min || x > _max ||
          y < _min || y > _max ||
          z < _min || z > _max) {
          return RangeErrorRc;
        }
      _x[pushed] = x;
      _y[pushed] = y;
      _z[pushed] = z;
      _here[pushed] = here;
      _populated    = !(x == 0.0f && y == 0.0f && z == 0.0f);
      return rc;
    }
  }

  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected three floating point numbers but got \"%1\" %2") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString Vector3Meta::format(bool local, bool global)
{
  QString foo = QString("%1 %2 %3")
                        .arg(double(_x[pushed]),_fieldWidth,'f',_precision)
                        .arg(double(_y[pushed]),_fieldWidth,'f',_precision)
                        .arg(double(_z[pushed]),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo.trimmed());
}
void Vector3Meta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <decimal X> <decimal Y> <decimal Z>";
}

/* ------------------ */

void Vector33Meta::init(BranchMeta *parent, const QString name, Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc Vector33Meta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() - index == 9) {
    bool ok[9];

    float x1 = argv[index  ].toFloat(&ok[0]);
    float y1 = argv[index+1].toFloat(&ok[1]);
    float z1 = argv[index+2].toFloat(&ok[2]);

    float x2 = argv[index+3].toFloat(&ok[3]);
    float y2 = argv[index+4].toFloat(&ok[4]);
    float z2 = argv[index+5].toFloat(&ok[5]);

    float x3 = argv[index+6].toFloat(&ok[6]);
    float y3 = argv[index+7].toFloat(&ok[7]);
    float z3 = argv[index+8].toFloat(&ok[8]);

    if (ok[0] && ok[1] && ok[2] &&
      ok[3] && ok[4] && ok[5] &&
      ok[6] && ok[7] && ok[8]) {
      if (x1 < _min || x1 > _max ||
        y1 < _min || y1 > _max ||
        z1 < _min || z1 > _max ||
        x2 < _min || x2 > _max ||
        y2 < _min || y2 > _max ||
        z2 < _min || z2 > _max ||
        x3 < _min || x3 > _max ||
        y3 < _min || y3 > _max ||
        z3 < _min || z3 > _max) {
        return RangeErrorRc;
      }

      _x1[pushed] = x1;
      _y1[pushed] = y1;
      _z1[pushed] = z1;

      _x2[pushed] = x2;
      _y2[pushed] = y2;
      _z2[pushed] = z2;

      _x3[pushed] = x3;
      _y3[pushed] = y3;
      _z3[pushed] = z3;

      _here[pushed] = here;
      _populated    = !(x1 == 0.0f && y1 == 0.0f && z1 == 0.0f &&
                        x2 == 0.0f && y2 == 0.0f && z2 == 0.0f &&
                        x3 == 0.0f && y3 == 0.0f && z3 == 0.0f);
      return rc;
    }
  }

  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected three floating point numbers but got \"%1\" %2") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString Vector33Meta::format(bool local, bool global)
{
  QString foo = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9")
      .arg(double(_x1[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_y1[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_z1[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_x2[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_y2[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_z2[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_x3[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_y3[pushed]),_fieldWidth,'f',_precision)
      .arg(double(_z3[pushed]),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo.trimmed());
}
void Vector33Meta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <decimal X1> <decimal Y1> <decimal Z1> <decimal X2> <decimal Y2> <decimal Z2> <decimal X3> <decimal Y3> <decimal Z3>";
}

/* ------------------ */

void StringMeta::init(
    BranchMeta *parent,
    QString name,
    Rc _rc,
    QString _delim)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
  delim = _delim;
}
Rc StringMeta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() - index >= 1) {  // changed operator from == to >= for LightMeta
    //_value[pushed] = argv[index].replace("\\""","""");
    _value[pushed] = argv[index];
    _here[pushed] = here;
    return rc;
  }

  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected a string after \"%1\"") .arg(argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString StringMeta::format(bool local, bool global)
{
  QString foo = delim + _value[pushed] + delim;
  return LeafMeta::format(local,global,foo);
}

void StringMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <\"string\">";
}

/* ------------------ */

void StringListMeta::init(
    BranchMeta *parent,
    QString name,
    Rc _rc,
    QString _delim)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
  delim = _delim;
}
Rc StringListMeta::parse(QStringList &argv, int index,Where &here)
{
  _value[pushed].clear();
  for (int i = index; i < argv.size(); i++) {
    _value[pushed] << argv[i];
  }
  _here[pushed] = here;
  return rc;
}
QString StringListMeta::format(bool local, bool global)
{
  QString foo;
  for (int i = 0; i < _value[pushed].size() ; i++) {
    foo += delim + _value[pushed][i] + delim + " ";
  }
  return LeafMeta::format(local,global,foo);
}

void StringListMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <\"string\"> <\"string\"> ...";
}

/* ------------------ */

Rc BoolMeta::parse(QStringList &argv, int index,Where &here)
{
  static QRegularExpression rx("^(TRUE|FALSE)$");
  if (index == argv.size() - 1 && argv[index].contains(rx)) {
      _value[pushed] = argv[index] == "TRUE";
      _here[pushed] = here;
      return OkRc;
    }

  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected TRUE or FALSE \"%1\" %2") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
  }

  return FailureRc;
}
QString BoolMeta::format(bool local, bool global)
{
  QString foo (_value[pushed] ? "TRUE" : "FALSE");
  return LeafMeta::format(local,global,foo);
}

void BoolMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRUE | FALSE )";
}

/* ------------------ */

const QString placementOptions[][3] =
{
  { "TOP_LEFT",    "",      "OUTSIDE"  },
  { "TOP",         "LEFT",  "OUTSIDE"  },
  { "TOP",         "CENTER","OUTSIDE"  },
  { "TOP",         "RIGHT", "OUTSIDE"  },
  { "TOP_RIGHT",   "",      "OUTSIDE"  },

  { "LEFT",        "TOP",   "OUTSIDE"  },
  { "TOP_LEFT",    "",      "INSIDE"   },
  { "TOP",         "",      "INSIDE"   },
  { "TOP_RIGHT",   "",      "INSIDE"   },
  { "RIGHT",       "TOP",   "OUTSIDE"  },

  { "LEFT",        "CENTER","OUTSIDE"  },
  { "LEFT",        "",      "INSIDE"   },
  { "CENTER",      "",      "INSIDE"   },
  { "RIGHT",       "",      "INSIDE"   },
  { "RIGHT",       "CENTER","OUTSIDE"  },

  { "LEFT",        "BOTTOM", "OUTSIDE" },
  { "BOTTOM_LEFT", "",       "INSIDE"  },
  { "BOTTOM",      "",       "INSIDE"  },
  { "BOTTOM_RIGHT","",       "INSIDE"  },
  { "RIGHT",       "BOTTOM", "OUTSIDE" },

  { "BOTTOM_LEFT", "",       "OUTSIDE" },
  { "BOTTOM",      "LEFT",   "OUTSIDE" },
  { "BOTTOM",      "CENTER", "OUTSIDE" },
  { "BOTTOM",      "RIGHT",  "OUTSIDE" },
  { "BOTTOM_RIGHT","",       "OUTSIDE" }
};

int placementDecode[][3] =
{
  { TopLeft,     Center, Outside },     //00
  { Top,         Left,   Outside },     //01
  { Top,         Center, Outside },     //02
  { Top,         Right,  Outside },     //03
  { TopRight,    Center, Outside },     //04

  { Left,        Top,    Outside },     //05
  { TopLeft,     Center, Inside  },     //06 "Page Top Left"
  { Top,         Center, Inside  },     //07 "Page Top"
  { TopRight,    Center, Inside  },     //08 "Page Top Right"
  { Right,       Top,    Outside },     //09

  { Left,        Center, Outside },     //10
  { Left,        Center, Inside  },     //11 "Page Left"
  { Center,      Center, Inside  },     //12 "Page Center"
  { Right,       Center, Inside  },     //13 "Page Right"
  { Right,       Center, Outside },     //14

  { Left,        Bottom, Outside },     //15
  { BottomLeft,  Center, Inside  },     //16 "Page Bottom Left"
  { Bottom,      Center, Inside  },     //17 "Page Bottom"
  { BottomRight, Center, Inside  },     //18 "Page Bottom Right"
  { Right,       Bottom, Outside },     //19

  { BottomLeft,  Center, Outside },     //20
  { Bottom,      Left,   Outside },     //21
  { Bottom,      Center, Outside },     //22
  { Bottom,      Right,  Outside },     //23
  { BottomRight, Center, Outside }      //24
};

const QString relativeNames[] =
{
  "PAGE","ASSEM","MULTI_STEP","STEP_NUMBER","PLI","CALLOUT","PAGE_NUMBER",
  "DOCUMENT_TITLE","MODEL_ID","DOCUMENT_AUTHOR","PUBLISH_URL","MODEL_DESCRIPTION",
  "PUBLISH_DESCRIPTION","PUBLISH_COPYRIGHT","PUBLISH_EMAIL","LEGO_DISCLAIMER",
  "MODEL_PARTS","APP_PLUG","SUBMODEL_INST_COUNT","DOCUMENT_LOGO","DOCUMENT_COVER_IMAGE",
  "APP_PLUG_IMAGE","PAGE_HEADER","PAGE_FOOTER","MODEL_CATEGORY","SUBMODEL_DISPLAY",
  "ROTATE_ICON","ASSEM_PART","STEP","RANGE","TEXT","BOM","PAGE_POINTER","SINGLE_STEP","RESERVE",
  "COVER_PAGE","ANNOTATION","DIVIDER_POINTER"
};

PlacementMeta::PlacementMeta() : LeafMeta()
{
  _value[0].placement     = PlacementEnc(placementDecode[TopLeftInsideCorner][0]);
  _value[0].justification = PlacementEnc(placementDecode[TopLeftInsideCorner][1]);
  _value[0].relativeTo    = PageType;
  _value[0].preposition   = PrepositionEnc(placementDecode[TopLeftInsideCorner][2]);
  _value[0].rectPlacement = TopLeftInsideCorner;
  _value[0].offsets[0]    = 0.0f;
  _value[0].offsets[1]    = 0.0f;
}

void PlacementMeta::setValue(
    RectPlacement placement,
    PlacementType relativeTo)
{
  _value[pushed].placement     = PlacementEnc(placementDecode[placement][0]);
  _value[pushed].justification = PlacementEnc(placementDecode[placement][1]);
  _value[pushed].relativeTo    = relativeTo;
  _value[pushed].preposition   = PrepositionEnc(placementDecode[placement][2]);
  _value[pushed].rectPlacement = placement;
}

Rc PlacementMeta::parse(QStringList &argv, int index,Where &here)
{
  float _offsets[2];
  Rc rc = FailureRc;
  int argc = argv.size();
  QString relativeTos = "^(PAGE|ASSEM|MULTI_STEP|STEP_NUMBER|PLI|CALLOUT|PAGE_NUMBER|"
                        "DOCUMENT_TITLE|MODEL_ID|DOCUMENT_AUTHOR|PUBLISH_URL|MODEL_DESCRIPTION|"
                        "PUBLISH_DESCRIPTION|PUBLISH_COPYRIGHT|PUBLISH_EMAIL|LEGO_DISCLAIMER|"
                        "MODEL_PARTS|APP_PLUG|MODEL_CATEGORY|DOCUMENT_LOGO|DOCUMENT_COVER_IMAGE|"
                        "APP_PLUG_IMAGE|PAGE_HEADER|PAGE_FOOTER|MODEL_CATEGORY|SUBMODEL_DISPLAY|"
                        "ROTATE_ICON|ASSEM_PART|STEP|RANGE|TEXT|BOM|PAGE_POINTER|SINGLE_STEP|RESERVE|"
                        "COVER_PAGE|ANNOTATION|DIVIDER_POINTER)$";
  _offsets[0]    = 0;
  _offsets[1]    = 0;

/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  qDebug() << "\nMETA.CPP: PLACEMENT META PARSE -";
  for(int i=0;i<argv.size();i++) {
      int size = argv.size();
      int incr = i;
      int result = size - incr;

     qDebug() << "\nPAGE ARGV pos:(" << i+1 << ") index:(" << i << ") [" << size << " - " << incr << " = " << result << "] " << argv[i]
     ;
  }
  qDebug() << "\nWHERE: " << here.modelName << ", Line: " << here.lineNumber
           << "\nSIZE: " << argv.size() << ", INDEX: " << index
           << "\nSTART - Value at index: " << argv[index]
           ;
#endif
//*/

/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  qDebug() << "\nMETA.CPP: PLACEMENTMETA::PARSE -"
           << "\nPREAMBLE:" << QString(preamble).replace("0 !LPUB ","")
           << "\nPLACEMENT WHERE -"
           << "\n Here(Model Name): " << here.modelName
           << "\n Here(Line Number):" << here.lineNumber
           << "\nDATA -"
           << "\n ARGV:             " << argv.join(" ")
           << "\nOTHER DATA -"
           << "\n Pushed:          [" << pushed << "]"
           << "\n Global:           " << (global ? "True" : "False")
           << "\n Index:            " << index
              ;
#endif
  //*/
  if (argv[index] == "OFFSET") {
    index++;
    if (argc - index == 2) {
      bool ok[2];
      _value[pushed].offsets[0] = argv[index  ].toFloat(&ok[0]);
      _value[pushed].offsets[1] = argv[index+1].toFloat(&ok[1]);
      if (!ok[0] || !ok[1])
          return rc;
      _here[pushed] = here;
      return OkRc;
    } else if (argc - index < 2) {
      return rc;
    }
  }

  RectPlacement _placementR; //_value[pushed].rectPlacement;
  PlacementType _relativeTo; //_value[pushed].relativeTo;
  QString placement, justification, preposition, relativeTo;

  static QRegularExpression rx("^(TOP|BOTTOM)$");
  if (argv[index].contains(rx)) {
    placement = argv[index++];

    if (index < argc) {
      rx.setPattern("^(LEFT|CENTER|RIGHT)$");
      if (argv[index].contains(rx)) {
        justification = argv[index++];
        rc = OkRc;
      } else {
        rx.setPattern(relativeTos);
        if (argv[index].contains(rx)) {
          rc = OkRc;
        }
      }
    }
  } else {
    rx.setPattern("^(LEFT|RIGHT)$");
    if (argv[index].contains(rx)) {
      placement = argv[index++];

      if (index < argc) {
        rx.setPattern("^(TOP|CENTER|BOTTOM)$");
        if (argv[index].contains(rx)) {
          justification = argv[index++];
          rc = OkRc;
        } else {
          rx.setPattern(relativeTos);
          if (argv[index].contains(rx)) {
            rc = OkRc;
          }
        }
      }
    } else {
      rx.setPattern("^(TOP_LEFT|TOP_RIGHT|BOTTOM_LEFT|BOTTOM_RIGHT|CENTER)$");
      if (argv[index].contains(rx)) {
        placement = argv[index++];
        rc = OkRc;
      } else {
        return rc;
      }
    }
  }

  if (rc == OkRc && index < argv.size()) {
    rx.setPattern(relativeTos);
    if (argv[index].contains(rx)) {
      relativeTo = argv[index++];
      if (index < argc) {
        rx.setPattern("^(INSIDE|OUTSIDE)$");
        if (argv[index].contains(rx)) {
          preposition = argv[index++];
        }
        if (argc - index >= 2) {
          if (argv[index] == "OFFSET")
            index++;
          bool ok[2];
          _offsets[0] = argv[index  ].toFloat(&ok[0]);
          _offsets[1] = argv[index+1].toFloat(&ok[1]);
          if (!ok[0] || !ok[1])
            return FailureRc;
        }
      }
    }

    int i;

    if (preposition == "INSIDE" && justification == "CENTER") {
      justification = "";
    }

    for (i = 0; i < NumSpots; i++) {
      if (placementOptions[i][0] == placement &&
        placementOptions[i][1] == justification &&
        placementOptions[i][2] == preposition) {
        break;
      }
    }

    if (i == NumSpots) {
      return FailureRc;
    }

    _placementR = RectPlacement(i);
    _relativeTo = PlacementType(tokenMap[relativeTo]);
    setValue(_placementR,_relativeTo);
    _value[pushed].offsets[0] = _offsets[0];
    _value[pushed].offsets[1] = _offsets[1];
    _here[pushed] = here;
  }
  return rc;
}

const QString bRectPlacementNames[] =
{
  "","","","","","",
  "BASE_TOP_LEFT ","BASE_TOP","BASE_TOP_RIGHT",
  "","",
  "BASE_LEFT","BASE_CENTER","BASE_RIGHT",
  "","",
  "BASE_BOTTOM_LEFT","BASE_BOTTOM","BASE_BOTTOM_RIGHT",
  "","","","","",""
};

const QString bPlacementEncNames[] =
{
  "BASE_TOP_LEFT", "BASE_TOP", "BASE_TOP_RIGHT",
  "BASE_RIGHT", "BASE_BOTTOM_RIGHT", "BASE_BOTTOM",
  "BASE_BOTTOM_LEFT", "BASE_LEFT", "BASE_CENTER",""
};

const QString placementNames[] =
{
  "TOP_LEFT", "TOP", "TOP_RIGHT",
  "RIGHT", "BOTTOM_RIGHT", "BOTTOM",
  "BOTTOM_LEFT", "LEFT", "CENTER",""
};

const QString prepositionNames[] =
{
  "INSIDE", "OUTSIDE"
};

QString PlacementMeta::format(bool local, bool global)
{
  QString foo;

  if (_value[pushed].preposition == Inside) {
    switch (_value[pushed].placement) {
      case Top:
      case Bottom:
      case Right:
      case Left:
        foo = placementNames[_value[pushed].placement] + " "
            + relativeNames [_value[pushed].relativeTo] + " "
            + prepositionNames[_value[pushed].preposition];
        break;
      default:
        foo = placementNames[_value[pushed].placement] + " "
            + relativeNames [_value[pushed].relativeTo] + " "
            + prepositionNames[_value[pushed].preposition];
    }
  } else {

    switch (_value[pushed].placement) {
      case Top:
      case Bottom:
      case Right:
      case Left:
        foo = placementNames[_value[pushed].placement] + " "
            + placementNames[_value[pushed].justification] + " "
            + relativeNames [_value[pushed].relativeTo] + " "
            + prepositionNames[_value[pushed].preposition];
        break;
      default:
        foo = placementNames[_value[pushed].placement] + " "
            + relativeNames [_value[pushed].relativeTo] + " "
            + prepositionNames[_value[pushed].preposition];
    }
  }
  if (_value[pushed].offsets[0] != 0.0f || _value[pushed].offsets[1] != 0.0f) {
    QString bar = QString(" OFFSET %1 %2")
                          .arg(double(_value[pushed].offsets[0]),0,'f',4)
                          .arg(double(_value[pushed].offsets[1]),0,'f',4);
    foo += bar;
  }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  qDebug()  << "\nMETA.CPP: PLACEMENT META FORMAT: "
            << "\nPREAMBLE:" << QString(preamble).replace("0 !LPUB ","")
            << "\nPUSHED VALUES:"
            << " \nPlacement:                  " << PlacNames[_value[pushed].placement]     << " (" << _value[pushed].placement << ")"
            << " \nJustification:              " << PlacNames[_value[pushed].justification] << " (" << _value[pushed].justification << ")"
            << " \nRelativeTo:                 " << RelNames[_value[pushed].relativeTo]     << " (" << _value[pushed].relativeTo << ")"
            << " \nPreposition:                " << PrepNames[_value[pushed].preposition]   << " (" << _value[pushed].preposition << ")"
            << " \nRectPlacement:              " << RectNames[_value[pushed].rectPlacement] << " (" << _value[pushed].rectPlacement << ")"
            << " \nOffset[0]:                  " << _value[pushed].offsets[0]
            << " \nOffset[0]:                  " << _value[pushed].offsets[1]
            << "\nPLACEMENT WHERE -"
            << " \nPlacement Here(Model Name): " << _here[pushed].modelName
            << " \nPlacement Here(Line Number):" << _here[pushed].lineNumber
            << "\nOTHER DATA -                 "
            << " \n:Pushed:                   [" << pushed << "]"
            << " \n:Local:                     " << (local ? "True" : "False")
            << " \n:Global:                    " << (global ? "True" : "False")
            << "\nFORMATTED META -"
            << "\n Meta Format:               " << LeafMeta::format(local,global,foo)
               ;
#endif
//*/
  return LeafMeta::format(local,global,foo);
}

void PlacementMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (( TOP | BOTTOM ) | ( LEFT | CENTER | RIGHT )) | ( TOP_LEFT | TOP_RIGHT | BOTTOM_LEFT | BOTTOM_RIGHT )"
                    " ( PAGE | ASSEM ( INSIDE | OUTSIDE ) | MULTI_STEP | STEP_NUMBER | PLI | SUBMODEL_DISPLAY | ROTATE_ICON | CALLOUT )";
}

void PlacementMeta::metaKeywords(QStringList &out, QString preamble)
{

  out << preamble + " TOP BOTTOM LEFT CENTER RIGHT TOP_LEFT TOP_RIGHT BOTTOM_LEFT BOTTOM_RIGHT"
                    " PAGE ASSEM INSIDE OUTSIDE MULTI_STEP STEP_NUMBER PLI SUBMODEL_DISPLAY ROTATE_ICON CALLOUT";
}
/* ------------------ */

Rc BackgroundMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  if (argv.size() - index == 1) {
    if (argv[index] == "TRANS" || argv[index] == "TRANSPARENT") {
      _value[pushed].type = BackgroundData::BgTransparent;
      rc = OkRc;
    } else if (argv[index] == "SUBMODEL_BACKGROUND_COLOR") {
      _value[pushed].type = BackgroundData::BgSubmodelColor;
      rc = OkRc;
    } else {
      _value[pushed].type = BackgroundData::BgImage;
      _value[pushed].string = argv[index];
      _value[pushed].stretch = false;
      rc = OkRc;
    }
  } else if (argv.size() - index == 2) {
    if (argv[index] == "COLOR") {
      _value[pushed].type = BackgroundData::BgColor;
      _value[pushed].string = argv[index+1];
      rc = OkRc;
    } else if (argv[index] == "PICTURE") {
      _value[pushed].type = BackgroundData::BgImage;
      _value[pushed].string = argv[index+1];
      _value[pushed].stretch = false;
      rc = OkRc;
    }
  } else if (argv.size() - index == 3) {
    if (argv[index] == "PICTURE" && argv[index+2] == "STRETCH") {
      _value[pushed].type = BackgroundData::BgImage;
      _value[pushed].string = argv[index+1];
      _value[pushed].stretch = true;
      rc = OkRc;
    }
  } else if (argv.size() - index == 9) {

    if (argv[index] == "GRADIENT") {

      bool ok[6];
      bool pass = true;
      argv[index+1].toInt(&ok[0]);
      argv[index+2].toInt(&ok[1]);
      argv[index+3].toInt(&ok[2]);
      argv[index+4].toFloat(&ok[3]);
      argv[index+5].toFloat(&ok[4]);
      argv[index+6].toFloat(&ok[5]);

      const QStringList _gpoints = argv[index+7].split("|");
      QVector<QPointF> gpoints;
      Q_FOREACH(const QString &gpoint, _gpoints) {
        bool ok[2];
        int x = gpoint.section(',',0,0).toInt(&ok[0]);
        int y = gpoint.section(',',1,1).toInt(&ok[1]);
        if (ok[0] && ok[1])
          gpoints << QPointF(x, y);
        else if (pass)
          pass = false;
      }
      const QStringList _gstops  = argv[index+8].split("|");
      QVector<QPair<qreal,QColor> > gstops;
      Q_FOREACH(const QString &_gstop, _gstops) {
        bool ok[2];
        qreal point  = _gstop.section(',',0,0).toDouble(&ok[0]);
        unsigned int rgba = _gstop.section(',',1,1).toUInt(&ok[1],16);
        if (ok[0] && ok[1])
          gstops.append(qMakePair(point, QColor::fromRgba(rgba)));
        else if (pass)
          pass = false;
      }

      if (ok[0] && ok[1] && ok[2] &&
        ok[3] && ok[4] && ok[5] && pass) {

        int _gmode   = argv[index+1].toInt(&ok[0]);
        int _gspread = argv[index+2].toInt(&ok[1]);
        int _gtype   = argv[index+3].toInt(&ok[2]);

        _value[pushed].type = BackgroundData::BgGradient;

        switch (_gmode) {
          case 0:
            _value[pushed].gmode = BackgroundData::LogicalMode;
          break;
          case 1:
            _value[pushed].gmode = BackgroundData::StretchToDeviceMode;
          break;
          case 2:
            _value[pushed].gmode = BackgroundData::ObjectBoundingMode;
          break;
        }
        switch (_gspread) {
          case 0:
            _value[pushed].gspread = BackgroundData::PadSpread;
          break;
          case 1:
            _value[pushed].gspread = BackgroundData::RepeatSpread;
          break;
          case 2:
            _value[pushed].gspread = BackgroundData::ReflectSpread;
          break;
        }
        switch (_gtype) {
          case 0:
            _value[pushed].gtype = BackgroundData::LinearGradient;
          break;
          case 1:
            _value[pushed].gtype = BackgroundData::RadialGradient;
          break;
          case 2:
            _value[pushed].gtype = BackgroundData::ConicalGradient;
          break;
          case 3:
          break;
        }
        _value[pushed].gsize[0] = argv[index+4].toInt(&ok[3]);
        _value[pushed].gsize[1] = argv[index+5].toInt(&ok[4]);
        _value[pushed].gangle   = argv[index+6].toInt(&ok[5]);
        _value[pushed].gpoints  = gpoints;
        _value[pushed].gstops   = gstops;
        rc = OkRc;
      }
    }
  }
  if (rc == OkRc) {
    _here[pushed] = here;
    return rc;
  } else {

    if (reportErrors) {
      QString const message = QMessageBox::tr("Malformed background \"%1\"") .arg(argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
    }

    return FailureRc;
  }
}

QString BackgroundMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value[pushed].type) {
    case BackgroundData::BgTransparent:
      foo = "TRANSPARENT";
      break;
    case BackgroundData::BgSubmodelColor:
      foo = "SUBMODEL_BACKGROUND_COLOR";
      break;
    case BackgroundData::BgColor:
      foo = "COLOR \"" + _value[pushed].string + "\"";
      break;
    case BackgroundData::BgGradient:
      {
        QString points;
        const QVector<QPointF> _points = _value[pushed].gpoints;
        Q_FOREACH(const QPointF &point, _points) {
          points += QString("%1,%2|").arg(point.x()).arg(point.y());
        }
        points = points.remove(points.size()-1,1);

        QString stops;
        const QVector<QPair<qreal,QColor> > _gstops = _value[pushed].gstops;
        typedef QPair<qreal,QColor> _gstop;
        Q_FOREACH(const _gstop &gstop, _gstops) {
//          if (gstop.second.name() != "#000000")
//          gstop.second.name().replace("#","0xff");
          stops += QString("%1,%2|")
                           .arg(gstop.first)
                           .arg(gstop.second.name().replace("#","0xff"));
        }
        stops = stops.remove(stops.size()-1,1);

        foo = QString("GRADIENT %1 %2 %3 %4 %5 %6 \"%7\" \"%8\"")
            .arg(_value[pushed].gmode)
            .arg(_value[pushed].gspread)
            .arg(_value[pushed].gtype)
            .arg(_value[pushed].gsize[0])
            .arg(_value[pushed].gsize[1])
            .arg(_value[pushed].gangle)
            .arg(points, stops);
      }
      break;
    case BackgroundData::BgImage:
      foo = "PICTURE \"" + _value[pushed].string + "\"";
      if (_value[pushed].stretch) {
          foo += " STRETCH";
        }
      break;
    }
  return LeafMeta::format(local,global,foo);
}

void BackgroundMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRANSPARENT | SUBMODEL_BACKGROUND_COLOR | COLOR <\"color name\"|\"#RRGGBB\"> | "
                    "GRADIENT <mode spread type sizeX sizeY angle \"points\" \"stops\"> | "
                    "PICTURE <\"file path\"> [ STRETCH ])";
}

void BackgroundMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " TRANSPARENT SUBMODEL_BACKGROUND_COLOR COLOR GRADIENT PICTURE STRETCH";
}

QString BackgroundMeta::text()
{
  BackgroundData background = value();
  switch (background.type) {
    case BackgroundData::BgTransparent:
      return "Transparent";
    case BackgroundData::BgImage:
      return "Picture " + background.string;
    case BackgroundData::BgColor:
      return "Color " + background.string;
    case BackgroundData::BgGradient:
      return "Gradient " + background.string;
    default:
      break;
  }
  return "Submodel Level Color";
}

Rc BorderMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  bool newFormat   = false;
  if (argv.size() - index >= 1) {
    argv[index+1].toInt(&newFormat);
  }
  if (argv[index] == "NONE") {
    _value[pushed].type = BorderData::BdrNone;
    if (newFormat) {
      _value[pushed].line = setBorderLine(argv[index+1]);
    } else {
      _value[pushed].line = BorderData::BdrLnSolid;
    }
    index++;
    rc = OkRc;
  } else if (newFormat && argv[index] == "HIDDEN" && argv.size() - index >= 4) {
    rc = FailureRc;
    bool ok[2];
    argv[index+1].toInt(&ok[0]);
    argv[index+3].toFloat(&ok[1]);
    if (ok[0] && ok[1]) {
      _value[pushed].hideTip    = true;
      _value[pushed].type       = BorderData::BdrSquare;
      _value[pushed].line       = setBorderLine(argv[index+1]);
      _value[pushed].color      = argv[index+2];
      _value[pushed].thickness  = argv[index+3].toFloat(&ok[1]);
      index += 4;
      rc = OkRc;
    }
  } else if (argv[index] == "HIDDEN" && argv.size() - index >= 3) {
    rc = FailureRc;
    bool ok;
    argv[index+2].toFloat(&ok);
    if (ok) {
      _value[pushed].hideTip = true;
      _value[pushed].type       = BorderData::BdrSquare;
      _value[pushed].line       = BorderData::BdrLnSolid;
      _value[pushed].color      = argv[index+1];
      _value[pushed].thickness  = argv[index+2].toFloat(&ok);
      index += 3;
      rc = OkRc;
    }
  } else if (newFormat && argv[index] == "SQUARE" && argv.size() - index >= 4) {
    rc = FailureRc;
    bool ok[2];
    argv[index+1].toInt(&ok[0]);
    argv[index+3].toFloat(&ok[1]);
    if (ok[0] && ok[1]) {
      _value[pushed].type      = BorderData::BdrSquare;
      _value[pushed].line      = setBorderLine(argv[index+1]);
      _value[pushed].color     = argv[index+2];
      _value[pushed].thickness = argv[index+3].toFloat(&ok[1]);
      index += 4;
      rc = OkRc;
    }
  } else if (argv[index] == "SQUARE" && argv.size() - index >= 3) {
    rc = FailureRc;
    bool ok;
    argv[index+2].toFloat(&ok);
    if (ok) {
      _value[pushed].type      = BorderData::BdrSquare;
      _value[pushed].line      = BorderData::BdrLnSolid;
      _value[pushed].color     = argv[index+1];
      _value[pushed].thickness = argv[index+2].toFloat(&ok);
      index += 3;
      rc = OkRc;
    }
  } else if (newFormat && argv[index] == "ROUND" && argv.size() - index >= 5) {
    rc = FailureRc;
    bool ok[3];
    argv[index+1].toInt(&ok[0]);
    argv[index+3].toFloat(&ok[1]);
    argv[index+4].toFloat(&ok[2]);
    if (ok[0] && ok[1] && ok[2]) {
        _value[pushed].type      = BorderData::BdrRound;
        _value[pushed].line      = setBorderLine(argv[index+1]);
        _value[pushed].color     = argv[index+2];
        _value[pushed].thickness = argv[index+3].toFloat(&ok[0]);
        _value[pushed].radius    = argv[index+4].toFloat(&ok[1]);
        index += 5;
        rc = OkRc;
      }
  } else if (argv[index] == "ROUND" && argv.size() - index >= 4) {
    rc = FailureRc;
    bool ok[2];
    argv[index+2].toFloat(&ok[0]);
    argv[index+3].toFloat(&ok[1]);
    if (ok[0] && ok[1]) {
      _value[pushed].type  = BorderData::BdrRound;
      _value[pushed].line  = BorderData::BdrLnSolid;
      _value[pushed].color = argv[index+1];
      _value[pushed].thickness = argv[index+2].toFloat(&ok[0]);
      _value[pushed].radius    = argv[index+3].toFloat(&ok[1]);
      index += 4;
      rc = OkRc;
    }
  }
  if (rc == OkRc && argv.size() - index == 3) {
    if (argv[index] == "MARGINS") {
      bool ok[2];
      argv[index + 1].toFloat(&ok[0]);
      argv[index + 2].toFloat(&ok[1]);
      if (ok[0] && ok[1]) {
        _value[pushed].margin[0] = argv[index + 1].toFloat(&ok[0]);
        _value[pushed].margin[1] = argv[index + 2].toFloat(&ok[1]);
      } else {
        rc = FailureRc;
      }
    } else {
      rc = FailureRc;
    }
  }
  if (rc == OkRc) {
    _value[pushed].useDefault = false;
    _here[pushed] = here;
  }
  return rc;
}

QString BorderMeta::format(bool local, bool global)
{
  QString foo,border,colour;

  switch (_value[pushed].type) {
    case BorderData::BdrNone:
      foo = QString("NONE %1")
                    .arg(_value[pushed].line);
      break;
    case BorderData::BdrSquare:
      border = _value[pushed].hideTip ? "HIDDEN" : "SQUARE";
      colour = _value[pushed].color[0] == '#' ? _value[pushed].color : "\"" + _value[pushed].color + "\"";
      foo = QString("%1 %2 %3 %4")
                    .arg(border)
                    .arg(_value[pushed].line)
                    .arg(colour)
                    .arg(double(_value[pushed].thickness),0,'f',3);
      break;
    case BorderData::BdrRound:
      colour = _value[pushed].color[0] == '#' ? _value[pushed].color : "\"" + _value[pushed].color + "\"";
      foo = QString("ROUND %1 %2 %3 %4")
                    .arg(_value[pushed].line)
                    .arg(colour)
                    .arg(double(_value[pushed].thickness),0,'f',3)
                    .arg(double(_value[pushed].radius),0,'f',3);
      break;
  }
  QString bar = QString(" MARGINS %1 %2")
              .arg(double(_value[pushed].margin[0]),0,'f',3)
              .arg(double(_value[pushed].margin[1]),0,'f',3);
  foo += bar;
  return LeafMeta::format(local,global,foo);
}

void BorderMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( NONE <line integer 0> | HIDDEN <line integer 0> <\"color name|#RRGGBB\"> <thickness decimal> | "
                    "SQUARE <line integer 0-5> <\"color name|#RRGGBB\"> <thickness decimal> | "
                    "ROUND <line integer 0-5> <\"color name|#RRGGBB\"> <thickness decimal> <radius decimal> ) MARGINS <decimal X> <decimal Y>";
}

void BorderMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " NONE HIDDEN SQUARE ROUND MARGINS";
}

QString BorderMeta::text()
{
  BorderData border = valueInches();
  QString result, thickness;
  switch (border.type) {
    case BorderData::BdrNone:
      result = "No Border";
      break;
    case BorderData::BdrSquare:
      thickness = QString("%1") .arg(double(border.thickness),4,'f',3);
      result = "Square Corners, thickness " + thickness + " " + units2abbrev();
      break;
    default:
      thickness = QString("%1") .arg(double(border.thickness),4,'f',3);
      result = "Round Corners, thickness " + thickness + " " + units2abbrev();
      break;
    }
    return result;
}

/* ------------------ */

// Example: [LINE|BORDER] 1 Black 0.02
Rc PointerAttribMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = FailureRc;

  int arge = PointerAttribData::Invalid;

   _value[pushed] = parseAttributes(argv, here, index, arge, rc);

   if (arge == PointerAttribData::Invalid || rc != OkRc) {
     QString const message = QMessageBox::tr("Invalid command \"%1\"") .arg(argv.join(" "));
     emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
     return rc;
   }

   _here[pushed] = here;

  if (arge == PointerAttribData::Scoped)
    return rc;

  if (argv[index-2] == "CALLOUT") {
    if (argv[index-1] == "POINTER_ATTRIBUTE")
      rc = CalloutPointerAttribRc;
    else if (argv[index-1] == "DIVIDER_POINTER_ATTRIBUTE")
      rc = CalloutDividerPointerAttribRc;
  } else if (argv[index-2] == "MULTI_STEP" && argv[index-1] == "DIVIDER_POINTER_ATTRIBUTE") {
    rc = StepGroupDividerPointerAttribRc;
  } else if (argv[index-2] == "PAGE" && argv[index-1] == "POINTER_ATTRIBUTE") {
    rc = PagePointerAttribRc;
  }

  return rc;
}

PointerAttribData &PointerAttribMeta::parseAttributes(const QStringList &argv, const Where &here, const int indx, int &arge, Rc &rc)
{
  static QRegularExpression rx;
  QRegularExpressionMatch match;
  int index = indx;
  rx.setPattern("^(LINE|BORDER|TIP)$");
  if (!index)
    index = argv.indexOf(rx);
  match = rx.match(argv[index]);
  rx.setPattern("^(GLOBAL|LOCAL)$");
  bool scoped = argv[index-1].contains(rx);
  rx.setPattern("^(POINTER_ATTRIBUTE|DIVIDER_POINTER_ATTRIBUTE)$");
  bool valid  = argv[index-(scoped ? 2 : 1)].contains(rx);
  int type = PointerAttribData::Line;

  _result = _value[pushed];

  if (valid && match.hasMatch()) {
    if (match.captured(1) == "TIP")
      type = PointerAttribData::Tip;
    else if (match.captured(1) == "BORDER")
      type = PointerAttribData::Border;
    else // "LINE"
      type = PointerAttribData::Line;

    if (scoped)
      arge = PointerAttribData::Scoped;
    else
      arge = PointerAttribData::Valid;
  } else {
    arge = PointerAttribData::Invalid;
    return _result;
  }

  bool ok = false;
  bool widthKey = false;
  bool noParent = argv[index-(scoped ? 3 : 2)] == "CALLOUT" || argv[index-(scoped ? 2 : 1)] == "DIVIDER_POINTER_ATTRIBUTE";
  int widthIndex = -1, heightIndex = -1, sizeIndex = -1, colorIndex = -1;
  BorderData::Line lineType = BorderData::BdrLnNone;

  if (type == PointerAttribData::Tip) {
    bool ok[2];
    widthKey = argv[index+1] == "WIDTH";
    widthIndex = widthKey ? index+2 : index+1;
    argv[widthIndex].toFloat(&ok[0]);                          // width
    bool heightKey = argv[widthIndex+1] == "HEIGHT";
    heightIndex = heightKey ? widthIndex+2 : widthIndex+1;
    argv[heightIndex].toFloat(&ok[1]);                         // height
    if (ok[0] && ok[1]) {
      sizeIndex = heightIndex;
      rc = OkRc;
    } else {
      rc = FailureRc;
    }
  } else {
    if (_value[pushed].lineData.map.contains(argv[index+1])) { // line type word
      lineType = BorderData::Line(_result.lineData.map[argv[index+1]]);
    } else {
      int value = argv[index+1].toInt(&ok);                  // line type integer
      if (ok) {
        lineType = BorderData::Line(value);
        rc = OkRc;
      } else {
        rc = FailureRc;
      }
    }
    bool colorKey = argv[index+2] == "COLOR";
    colorIndex = colorKey ? index+3 : index+2;
    widthKey = argv[colorIndex+1] == "WIDTH";
    widthIndex = widthKey ? colorIndex+2 : colorIndex+1;
    argv[widthIndex].toFloat(&ok);                           // thickness
    if (ok) {
      sizeIndex = widthIndex;
      rc = OkRc;
    } else {
      rc = FailureRc;
    }
  }

  int id = 0;
  bool haveId = false;
  bool haveHideTip = false;
  int tip_idIndex = -1, idIndex = -1;

  if (argv.size() > sizeIndex+1) {
    rx.setPattern("^(HIDE_TIP|ID)$");
    bool tip_idKey = argv[sizeIndex+1].contains(rx);
    tip_idIndex = tip_idKey ? sizeIndex+2 : sizeIndex+1;
    if (tip_idKey) {                       // if line (hide tip or id), else if border or tip (id)
      if (argv[sizeIndex+1] == "ID") {     // check if line  id
        id = argv[tip_idIndex].toInt(&ok); // id integer
        haveId = ok;
        if (haveId)
          idIndex = tip_idIndex;
      } else {                             // set hide tip
        rx.setPattern("^(TRUE|FALSE)$");
        ok = haveHideTip = argv[tip_idIndex].contains(rx);
      }
    }
    rc = ok ? OkRc : FailureRc;
  }

  bool lineOrTip = type == PointerAttribData::Line || type == PointerAttribData::Tip;
  if (lineOrTip && !haveId && !scoped && tip_idIndex > -1 && argv.size() > tip_idIndex+1) {
    bool idKey = argv[tip_idIndex+1] == "ID";
    idIndex = idKey ? tip_idIndex+2 : tip_idIndex+1;
    id = argv[idIndex].toInt(&ok);  // if line id
    haveId = ok;
    rc = ok ? OkRc : FailureRc;
  }

  if (!id && !scoped)
  {
    QString const message = QMessageBox::tr("Expected ID greater than 0, but got \"%1\" in \"%2\"") .arg(id) .arg(argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    rc = FailureRc;
  }

  if (rc != OkRc)
    return _result;

  bool haveParent = !noParent && idIndex > -1 && !argv[tip_idIndex+1].isEmpty();

  if (type == PointerAttribData::Tip) {
    _result.attribType            = PointerAttribData::Tip;
    _result.tipData.tipWidth      = argv[widthIndex].toFloat();
    _result.tipData.tipHeight     = argv[heightIndex].toFloat();
    _result.tipHere.modelName     = here.modelName;
    _result.tipHere.lineNumber    = here.lineNumber;
    _result.tipData.useDefault    = false;
    _result.tipData.parametricTip = false;
  } else if (type == PointerAttribData::Line || type == PointerAttribData::Border) {
    float tipHeight               = DEFAULT_TIP_HEIGHT;
    float lineThickness           = DEFAULT_LINE_THICKNESS;
    float borderThickness         = DEFAULT_BORDER_THICKNESS;
    const float defaultThickness  = DEFAULT_POINTER_THICKNESS;
    const float tipRatio          = DEFAULT_TIP_RATIO;
    const double epsilon          = 0.0001;

    if (type == PointerAttribData::Line) {
      lineThickness               = argv[widthIndex].toFloat();
      _result.attribType          = PointerAttribData::Line;
      _result.lineData.line       = lineType;
      _result.lineData.color      = argv[colorIndex];
      _result.lineData.thickness  = lineThickness;
      if (haveHideTip)
        _result.lineData.hideTip  = argv[tip_idIndex] == "TRUE";
      else
        _result.lineData.hideTip  = argv[tip_idIndex].toInt(); // set show/hide arrow tip
      _result.lineHere.modelName  = here.modelName;
      _result.lineHere.lineNumber = here.lineNumber;
      _result.lineData.useDefault = false;
    } else if (type == PointerAttribData::Border) {
      borderThickness               = argv[widthIndex].toFloat();
      _result.attribType            = PointerAttribData::Border;
      _result.borderData.line       = lineType;
      _result.borderData.color      = argv[colorIndex];
      _result.borderData.thickness  = borderThickness;
      _result.borderHere.modelName  = here.modelName;
      _result.borderHere.lineNumber = here.lineNumber;
      _result.borderData.useDefault = false;
    }

    float delta = (lineThickness  + borderThickness) - defaultThickness;
    if (_result.tipData.parametricTip && (trunc(1000. * delta) != trunc(1000. * epsilon))) {
      tipHeight                += delta;
      _result.tipData.thickness = borderThickness;
      _result.tipData.tipWidth  = tipHeight * tipRatio;
      _result.tipData.tipHeight = tipHeight;
    }
  }

  if (_result.attribType == PointerAttribData::Tip) {
    _result.lineData    = _value[pushed].lineData;
    _result.lineHere    = _value[pushed].lineHere;
    _result.borderData  = _value[pushed].borderData;
    _result.borderHere  = _value[pushed].borderHere;
  } else {
    if (_result.attribType == PointerAttribData::Line) {
      _result.borderData  = _value[pushed].borderData;
      _result.borderHere  = _value[pushed].borderHere;
    }
    else if (_result.attribType == PointerAttribData::Border) {
      _result.lineData    = _value[pushed].lineData;
      _result.lineHere    = _value[pushed].lineHere;
    }

    if (_result.tipData.thickness == DEFAULT_BORDER_THICKNESS)
      _result.tipData.thickness = _value[pushed].tipData.thickness;
    if (_result.tipData.tipWidth == DEFAULT_TIP_WIDTH)
      _result.tipData.tipWidth = _value[pushed].tipData.tipWidth;
    if (_result.tipData.tipHeight == DEFAULT_TIP_HEIGHT)
     _result.tipData.tipHeight = _value[pushed].tipData.tipHeight;
  }

  _result.id     = haveId ? argv[idIndex].toInt() : 0;
  _result.parent = haveParent ? argv[idIndex+1] : QString();

  if (!indx) {
     _value[pushed] = _result;
     _here[pushed] = here;
  }

  return _result;
}

void PointerAttribMeta::setOtherDataInches(PointerAttribData pointerAttribData)
{
  if (_value[pushed].attribType == PointerAttribData::Tip) {
    _value[pushed].lineData    = pointerAttribData.lineData;
    _value[pushed].lineHere    = pointerAttribData.lineHere;
    _value[pushed].borderData  = pointerAttribData.borderData;
    _value[pushed].borderHere  = pointerAttribData.borderHere;
  } else {
    if (_value[pushed].attribType == PointerAttribData::Line) {
      _value[pushed].borderData  = pointerAttribData.borderData;
      _value[pushed].borderHere  = pointerAttribData.borderHere;
    }
    else if (_value[pushed].attribType == PointerAttribData::Border) {
      _value[pushed].lineData    = pointerAttribData.lineData;
      _value[pushed].lineHere    = pointerAttribData.lineHere;
    }
    if (_value[pushed].tipData.thickness == DEFAULT_BORDER_THICKNESS)
      _value[pushed].tipData.thickness  = pointerAttribData.tipData.thickness;
    if (_value[pushed].tipData.tipWidth  == DEFAULT_TIP_WIDTH)
      _value[pushed].tipData.tipWidth   = pointerAttribData.tipData.tipWidth;
    if (_value[pushed].tipData.tipHeight == DEFAULT_TIP_HEIGHT)
      _value[pushed].tipData.tipHeight  = pointerAttribData.tipData.tipHeight;
  }
}

QString PointerAttribMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value[pushed].attribType)
  {
  case PointerAttribData::Tip:
    foo = QString("TIP WIDTH %1 HEIGHT %2")
                  .arg(double(_value[pushed].tipData.tipWidth),0,'f',3)
                  .arg(double(_value[pushed].tipData.tipHeight),0,'f',3);
    break;
  case PointerAttribData::Line:
    foo = QString("LINE %1").arg(LineTypeNames[_value[pushed].lineData.line]);
    if (_value[pushed].lineData.color[0] == '#')
      foo += QString(" COLOR %1").arg(_value[pushed].lineData.color);
    else
      foo += QString(" COLOR \"%1\"").arg(_value[pushed].lineData.color);
    foo += QString(" WIDTH %1").arg(double(_value[pushed].lineData.thickness),0,'f',3);
    if (_value[pushed].lineData.hideTip)
      foo += QString(" HIDE_TIP TRUE");
    break;
  case PointerAttribData::Border:
    foo = QString("BORDER %1").arg(LineTypeNames[_value[pushed].borderData.line]);
    if (_value[pushed].borderData.color[0] == '#')
      foo += QString(" COLOR %1").arg(_value[pushed].borderData.color);
    else
      foo += QString(" COLOR \"%1\"").arg(_value[pushed].borderData.color);
    foo += QString(" WIDTH %1").arg(double(_value[pushed].borderData.thickness),0,'f',3);
    break;
  }
  if (_value[pushed].id || !_value[pushed].parent.isEmpty()) {
    foo += QString(" ID %1").arg(_value[pushed].id);
    if (!_value[pushed].parent.isEmpty())
      foo += QString(" %1").arg(_value[pushed].parent);
  }
  return LeafMeta::format(local,global,foo);
}

void PointerAttribMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( LINE | BORDER ) ( NONE | SOLID | DASH | DOT | DASH_DOT | DASH_DOT_DOT ) COLOR <\"#RRGGBB\"> WIDTH <decimal> "
                    "[ HIDE_TIP ( TRUE | FALSE )] [ ID <pointer number integer>] [( BASE_TOP | BASE_BOTTOM | BASE_LEFT | BASE_RIGHT )]) "
                    "( TIP WIDTH <decimal> HEIGHT <decimal> [ ID <pointer number integer>])";
}

void PointerAttribMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " LINE BORDER NONE SOLID DASH DOT DASH_DOT DASH_DOT_DOT COLOR WIDTH HIDE_TIP TRUE FALSE ID BASE_TOP BASE_BOTTOM BASE_LEFT BASE_RIGHT TIP HEIGHT";
}

/* ------------------ */

PointerMeta::PointerMeta() : LeafMeta()
{
  _value[0].placement = TopLeft;
  _value[0].loc       = 0;      // Location
  _value[0].x1        = 0.5;    // TipX
  _value[0].y1        = 0.5;    // TipY
  _value[0].x2        = 0.5;    // BaseX
  _value[0].y2        = 0.5;    // BaseY
  _value[0].x3        = 0.5;    // MidBaseX
  _value[0].y3        = 0.5;    // MidBaseY
  _value[0].x4        = 0.5;    // MidTipX
  _value[0].y4        = 0.5;    // MidTipY
  _value[0].base      = 0.125;  // BasePoint
  _value[0].segments  = 1;      // NumberOfSegments
  _value[0].rectPlacement = TopLeftOutsideCorner; // Page Pointer Base Rect Placement
  _value[1].placement = TopLeft;
  _value[1].loc       = 0;      // Location
  _value[1].x1        = 0.5;    // TipX
  _value[1].y1        = 0.5;    // TipY
  _value[1].x2        = 0.5;    // BaseX
  _value[1].y2        = 0.5;    // BaseY
  _value[1].x3        = 0.5;    // MidBaseX
  _value[1].y3        = 0.5;    // MidBaseY
  _value[1].x4        = 0.5;    // MidTipX
  _value[1].y4        = 0.5;    // MidTipY
  _value[1].base      = 0.125;  // BasePoint
  _value[1].segments  = 1;      // NumberOfSegments
  _value[1].rectPlacement = TopLeftOutsideCorner; // Page Pointer Base Rect Placement
}

/*
 * (TopLeft|TopRight|BottomRight|BottomLeft) <x> <y> (<base>)
 *
 * (Top|Right|Bottom|Left) <loc> <x> <y> (<base>)
 */

Rc PointerMeta::parse(QStringList &argv, int index, Where &here)
{
  int   _segments = 1;
  float _loc = 0.0, _base = -1.0;
  float _x1 = 0.0, _y1 = 0.0;
  float _x2 = 0.0, _y2 = 0.0;
  float _x3 = 0.0, _y3 = 0.0;
  float _x4 = 0.0, _y4 = 0.0;
  int   n_tokens = argv.size() - index;
  RectPlacement _bRect = RectPlacement(tokenMap["BASE_TOP_LEFT"]);
  bool  fail        = true;
  bool  pagePointer = false;

  //logTrace() << "Pointer: " << argv.join(" ") << ", Index:" << index;

  if (argv.size() - index > 0) {

    pagePointer = (argv[1] == "PAGE" && argv[2] == "POINTER");

    // pagePointer legacy
    if (argv[1] == "PAGE_POINTER") {
      pagePointer     = true;
      index += 1;
      n_tokens = argv.size() - index;

      QString message = QString("'%1' meta is deprecated. Use 'PAGE POINTER'").arg(argv[1]);
      QString parseMessage = QString("%1 (file: %2, line: %3)") .arg(message, here.modelName) .arg(here.lineNumber);
      if (Preferences::modeGUI)
        QMessageBox::warning(nullptr,
                             QMessageBox::tr(VER_PRODUCTNAME_STR),
                             parseMessage);
      else
        emit gui->messageSig(LOG_STATUS, parseMessage);
    }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    qDebug() << "\nPARSE LINE: " << argv.join(" ") <<
                "\n||| [index-1]: " << argv[index-1] << ", [index]: " << argv[index] <<
                ", argv[1]: " << argv[1] << ", argv[2]: " << argv[2]
                ;
#endif
//*/
    static QRegularExpression rx("^(TOP_LEFT|TOP_RIGHT|BOTTOM_LEFT|BOTTOM_RIGHT)$");

    // legacy single-segment pattern - base included
    if (argv[index].contains(rx) && n_tokens == 4) {
      _loc = 0;
      bool ok[3];
      _x1   = argv[index+1].toFloat(&ok[0]);
      _y1   = argv[index+2].toFloat(&ok[1]);
      _base = argv[index+3].toFloat(&ok[2]);
      fail  = ! (ok[0] && ok[1] && ok[2]);
    }
    // legacy single-segment pattern - no base
    if (argv[index].contains(rx) && n_tokens == 3) {
      _loc = 0;
      bool ok[2];
      _x1   = argv[index+1].toFloat(&ok[0]);
      _y1   = argv[index+2].toFloat(&ok[1]);
      fail  = ! (ok[0] && ok[1]);
    }
    // new multi-segment patterns (addl tokens: x2,y2,x3,y3,x4,y4,segments,[baseRect])
    if (argv[index].contains(rx) && (pagePointer ? n_tokens == 12 : n_tokens == 11)) {
      _loc = 0;
      bool ok[10];
      _x1       = argv[index+1].toFloat(&ok[0]);
      _y1       = argv[index+2].toFloat(&ok[1]);
      _x2       = argv[index+3].toFloat(&ok[2]);
      _y2       = argv[index+4].toFloat(&ok[3]);
      _x3       = argv[index+5].toFloat(&ok[4]);
      _y3       = argv[index+6].toFloat(&ok[5]);
      _x4       = argv[index+7].toFloat(&ok[6]);
      _y4       = argv[index+8].toFloat(&ok[7]);
      _base     = argv[index+9].toFloat(&ok[8]);
      _segments = argv[index+10].toInt(&ok[9]);
      if (pagePointer)
        _bRect  = RectPlacement(tokenMap[argv[index+11]]);
      fail      = ! (ok[0] && ok[1] && ok[2] && ok[3] && ok[4] &&
                     ok[5] && ok[6] && ok[7] && ok[8] && ok[9]);
    }
    if (argv[index].contains(rx) && (pagePointer ? n_tokens == 11 : n_tokens == 10)) {
      _loc = 0;
      bool ok[9];
      _x1       = argv[index+1].toFloat(&ok[0]);
      _y1       = argv[index+2].toFloat(&ok[1]);
      _x2       = argv[index+3].toFloat(&ok[2]);
      _y2       = argv[index+4].toFloat(&ok[3]);
      _x3       = argv[index+5].toFloat(&ok[4]);
      _y3       = argv[index+6].toFloat(&ok[5]);
      _x4       = argv[index+7].toFloat(&ok[6]);
      _y4       = argv[index+8].toFloat(&ok[7]);
      _segments = argv[index+9].toInt(&ok[8]);
      if (pagePointer)
        _bRect  = RectPlacement(tokenMap[argv[index+10]]);
      fail      = ! (ok[0] && ok[1] && ok[2] && ok[3] && ok[4] &&
                     ok[5] && ok[6] && ok[7] && ok[8]);
    }
    rx.setPattern("^(TOP|BOTTOM|LEFT|RIGHT|CENTER)$");
    if (argv[index].contains(rx) && n_tokens == 5) {
      bool ok[4];
      _loc  = argv[index+1].toFloat(&ok[0]);
      _x1    = argv[index+2].toFloat(&ok[1]);
      _y1    = argv[index+3].toFloat(&ok[2]);
      _base = argv[index+4].toFloat(&ok[3]);
      fail  = ! (ok[0] && ok[1] && ok[2] && ok[3]);
    }
    // legacy single-segment pattern - no base
    if (argv[index].contains(rx) && n_tokens == 4) {
      bool ok[3];
      _loc  = argv[index+1].toFloat(&ok[0]);
      _x1    = argv[index+2].toFloat(&ok[1]);
      _y1    = argv[index+3].toFloat(&ok[2]);
      fail  = ! (ok[0] && ok[1] && ok[2]);
    }
    // new multi-segment pattern (addl tokens: x2,y2,x3,y3,x4,y4,segments,[baseRect])
    if (argv[index].contains(rx) && (pagePointer ? n_tokens == 13 : n_tokens == 12)) {
      bool ok[11];
      _loc      = argv[index+1].toFloat(&ok[0]);
      _x1       = argv[index+2].toFloat(&ok[1]);
      _y1       = argv[index+3].toFloat(&ok[2]);
      _x2       = argv[index+4].toFloat(&ok[3]);
      _y2       = argv[index+5].toFloat(&ok[4]);
      _x3       = argv[index+6].toFloat(&ok[5]);
      _y3       = argv[index+7].toFloat(&ok[6]);
      _x4       = argv[index+8].toFloat(&ok[7]);
      _y4       = argv[index+9].toFloat(&ok[8]);
      _base     = argv[index+10].toFloat(&ok[9]);
      _segments = argv[index+11].toInt(&ok[10]);
      if (pagePointer)
        _bRect  = RectPlacement(tokenMap[argv[index+12]]);
      fail      = ! (ok[0] && ok[1] && ok[2] && ok[3] && ok[4] && ok[5] &&
                     ok[6] && ok[7] && ok[8] && ok[9] && ok[10]);
    }
    if (argv[index].contains(rx) && (pagePointer ? n_tokens == 12 : n_tokens == 11)) {
      bool ok[10];
      _loc      = argv[index+1].toFloat(&ok[0]);
      _x1       = argv[index+2].toFloat(&ok[1]);
      _y1       = argv[index+3].toFloat(&ok[2]);
      _x2       = argv[index+4].toFloat(&ok[3]);
      _y2       = argv[index+5].toFloat(&ok[4]);
      _x3       = argv[index+6].toFloat(&ok[5]);
      _y3       = argv[index+7].toFloat(&ok[6]);
      _x4       = argv[index+8].toFloat(&ok[7]);
      _y4       = argv[index+9].toFloat(&ok[8]);
      _segments = argv[index+10].toInt(&ok[9]);
      if (pagePointer)
        _bRect  = RectPlacement(tokenMap[argv[index+11]]);
      fail      = ! (ok[0] && ok[1] && ok[2] && ok[3] && ok[4] &&
                     ok[5] && ok[6] && ok[7] && ok[8] && ok[9]);
    }
  }
  if ( ! fail) {
    _value[pushed].placement = PlacementEnc(tokenMap[argv[index]]);
    _value[pushed].loc        = _loc;
    _value[pushed].x1         = _x1;  //Tip.x
    _value[pushed].y1         = _y1;  //Tip.y
    _value[pushed].x2         = _x2;  //Base.x
    _value[pushed].y2         = _y2;  //Base.y
    _value[pushed].x3         = _x3;  //MidBase.x
    _value[pushed].y3         = _y3;  //MidBase.y
    _value[pushed].x4         = _x4;  //MidTip.x
    _value[pushed].y4         = _y4;  //MidTip.y
    if (_base > 0) {
      _value[pushed].base = _base;
    } else if (_value[pushed].base == 0.0f) {
      _value[pushed].base = 1.0/8;
    }
    _value[pushed].segments   = _segments;
    if (pagePointer)
      _value[pushed].rectPlacement = _bRect; //Base Rect Placement
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    if (/ *argv[1] == "PAGE"* / true) {
      qDebug()  << "\nPOINTER DATA " << argv[1] << " (Parsed)"
                << " \nPlacement:             "   << PlacNames[_value[pushed].placement] << " ("
                                                  << _value[pushed].placement << ") of Base"
                << " \nLoc(fraction of side): "   << _value[pushed].loc
                << " \nx1 (Tip.x):            "   << _value[pushed].x1
                << " \ny1 (Tip.y):            "   << _value[pushed].y1
                << " \nx2 (Base.x):           "   << _value[pushed].x2
                << " \ny2 (Base.y):           "   << _value[pushed].y2
                << " \nx3 (MidBase.x):        "   << _value[pushed].x3
                << " \ny3 (MidBase.y):        "   << _value[pushed].y3
                << " \nx4 (MidTip.x):         "   << _value[pushed].x4
                << " \ny4 (MidTip.y):         "   << _value[pushed].y4
                << " \nBase:                  "   << _value[pushed].base
                << " \nSegments:              "   << _value[pushed].segments
                << " \nPagePointer Rect:      "   << (pagePointer ? QString("%1 (%2) of Page")
                                                                            .arg(RectNames[_value[pushed].rectPlacement])
                                                                            .arg(_value[pushed].rectPlacement) :
                                                                            "None - Not PagePointer")
                   ;
    }
#endif
//*/
    _here[pushed] = here;

    if ((argv[1] == "PAGE" && argv[2] == "POINTER") ||
      (argv[1] == "PAGE_POINTER")) {
//      logTrace() << "Return PagePointerRc";
      return PagePointerRc;
    }
    else
    if (argv[1] == "CALLOUT" && argv[2] == "POINTER") {
//      logTrace() << "Return CalloutPointerRc";
      return CalloutPointerRc;
    }
    else
    if (argv[1] == "CALLOUT" && argv[2] == "DIVIDER_POINTER") {
//    logTrace() << "Return CalloutDividerPointerRc";
      return CalloutDividerPointerRc;
    }
    else
    if (argv[1] == "MULTI_STEP" && argv[2] == "DIVIDER_POINTER") {
//    logTrace() << "Return StepGroupDividerPointerRc";
      return StepGroupDividerPointerRc;
    }
    else
    if (argv[1] == "ILLUSTRATION" && argv[2] == "POINTER") {
//      logTrace() << "Return IllustrationPointerRc";
      return IllustrationPointerRc;
    }

    emit gui->messageSig(LOG_ERROR,"Pointer type not defined. Returning 0.");

    return OkRc; // this should never fire.

  } else {

    if (reportErrors) {
      QString const message = QMessageBox::tr("Malformed pointer \"%1\"") .arg(argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
    return FailureRc;
  }
}

QString PointerMeta::format(bool local, bool global)
{
  static QRegularExpression rx("^\\s*0.*\\s+(PAGE POINTER|PAGE_POINTER)\\s+.*$");
  bool pagePointer = preamble.contains(rx);
  QString foo;
  switch(_value[pushed].placement) {
    case TopLeft:
    case TopRight:
    case BottomRight:
    case BottomLeft:
      foo = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11")
          .arg(placementNames[_value[pushed].placement])
          .arg(double(_value[pushed].x1),0,'f',3)
          .arg(double(_value[pushed].y1),0,'f',3)
          .arg(double(_value[pushed].x2),0,'f',3)
          .arg(double(_value[pushed].y2),0,'f',3)
          .arg(double(_value[pushed].x3),0,'f',3)
          .arg(double(_value[pushed].y3),0,'f',3)
          .arg(double(_value[pushed].x4),0,'f',3)
          .arg(double(_value[pushed].y4),0,'f',3)
          .arg(double(_value[pushed].base),0,'f',3)
          .arg(QString("%1%2")
                       .arg(                                     _value[pushed].segments)
                       .arg(pagePointer ?
                                QString(" %1")
                                        .arg(bRectPlacementNames[_value[pushed].rectPlacement]) : ""));
      break;
    default:
      foo = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12")
          .arg(placementNames[_value[pushed].placement])
          .arg(double(_value[pushed].loc),0,'f',3)
          .arg(double(_value[pushed].x1),0,'f',3)
          .arg(double(_value[pushed].y1),0,'f',3)
          .arg(double(_value[pushed].x2),0,'f',3)
          .arg(double(_value[pushed].y2),0,'f',3)
          .arg(double(_value[pushed].x3),0,'f',3)
          .arg(double(_value[pushed].y3),0,'f',3)
          .arg(double(_value[pushed].x4),0,'f',3)
          .arg(double(_value[pushed].y4),0,'f',3)
          .arg(double(_value[pushed].base),0,'f',3)
          .arg(QString("%1%2")
                       .arg(                                     _value[pushed].segments)
                       .arg(pagePointer ?
                                QString(" %1")
                                        .arg(bRectPlacementNames[_value[pushed].rectPlacement]) : ""));
      break;
    }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  if (/ *pagePointer* / true) {
    qDebug() << "\nPOINTER META FORMAT"
                "\nPreamble:         " <<  preamble <<
                "\nMatch Test:       " << (preamble.contains(rx) ? "Success :)" : "Failed :(") <<
                "\nSegments:         " << _value[pushed].segments <<
                "\nPagePointer Rect: " << (pagePointer ? RectNames[_value[pushed].rectPlacement] + " (" +
                                           QString::number(_value[pushed].rectPlacement) + ") of Page" : "N/A") <<
                "\nNew Meta Line:" << preamble + foo
                  ;
  }
#endif
//*/
  return LeafMeta::format(local,global,foo);
}

void PointerMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TOP | BOTTOM | LEFT | RIGHT | CENTER | TOP_LEFT | TOP_RIGHT | BOTTOM_LEFT | BOTTOM_RIGHT ) <decimal Location> <decimal X1> <decimal Y1>"
                    " [ <decimal X2> <decimal Y2> <decimal X3> <decimal Y3> <decimal X4> <decimal  Y4>] <decimal Base> [<integer Segments>]"
                    " [( BASE_TOP | BASE_BOTTOM | BASE_LEFT | BASE_RIGHT )]";
}

void PointerMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " TOP BOTTOM LEFT RIGHT CENTER TOP_LEFT TOP_RIGHT BOTTOM_LEFT BOTTOM_RIGHT BASE_TOP BASE_BOTTOM BASE_LEFT BASE_RIGHT";
}

//--------------
Rc CsiAnnotationIconMeta::parse(QStringList &argv, int index,Where &here)
{
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  QStringList debugLine = QStringList() << "[LINE:";
  for(int i=0;i<argv.size();i++) {
      debugLine << argv[i];
      int size = argv.size();
      int incr = i;
      int result = size - incr;
      QString traceLine = QString("ARGV Pos:(%1), PosIndex:(%2) [%3 - %4 = %5], Value:(%6)")
                                  .arg(i+1)
                                  .arg(i)
                                  .arg(size)
                                  .arg(incr)
                                  .arg(result)
                                  .arg(argv[i]);
      qDebug() << "\nCSI ANNOTATION ICON META PARSE" << traceLine;  // ARGS DETAIL
  }
  debugLine << QString("], Index (%1)[%2], Size (%3), Valid (%4), LineNum (%5), ModelName (%6)")
                       .arg(index)
                       .arg(argv[index])
                       .arg(argv.size())
                       .arg(argv.size() - index)
                       .arg(here.modelName)
                       .arg(here.lineNumber);
#endif
//*/
  CsiAnnotationIconData annotationData;
  Rc rc = FailureRc;
  static QRegularExpression rx;
  if (argv.size() - index == 1) {
      rx.setPattern("^(HIDE|HIDDEN)$");
      if (argv[index].contains(rx)) {
          annotationData.hidden = true;
          rc = OkRc;
      }
  }
  else
  if (argv.size() - index >= 10) {
    rx.setPattern("^(TOP_LEFT|TOP|TOP_RIGHT|LEFT|CENTER|RIGHT|BOTTOM_LEFT|BOTTOM|BOTTOM_RIGHT)$");
    QStringList entries;
    if (argv[index].contains(rx)) {
      entries << QString::number(PlacementEnc(tokenMap[argv[index]]));
      rc = OkRc;
    }
    if (argv.size() - index == 11) {
      if (argv[++index].contains(rx)) {
        entries << QString::number(PlacementEnc(tokenMap[argv[index]]));
        rc = OkRc;
      }
    }
    rx.setPattern("^(INSIDE|OUTSIDE)$");
    if (argv[++index].contains(rx)) {
      entries << QString::number(PrepositionEnc(tokenMap[argv[index]]));
      rc = OkRc;
    }
    annotationData.placements = entries;

    if (rc == OkRc) {
      bool good = false, ok = false;
      annotationData.iconOffset[0] = argv[++index].toInt(&good);
      annotationData.iconOffset[1] = argv[++index].toInt(&ok);
      good &= ok;
      annotationData.partOffset[0] = argv[++index].toFloat(&ok);
      good &= ok;
      annotationData.partOffset[1] = argv[++index].toFloat(&ok);
      good &= ok;
      annotationData.partSize[0] = argv[++index].toInt(&ok);
      good &= ok;
      annotationData.partSize[1] = argv[++index].toInt(&ok);
      good &= ok;
      annotationData.typeColor = argv[++index].toInt(&ok);
      good &= ok;
      if (!good) {
        rc = FailureRc;
      } else {
        annotationData.typeBaseName = argv[++index];
      }
    }
  }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
  QString result = QString(", Result (%1)").arg(rc == 0 ? "OkRc" : "FailureRc");
  qDebug() << "\nCSI ANNOTATION ICON META PARSE DEBUG" << debugLine.join(" ") << result;
#endif
//*/
  if (rc == OkRc) {
    _value[pushed] = annotationData;
    _here[pushed]  = here;
    return AssemAnnotationIconRc;
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Malformed CSI Annotation metacommand \"%1\"\n").arg(argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return rc;
}

// format() should never be used as we manage CsiAnnotationIconData directly in pliPart
QString CsiAnnotationIconMeta::format(bool local, bool global)
{

  QString foo,bar;

  if (_value[pushed].hidden) {
      foo = "HIDDEN";
  } else {
    if (_value[pushed].placements.size() == 2) {
      foo = QString("%1 %2 ")
              .arg(placementNames[PlacementEnc(_value[pushed].placements.at(0).toInt())],
                   prepositionNames[PrepositionEnc(_value[pushed].placements.at(1).toInt())]);
    }
    else
    if (_value[pushed].placements.size() == 3) {
      foo = QString("%1 %2 %3 ")
              .arg(placementNames[PlacementEnc(_value[pushed].placements.at(0).toInt())],
                   placementNames[PlacementEnc(_value[pushed].placements.at(1).toInt())],
                   prepositionNames[PrepositionEnc(_value[pushed].placements.at(2).toInt())]);
    }
    bar = QString("%1 %2 %3 %4 %5 %6 %7 %8")
                   .arg(double(_value[pushed].iconOffset[0]),0,'f',0)
                   .arg(double(_value[pushed].iconOffset[1]),0,'f',0)
                   .arg(double(_value[pushed].partOffset[0]),0,'f',4)
                   .arg(double(_value[pushed].partOffset[1]),0,'f',4)
                   .arg(_value[pushed].partSize[0])
                   .arg(_value[pushed].partSize[1])
                   .arg(_value[pushed].typeColor)
                   .arg(_value[pushed].typeBaseName);
    foo += bar;
  }
/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    qDebug() << "\nCSI ANNOTATION ICON META FORMAT" <<
                "\nPreamble:" << preamble << "LINE DATA" << foo;
#endif
//*/
  return LeafMeta::format(local,global,foo);
}

void CsiAnnotationIconMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TOP_LEFT | TOP | TOP_RIGHT | LEFT | CENTER | RIGHT | BOTTOM_LEFT | BOTTOM | BOTTOM_RIGHT )"
                    " <icon offset X px> <icon offset Y px> <part offset X px> <part offset Y px>"
                    " <part size X px> <part size Y px> <LDraw color code for part> <LDraw part name without extension>";
}

void CsiAnnotationIconMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " TOP_LEFT TOP TOP_RIGHT LEFT CENTER RIGHT BOTTOM_LEFT BOTTOM BOTTOM_RIGHT";
}
/* ------------------ */

PreferredRendererMeta::PreferredRendererMeta() : LeafMeta()
{
  _value[0].renderer           = Preferences::preferredRenderer;
  _value[0].useLDVSingleCall   = Preferences::enableLDViewSingleCall;
  _value[0].useLDVSnapShotList = Preferences::enableLDViewSnaphsotList;
  _value[0].useNativeGenerator = Preferences::useNativePovGenerator;
  _value[0].usePerspectiveProjection = Preferences::perspectiveProjection;

  _value[1].renderer           = Preferences::preferredRenderer;
  _value[1].useLDVSingleCall   = Preferences::enableLDViewSingleCall;
  _value[1].useLDVSnapShotList = Preferences::enableLDViewSnaphsotList;
  _value[1].useNativeGenerator = Preferences::useNativePovGenerator;
  _value[1].usePerspectiveProjection = Preferences::perspectiveProjection;
}

void PreferredRendererMeta::setPreferences(bool reset)
{
  RendererData data      = _value[pushed];
  bool displayPreference = false;
  if (reset) {
    Preferences::preferredRendererPreferences();
    displayPreference = Preferences::preferredRenderer != data.renderer;
    data.renderer           = Preferences::preferredRenderer;
    data.useLDVSingleCall   = Preferences::enableLDViewSingleCall;
    data.useLDVSnapShotList = Preferences::enableLDViewSnaphsotList;
    data.useNativeGenerator = Preferences::useNativePovGenerator;
    data.usePerspectiveProjection = Preferences::perspectiveProjection;
    _value[pushed] = data;
  } else {
    if ((displayPreference = Preferences::preferredRenderer != data.renderer || global)) {
      Preferences::preferredRenderer      = data.renderer;
      Render::setRenderer(Preferences::preferredRenderer);
    }
    Preferences::enableLDViewSingleCall   = data.useLDVSingleCall ;
    Preferences::enableLDViewSnaphsotList = data.useLDVSnapShotList;
    Preferences::useNativePovGenerator    = data.useNativeGenerator;
    Preferences::perspectiveProjection    = data.usePerspectiveProjection;
    if (global) {
      Preferences::preferredRendererPreferences(global);
      if (Preferences::preferredRenderer != data.renderer)
        emit gui->clearAndReloadModelFileSig(false/*fileReload*/, true/*savePrompt*/, false/*keepWork*/);
    }
  }
  Preferences::updatePOVRayConfigFiles();
  if (displayPreference)
    emit gui->messageSig(LOG_INFO,QMessageBox::tr("Renderer %1 %2%3.")
                                                  .arg(reset ? "reset to" : global ? "save as" : "changed to",
                                                      rendererNames[Preferences::preferredRenderer],
                                                      Preferences::preferredRenderer == RENDERER_POVRAY ? QString(" (POV file generator is %1)")
                                                                                                                   .arg(Preferences::useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW]) :
                                                       Preferences::preferredRenderer == RENDERER_LDVIEW ? Preferences::enableLDViewSingleCall ?
                                                                                                           Preferences::enableLDViewSnaphsotList ? QString(" (Single Call using Export File List)") :
                                                                                                                                                   QString(" (Single Call)") :
                                                                                                                                                   QString() : QString()));

}

Rc PreferredRendererMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = OkRc;

  if (argv.size() - index >= 1) {
    if (argv[index] == "NATIVE")
      _value[pushed].renderer = RENDERER_NATIVE;
    else if (argv[index] == "LDVIEW")
      _value[pushed].renderer = RENDERER_LDVIEW;
    else if (argv[index] == "LDGLITE")
      _value[pushed].renderer = RENDERER_LDGLITE;
    else if (argv[index] == "POVRAY")
      _value[pushed].renderer = RENDERER_POVRAY;
    else
      rc = FailureRc;
    if (argv.size() - index > 1) {
      index++;
      if (_value[pushed].renderer == RENDERER_LDVIEW) {
        if (argv[index] == "SINGLE_CALL")
          _value[pushed].useLDVSingleCall = true;
        else if (argv[index] == "SINGLE_CALL_EXPORT_LIST")
          _value[pushed].useLDVSnapShotList = _value[pushed].useLDVSingleCall = true;
      } else if (_value[pushed].renderer == RENDERER_POVRAY) {
        if (argv[index] == "LDVIEW_POV_GENERATOR")
          _value[pushed].useNativeGenerator = false;
      }
    }
  }
  if (rc == OkRc) {
    _here[pushed] = here;
    if (argv[1] == "CALLOUT") {
      rc = PreferredRendererCalloutAssemRc;
    } else if (argv[1] == "MULTI_STEP") {
      rc = PreferredRendererGroupAssemRc;
    } else if (argv[1] == "ASSEM")
      rc = PreferredRendererAssemRc;
    else if (argv[1] == "SUBMODEL_DISPLAY")
      rc = PreferredRendererSubModelRc;
    else if (argv[1] == "PLI")
      rc = PreferredRendererPliRc;
    else if (argv[1] == "BOM")
      rc = PreferredRendererBomRc;
    else if (argv[1] == "PREFERRED_RENDERER")
      rc = PreferredRendererRc;
  } else if (reportErrors) {
    QString const message = QMessageBox::tr("Expected (NATIVE|LDGLITE|LDVIEW [SINGLE_CALL|SINGLE_CALL_EXPORT_LIST]|POVRAY [LDVIEW_POV_GENERATOR]) (RESET), but got \"%1\" %2")
                                            .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return rc;
}

QString PreferredRendererMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString(rendererNames[_value[pushed].renderer]).toUpper();
  if (_value[pushed].renderer == RENDERER_LDVIEW) {
    if (_value[pushed].useLDVSingleCall)
      foo.append(" SINGLE_CALL");
    else if (_value[pushed].useLDVSnapShotList)
      foo.append(" SINGLE_CALL_EXPORT_LIST");
  } else if (_value[pushed].renderer == RENDERER_POVRAY) {
    if (! _value[pushed].useNativeGenerator)
      foo.append(" LDVIEW_POV_GENERATOR");
  }
  return LeafMeta::format(local,global,foo);
}

void PreferredRendererMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( NATIVE | LDGLITE | LDVIEW [ SINGLE_CALL | SINGLE_CALL_EXPORT_LIST ] | POVRAY [ LDVIEW_POV_GENERATOR ])";
}

void PreferredRendererMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " NATIVE LDGLITE LDVIEW SINGLE_CALL SINGLE_CALL_EXPORT_LIST POVRAY LDVIEW_POV_GENERATOR";
}
/* ------------------ */

FreeFormMeta::FreeFormMeta() : LeafMeta()
{
  _value[0].mode = false;
  _value[1].mode = false;
}
Rc FreeFormMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  if (argv.size() - index == 1 && argv[index] == "FALSE") {
    _value[pushed].mode = false;
    rc = OkRc;
  } else if (argv.size() - index == 2) {
    _value[pushed].mode = true;
    static QRegularExpression rx("^(STEP_NUMBER|ASSEM|PLI|ROTATE_ICON)$");
    if (argv[index].contains(rx)) {
      rx.setPattern("^(LEFT|RIGHT|TOP|BOTTOM|CENTER)$");
      if (argv[index+1].contains(rx)) {
        _value[pushed].base = PlacementEnc(tokenMap[argv[index]]);
        _value[pushed].justification = PlacementEnc(tokenMap[argv[index+1]]);
        rc = OkRc;
      }
    }
  }
  if (rc == OkRc) {
    _here[pushed] = here;
  }
  return rc;
}

QString FreeFormMeta::format(bool local, bool global)
{
  QString foo;
  if (_value[pushed].mode) {
    foo = relativeNames[_value[pushed].base] + " " +
         placementNames[_value[pushed].justification];
  } else {
    foo = "FALSE";
  }
  return LeafMeta::format(local,global,foo);
}

void FreeFormMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( FALSE | ( STEP_NUMBER | ASSEM | PLI | ROTATE_ICON ) ( LEFT | RIGHT | TOP | BOTTOM | CENTER ))";
}

void FreeFormMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " FALSE STEP_NUMBER ASSEM PLI ROTATE_ICON LEFT RIGHT TOP BOTTOM CENTER";
}
/* ------------------ */

ConstrainMeta::ConstrainMeta() : LeafMeta()
{
  _default = true;
}

Rc ConstrainMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  bool ok;
  static QRegularExpression rx;
  switch(argv.size() - index) {
    case 1:
      rx.setPattern("^(AREA|SQUARE)$");
      if (argv[index].contains(rx)) {
        _value[pushed].type = ConstrainData::PliConstrain(tokenMap[argv[index]]);
        rc = OkRc;
      }
      break;
    case 2:
      argv[index+1].toFloat(&ok);
      if (ok) {
        rx.setPattern("^(WIDTH|HEIGHT|COLS)$");
        if (argv[index].contains(rx)) {
          _value[pushed].type = ConstrainData::PliConstrain(tokenMap[argv[index]]);
          switch (_value[pushed].type)
          {
          case ConstrainData::PliConstrainWidth:
            _value[pushed].constraint.width = argv[index+1].toFloat(&ok);
            break;
          case ConstrainData::PliConstrainHeight:
            _value[pushed].constraint.height = argv[index+1].toFloat(&ok);
            break;
          case ConstrainData::PliConstrainColumns:
            _value[pushed].constraint.columns = argv[index+1].toFloat(&ok);
            break;
          default:
            break;
          }
          rc = OkRc;
        }
      }
      break;
    }
  if (rc == OkRc) {
    _here[pushed] = here;
    _default = false;
  }
  return rc;
}

QString ConstrainMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value[pushed].type) {
  case ConstrainData::PliConstrainArea:
    foo = "AREA";
    break;
  case ConstrainData::PliConstrainSquare:
    foo = "SQUARE";
    break;
  case ConstrainData::PliConstrainWidth:
    foo = QString("WIDTH %1") .arg(double(_value[pushed].constraint.width),0,'f',4);
    break;
  case ConstrainData::PliConstrainHeight:
    foo = QString("HEIGHT %1") .arg(double(_value[pushed].constraint.height),0,'f',4);
    break;
  default:
    foo = QString("COLS %1") .arg(_value[pushed].constraint.columns);
    break;
  }
  return LeafMeta::format(local,global,foo);
}
void ConstrainMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( AREA | SQUARE | ( WIDTH | HEIGHT | COLS ) <integer> )";
}
void ConstrainMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " AREA SQUARE WIDTH HEIGHT COLS";
}

/* ------------------ */

AllocMeta::AllocMeta() : LeafMeta()
{
  type[0] = Vertical;
  type[1] = Vertical;
}

Rc AllocMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(HORIZONTAL|VERTICAL)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
      type[pushed] = AllocEnc(tokenMap[argv[index]]);
      _here[pushed] = here;
      return OkRc;
    }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected HORIZONTAL or VERTICAL but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}
QString AllocMeta::format(bool local, bool global)
{
  QString foo;
  if (type[pushed] == Horizontal) {
    foo = "HORIZONTAL";
  } else {
    foo = "VERTICAL";
  }
  return LeafMeta::format(local,global,foo);
}
void AllocMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( HORIZONTAL | VERTICAL )";
}
void AllocMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " HORIZONTAL VERTICAL";
}

/*------------------------*/

void CameraAnglesMeta::init(BranchMeta *parent, const QString name, Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}

Rc CameraAnglesMeta::parse(QStringList &argv, int index, Where &here)
{
  using namespace Options;
  using namespace CameraViews;
  QString message = QObject::tr("The specified latitude %1 or longitude %2 value is not within the mininum %3 or maximum %4 allowed \"%5\".");
  static QRegularExpression rx("^(FRONT|BACK|TOP|BOTTOM|LEFT|RIGHT|HOME|LAT_LON|DEFAULT)$");
  if (argv.size() - index == 1) {
    if (argv[index].contains(rx)) {
      float latitude  = 30.0f;
      float longitude = 45.0f;
      CameraView cameraView  = static_cast<CameraView>(_value[pushed].map[argv[index]]);
      switch (cameraView)
      {
        case CameraView::Front:
          latitude = 0.0f;
          longitude = 0.0f;
          break;
        case CameraView::Back:
          latitude = 0.0f;
          longitude = 180.0f;
          break;
        case CameraView::Top:
          latitude = 90.0f;
          longitude = 0.0f;
          break;
        case CameraView::Bottom:
          latitude = -90.0f;
          longitude =  0.0f;
          break;
        case CameraView::Left:
          latitude = 0.0f;
          longitude = 90.0f;
          break;
        case CameraView::Right:
          latitude = 0.0f;
          longitude = -90.0f;
          break;
        case CameraView::Home:
          latitude = 30.0f;
          longitude = 45.0f;
          break;
        case CameraView::LatLon:
          switch (_value[pushed].type)
          {
            case CSI:
              latitude = MetaDefaults::getAssemblyCameraLatitude();
              longitude = MetaDefaults::getAssemblyCameraLongitude();
              break;
            case PLI:
              latitude = MetaDefaults::getPartCameraLatitude();
              longitude = MetaDefaults::getPartCameraLongitude();
              break;
            case SMI:
              latitude = MetaDefaults::getSubmodelCameraLatitude();
              longitude = MetaDefaults::getSubmodelCameraLongitude();
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
      _value[pushed].cameraView = cameraView;
      _value[pushed].angles[0]  = latitude;
      _value[pushed].angles[1]  = longitude;
      _here[pushed] = here;
      return OkRc;
    }
    message = QObject::tr("Expected FRONT|BACK|TOP|BOTTOM|LEFT|RIGHT|HOME|LAT_LON but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
  } else if (argv.size() - index == 2) {
    bool ok[2];
    float latitude = argv[index  ].toFloat(&ok[0]);
    float longitude = argv[index+1].toFloat(&ok[1]);
    if (ok[0] && ok[1]) {
      if (latitude < _min || latitude > _max ||
          longitude < _min || longitude > _max) {
        emit gui->messageSig(LOG_ERROR,message .arg(latitude) .arg(longitude) .arg(_min) .arg(_max) .arg(argv.join(" ")));
        return RangeErrorRc;
      }
      _value[pushed].cameraView = CameraView::Default;
      _value[pushed].angles[0]  = latitude;
      _value[pushed].angles[1]  = longitude;
      _here[pushed] = here;
      return rc;
    }
    message = QObject::tr("Expected <decimal> <decimal> (e.g. 23.0 45.0), but got \"%1\" %2") .arg(argv[index], argv.join(" "));
  } else if (argv.size() - index == 3) {
    rx.setPattern("^(HOME|LAT_LON)$");
    if (argv[index].contains(rx)) {
      bool ok[2];
      float latitude = argv[index+1].toFloat(&ok[0]);
      float longitude = argv[index+2].toFloat(&ok[1]);
      if (ok[0] && ok[1]) {
        if (latitude < _min || latitude > _max ||
            longitude < _min || longitude > _max) {
          emit gui->messageSig(LOG_ERROR,message .arg(latitude) .arg(longitude) .arg(_min) .arg(_max) .arg(argv.join(" ")));
          return RangeErrorRc;
        }
        CameraView cameraView  = static_cast<CameraView>(_value[pushed].map[argv[index]]);
        _value[pushed].customViewpoint = true;
        _value[pushed].cameraView = cameraView;
        _value[pushed].angles[0]  = latitude;
        _value[pushed].angles[1]  = longitude;
        _here[pushed] = here;
        return rc;
      }
      message = QObject::tr("Expected LAT_LON <decimal> <decimal> or HOME  <decimal> <decimal> but got \"%1\" %2") .arg(argv[index], argv.join(" "));
    }
    message = QObject::tr("Expected LAT_LON or HOME but got \"%1\" %2") .arg(argv[index], argv.join(" "));
  }
  if (reportErrors) {
      emit gui->messageSig(LOG_ERROR,message);
  }
  return FailureRc;
}

QString CameraAnglesMeta::format(bool local, bool global)
{
  using namespace CameraViews;
  QString foo;
  if (_value[pushed].cameraView == CameraView::Default) {
    foo = QString("%1 %2")
                  .arg(double(_value[pushed].angles[0]),_fieldWidth,'f',_precision)
                  .arg(double(_value[pushed].angles[1]),_fieldWidth,'f',_precision);
  } else {
    foo = cameraViewNames[_value[pushed].cameraView];
    if (_value[pushed].customViewpoint) {
        foo += QString(" %1 %2")
                       .arg(double(_value[pushed].angles[0]),_fieldWidth,'f',_precision)
                       .arg(double(_value[pushed].angles[1]),_fieldWidth,'f',_precision);
    }
  }
  return LeafMeta::format(local,global,foo.trimmed());
}

void CameraAnglesMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( <decimal> <decimal> ) ( FRONT | BACK | TOP | BOTTOM | LEFT | RIGHT | HOME [ <decimal> <decimal> ] | LAT_LON <decimal> <decimal> | <decimal> <decimal> )";
}

void CameraAnglesMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " <decimal> FRONT BACK TOP BOTTOM LEFT RIGHT HOME LAT_LON";
}
/* ------------------ */

FillMeta::FillMeta() : LeafMeta()
{
  type[0] = Aspect;
  type[1] = Aspect;
}

Rc FillMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(ASPECT|STRETCH|TILE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
      type[pushed] = FillEnc(tokenMap[argv[index]]);
      _here[pushed] = here;
      return OkRc;
    }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected ASPECT, STRETCH, or TILE but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}
QString FillMeta::format(bool local, bool global)
{
  QString foo;
  if (type[pushed] == Stretch) {
    foo = "STRETCH";
  } else
  if (type[pushed] == Tile) {
    foo = "TILE";
  } else {
    foo = "ASPECT";
  }
  return LeafMeta::format(local,global,foo);
}
void FillMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( ASPECT | STRETCH | TILE )";
}
void FillMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " ASPECT STRETCH TILE";
}

/* ------------------ */

JustifyStepMeta::JustifyStepMeta() : LeafMeta()
{
  _value[0].type    = JustifyLeft;
  _value[0].spacing = STEP_SPACING_DEFAULT;
  _value[1].type    = JustifyLeft;
  _value[1].spacing = STEP_SPACING_DEFAULT;
}
Rc JustifyStepMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(JUSTIFY_LEFT|JUSTIFY_CENTER|JUSTIFY_CENTER_HORIZONTAL|JUSTIFY_CENTER_VERTICAL)$");
  if (argv[index].contains(rx)) {
    if (argv.size() - index >= 1)
      _value[pushed].type = JustifyStepEnc(tokenMap[argv[index]]);
    if (argv.size() - index == 3 && argv[index+1].contains("SPACING")) {
      bool ok;
      float spacing = argv[index+2].toFloat(&ok);
      if (!ok)
          return FailureRc;
      _value[pushed].spacing = spacing;
    }
    _here[pushed] = here;
    return OkRc;
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected JUSTIFY_LEFT,JUSTIFY_CENTER,JUSTIFY_CENTER_HORIZONTAL or JUSTIFY_CENTER_VERTICAL but got \"%1\" in \"%2\"")
                                            .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString JustifyStepMeta::format(bool local, bool global)
{
  QString foo;
  if (_value[pushed].type == JustifyLeft) {
    foo = "JUSTIFY_LEFT";
  } else if (_value[pushed].type == JustifyCenter) {
    foo = "JUSTIFY_CENTER";
  } else if (_value[pushed].type == JustifyCenterHorizontal) {
    foo = "JUSTIFY_CENTER_HORIZONTAL";
  } else {
    foo = "JUSTIFY_CENTER_VERTICAL";
  }
  if (_value[pushed].spacing > STEP_SPACING_DEFAULT ||
      _value[pushed].spacing < STEP_SPACING_DEFAULT) {
    foo += QString(" SPACING %1").arg(double(_value[pushed].spacing),4,'f',2);
  }
  return LeafMeta::format(local,global,foo);
}
void JustifyStepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( JUSTIFY_LEFT | JUSTIFY_CENTER | JUSTIFY_CENTER_HORIZONTAL | JUSTIFY_CENTER_VERTICAL )";
}
void JustifyStepMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " JUSTIFY_LEFT JUSTIFY_CENTER JUSTIFY_CENTER_HORIZONTAL JUSTIFY_CENTER_VERTICAL";
}
/* ------------------ */

PageOrientationMeta::PageOrientationMeta() : LeafMeta()
{
  type[0] = Landscape;
  type[1] = Landscape;
}
Rc PageOrientationMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(PORTRAIT|LANDSCAPE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
    type[pushed] = OrientationEnc(tokenMap[argv[index]]);
    _here[pushed] = here;
    return PageOrientationRc;
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected PORTRAIT or LANDSCAPE but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString PageOrientationMeta::format(bool local, bool global)
{
  QString foo;
  if (type[pushed] == Portrait) {
    foo = "PORTRAIT";
  } else {
    foo = "LANDSCAPE";
  }
  return LeafMeta::format(local,global,foo);
}
void PageOrientationMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( PORTRAIT | LANDSCAPE )";
}
void PageOrientationMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " PORTRAIT LANDSCAPE";
}

/* ------------------ */

CountInstanceMeta::CountInstanceMeta() : LeafMeta()
{
  countInstanceMap["TRUE"]     = CountTrue;
  countInstanceMap["FALSE"]    = CountFalse;
  countInstanceMap["AT_TOP"]   = CountAtTop;
  countInstanceMap["AT_MODEL"] = CountAtModel;
  countInstanceMap["AT_STEP"]  = CountAtStep;
  type[0] = CountAtModel;
  type[1] = CountAtModel;
}

Rc CountInstanceMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(AT_TOP|AT_MODEL|AT_STEP|TRUE|FALSE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
      type[pushed]  = CountInstanceEnc(countInstanceMap[argv[index]]);;
      _here[pushed] = here;
      return CountInstanceRc;
    }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected AT_TOP, AT_MODEL, AT_STEP, TRUE or FALSE but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString CountInstanceMeta::format(bool local, bool global)
{
  QString foo;
  switch (type[pushed])
  {
  case CountAtTop:
    foo = "AT_TOP";
    break;
  case CountAtModel:
    foo = "AT_MODEL";
    break;
  case CountAtStep:
    foo = "AT_STEP";
    break;
  case CountTrue:
    foo = "TRUE";
    break;
  default: /*CountFalse*/
    foo = "FALSE";
    break;
  }
  return LeafMeta::format(local,global,foo);
}
void CountInstanceMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( AT_TOP | AT_MODEL | AT_STEP | TRUE | FALSE )";
}
void CountInstanceMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " AT_TOP AT_MODEL AT_STEP";
}

/* ------------------ */

ContStepNumMeta::ContStepNumMeta() : LeafMeta()
{
  contStepNumMap["FALSE"] = ContStepNumFalse;
  contStepNumMap["TRUE"]  = ContStepNumTrue;
  type[0] = ContStepNumFalse;
  type[1] = ContStepNumFalse;
}

Rc ContStepNumMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(TRUE|FALSE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
    type[pushed]  = ContStepNumEnc(contStepNumMap[argv[index]]);;
    _here[pushed] = here;
    return ContStepNumRc;
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected TRUE or FALSE but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString ContStepNumMeta::format(bool local, bool global)
{
  QString foo;
  switch (type[pushed])
  {
  case ContStepNumTrue:
    foo = "TRUE";
    break;
  default: /*ContStepNumFalse*/
    foo = "FALSE";
    break;
  }
  return LeafMeta::format(local,global,foo);
}
void ContStepNumMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRUE | FALSE )";
}

void ContStepNumMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " TRUE FALSE";
}

/* ------------------ */

BuildModEnabledMeta::BuildModEnabledMeta() : LeafMeta()
{
  buildModEnabledMap["FALSE"] = BuildModEnabledFalse;
  buildModEnabledMap["TRUE"]  = BuildModEnabledTrue;
  BuildModEnabledEnc enabled = Preferences::buildModEnabled ? BuildModEnabledTrue : BuildModEnabledFalse;
  type[0] = enabled;
  type[1] = enabled;
}

Rc BuildModEnabledMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(TRUE|FALSE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
    type[pushed]  = BuildModEnabledEnc(buildModEnabledMap[argv[index]]);
    _here[pushed] = here;
    return BuildModEnableRc;
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected TRUE or FALSE but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString BuildModEnabledMeta::format(bool local, bool global)
{
  QString foo;
  switch (type[pushed])
  {
  case BuildModEnabledFalse:
    foo = "FALSE";
    break;
  default: /*BuildModEnabledTrue*/
    foo = "TRUE";
    break;
  }
  return LeafMeta::format(local,global,foo);
}

void BuildModEnabledMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRUE | FALSE )";
}

void BuildModEnabledMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble;
}


/* ------------------ */

FinalModelEnabledMeta::FinalModelEnabledMeta() : LeafMeta()
{
  finalModelEnabledMap["FALSE"] = FinalModelEnabledFalse;
  finalModelEnabledMap["TRUE"]  = FinalModelEnabledTrue;
  FinalModelEnabledEnc enabled  = Preferences::finalModelEnabled ? FinalModelEnabledTrue : FinalModelEnabledFalse;
  type[0] = enabled;
  type[1] = enabled;
}

Rc FinalModelEnabledMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(TRUE|FALSE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
      type[pushed]  = FinalModelEnabledEnc(finalModelEnabledMap[argv[index]]);
      _here[pushed] = here;
      return FinalModelEnableRc;
    }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected TRUE or FALSE but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString FinalModelEnabledMeta::format(bool local, bool global)
{
  QString foo;
  switch (type[pushed])
  {
  case FinalModelEnabledFalse:
    foo = "FALSE";
    break;
  default: /*FinalModelEnabledTrue*/
    foo = "TRUE";
    break;
  }
  return LeafMeta::format(local,global,foo);
}

void FinalModelEnabledMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRUE | FALSE )";
}

void FinalModelEnabledMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

void PageSizeMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc PageSizeMeta::parse(QStringList &argv, int index,Where &here)
{
  static QRegularExpression rx("^(PORTRAIT|LANDSCAPE)$");
  bool ok[2];
  bool sizeIDFound = false;
  bool sizeWandHFound = false;
  float v0 = 0.0f, v1 = 0.0f;
  int orientationIndex = -1, sizeIdIndex = -1;

  if (argv.size() - index >= 2) {
    v0 = argv[index  ].toFloat(&ok[0]);
    v1 = argv[index+1].toFloat(&ok[1]);
    // we have 2 items that are not numbers
    // so check which is orientation and sizeID
    if (!ok[0] && !ok[1]) {
      if (argv[index].contains(rx)) {
        sizeIdIndex = index+1;
        orientationIndex = index;
      } else if (argv[index+1].contains(rx)) {
        sizeIdIndex = index;
        orientationIndex = index+1;
      }
      sizeIDFound = !(argv[sizeIdIndex].toLower() == "custom");
    } else {
      sizeWandHFound = true;
    }
  }

  if (sizeWandHFound) {
    if (v0 < _min || v0 > _max ||
      v1 < _min || v1 > _max) {
      return RangeErrorRc;
    }
    _value[pushed].sizeW = v0;
    _value[pushed].sizeH = v1;

    if (argv.size() - index == 3) {
      if (argv[index+2].contains(rx)) {
        _value[pushed].orientation = OrientationEnc(tokenMap[argv[index+2]]);
        _value[pushed].sizeID = "Custom";
      } else {
        _value[pushed].sizeID = argv[index+2];
        _value[pushed].orientation = InvalidOrientation;
      }
    } else if (argv.size() - index == 4) {
      if (argv[index+2].contains(rx)) {
        sizeIdIndex = index+3;
        orientationIndex = index+2;
      } else if (argv[index+3].contains(rx)) {
        sizeIdIndex = index+2;
        orientationIndex = index+3;
      }
      _value[pushed].orientation = OrientationEnc(tokenMap[argv[orientationIndex]]);
      _value[pushed].sizeID = argv[sizeIdIndex];
    } else {
      _value[pushed].sizeID = "Custom";
      _value[pushed].orientation = InvalidOrientation;
    }
    _here[pushed] = here;
    return PageSizeRc;

  } else if (sizeIDFound || ((argv.size() - index == 1) && (argv[index].toLower() != "custom"))) {
    QString sizeID = argv[sizeIDFound ? sizeIdIndex : index];
    bool dpi = lpub->page.meta.LPub.resolution.type() == DPI;
    int  numPageTypes = PageSizes::numPageTypes();
    for (int i = 0; i < numPageTypes; i++) {
      if (sizeID.toLower() == PageSizes::pageTypeSizeID(i).toLower()) {
        v0 = dpi ? PageSizes::pageWidthIn(i) : PageSizes::pageWidthCm(i);
        v1 = dpi ? PageSizes::pageHeightIn(i) : PageSizes::pageHeightCm(i);
        break;
      }
    }

    if (v0 == 0.0f || v1 == 0.0f ) {
      return FailureRc;
    }

    if (v0 < _min || v0 > _max ||
      v1 < _min || v1 > _max) {
      return RangeErrorRc;
    }

    _value[pushed].sizeW       = v0;
    _value[pushed].sizeH       = v1;
    _value[pushed].sizeID      = sizeID;
    _value[pushed].orientation = sizeIDFound ? OrientationEnc(tokenMap[argv[orientationIndex]]) : InvalidOrientation;
    _here[pushed] = here;
    return PageSizeRc;
  }

  // suppress 'PORTRAIT or LANDSCAPE'  for now
  // "Expected two decimal numbers and/or page size id (e.g. A4, Letter, Custom...) and optionally PORTRAIT or LANDSCAPE but got \"%1\""
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected two decimal numbers and/or page size id (e.g. A4, Letter, Custom...) but got \"%1\"") .arg(argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }

  return FailureRc;
}
QString PageSizeMeta::format(bool local, bool global)
{
  QString foo = QString("%1 %2 %3")
                        .arg(double(_value[pushed].sizeW),_fieldWidth,'f',_precision)
                        .arg(double(_value[pushed].sizeH),_fieldWidth,'f',_precision)
                        .arg(_value[pushed].sizeID);
  if (_value[pushed].orientation == Portrait) {
    foo += QStringLiteral(" PORTRAIT");
  } else if (_value[pushed].orientation == Landscape) {
    foo += QStringLiteral(" LANDSCAPE");
  }

  return LeafMeta::format(local,global,foo.trimmed());
}
void PageSizeMeta::doc(QStringList &out, QString preamble)
{
  // suppress 'PORTRAIT or LANDSCAPE'  for now
  // " <decimal width> <decimal height> [ <\"page size id\"> ] [ PORTRAIT | LANDSCAPE ] | <\"page size id\"> [ PORTRAIT | LANDSCAPE ]"
  out << preamble + " <decimal width> <decimal height> [ <\"page size id\"> ] | <\"page size id\">";
}

/* ------------------ */

SepMeta::SepMeta() : LeafMeta()
{
  _value[0].type      = SepData::Default;
  _value[0].color     = "black";
  _value[0].length    = -1.0;
  _value[0].thickness = DEFAULT_LINE_THICKNESS;
  _value[0].margin[0] = DEFAULT_MARGIN;
  _value[0].margin[1] = DEFAULT_MARGIN;

  _value[1].type      = SepData::Default;
  _value[1].color     = "black";
  _value[1].length    = -1.0;
  _value[1].thickness = DEFAULT_LINE_THICKNESS;
  _value[1].margin[0] = DEFAULT_MARGIN;
  _value[1].margin[1] = DEFAULT_MARGIN;
}
Rc SepMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  bool good, ok;
  if (argv.size() - index == 4) {
    argv[index  ].toFloat(&good);
    argv[index+2].toFloat(&ok);
    good &= ok;
    argv[index+3].toFloat(&ok);
    good &= ok;
    if (good) {
      _value[pushed].thickness = argv[index].toFloat();
      _value[pushed].color     = argv[index+1];
      _value[pushed].margin[0] = argv[index+2].toFloat();
      _value[pushed].margin[1] = argv[index+3].toFloat();
      _here[pushed] = here;
      rc = OkRc;
    }
  } else
  if (argv.size() - index == 5) {       // legacy
    argv[index+1].toFloat(&good);
    argv[index+3].toFloat(&ok);
    good &= ok;
    argv[index+4].toFloat(&ok);
    good &= ok;
    if (good) {
      // backward compatibility - ticket #193
      QString sepLen = argv[index];
      if (argv[index] == "PAGE")
        sepLen = "PAGE_LENGTH";
      _value[pushed].type      = SepData::LengthType(tokenMap[sepLen]);
      // end backward compatibility
      _value[pushed].thickness = argv[index+1].toFloat();
      _value[pushed].color     = argv[index+2];
      _value[pushed].margin[0] = argv[index+3].toFloat();
      _value[pushed].margin[1] = argv[index+4].toFloat();
      _here[pushed] = here;
      rc = OkRc;
    }
  } else // PAGE_LENGTH
  if (argv.size() - index == 7) {
    argv[index+2].toFloat(&good);
    argv[index+5].toFloat(&ok);
    good &= ok;
    argv[index+6].toFloat(&ok);
    good &= ok;
    if (good) {
      _value[pushed].type      = SepData::LengthType(tokenMap[argv[index]]);
      _value[pushed].thickness = argv[index+2].toFloat();
      _value[pushed].color     = argv[index+3];
      _value[pushed].margin[0] = argv[index+5].toFloat();
      _value[pushed].margin[1] = argv[index+6].toFloat();
      _here[pushed] = here;
      rc = OkRc;
    }
  } else
  if (argv.size() - index == 6) {
    static QRegularExpression rx("CUSTOM|CUSTOM_LENGTH"); // legacy
    if (argv[index].contains(rx)) {
      argv[index+1].toFloat(&good);
      argv[index+2].toFloat(&ok);
      good &= ok;
      argv[index+4].toFloat(&ok);
      good &= ok;
      argv[index+5].toFloat(&ok);
      good &= ok;
      if (good) {
        // backward compatibility - ticket #193
        QString sepLen = argv[index];
        if (argv[index] == "CUSTOM")
          sepLen = "CUSTOM_LENGTH";
        _value[pushed].type      = SepData::LengthType(tokenMap[sepLen]);
        // end backward compatibility
        _value[pushed].length    = argv[index+1].toFloat();
        _value[pushed].thickness = argv[index+2].toFloat();
        _value[pushed].color     = argv[index+3];
        _value[pushed].margin[0] = argv[index+4].toFloat();
        _value[pushed].margin[1] = argv[index+5].toFloat();
        _here[pushed] = here;
        rc = OkRc;
      }
    } else
      if (argv[index].contains("THICKNESS")) {
        argv[index+1 ].toFloat(&good);
        argv[index+4].toFloat(&ok);
        good &= ok;
        argv[index+5].toFloat(&ok);
        good &= ok;
        if (good) {
          _value[pushed].thickness = argv[index+1].toFloat();
          _value[pushed].color     = argv[index+2];
          _value[pushed].margin[0] = argv[index+4].toFloat();
          _value[pushed].margin[1] = argv[index+5].toFloat();
          _here[pushed] = here;
          rc = OkRc;
        }
      }
    } else // CUSTOM_LENGTH
    if (argv.size() - index == 8) {
        argv[index+1].toFloat(&good);
        argv[index+3].toFloat(&ok);
        good &= ok;
        argv[index+6].toFloat(&ok);
        good &= ok;
        argv[index+7].toFloat(&ok);
        good &= ok;
        if (good) {
            _value[pushed].type      = SepData::LengthType(tokenMap[argv[index]]);
            _value[pushed].length    = argv[index+1].toFloat();
            _value[pushed].thickness = argv[index+3].toFloat();
            _value[pushed].color     = argv[index+4];
            _value[pushed].margin[0] = argv[index+6].toFloat();
            _value[pushed].margin[1] = argv[index+7].toFloat();
            _here[pushed] = here;
            rc = OkRc;
          }
      }
  if (rc == FailureRc) {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Malformed separator \"%1\"") .arg(argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
  }
  return SepRc;
}
QString SepMeta::format(bool local, bool global)
{
  QString foo;
  if (_value[pushed].type == SepData::LenCustom) {
    foo = QString("%1 %2 %3 %4 \"%5\" %6 %7 %8")
                .arg("CUSTOM_LENGTH")
                .arg(double(_value[pushed].length))
                .arg("THICKNESS")
                .arg(double(_value[pushed].thickness))
                .arg(_value[pushed].color,
                     "MARGINS")
                .arg(double(_value[pushed].margin[0]),
                     double(_value[pushed].margin[1]));
  } else
  if (_value[pushed].type == SepData::LenPage) {
    foo = QString("%1 %2 %3 \"%4\" %5 %6 %7")
                  .arg("PAGE_LENGTH",
                       "THICKNESS")
                  .arg(double(_value[pushed].thickness))
                  .arg(_value[pushed].color,
                       "MARGINS")
                  .arg(double(_value[pushed].margin[0]),
                       double(_value[pushed].margin[1]));
  } else {
    foo = QString("%1 %2 \"%3\" %4 %5 %6")
                  .arg("THICKNESS")
                  .arg(double(_value[pushed].thickness))
                  .arg(_value[pushed].color,
                       "MARGINS")
                  .arg(double(_value[pushed].margin[0]),
                       double(_value[pushed].margin[1]));
  }
  return LeafMeta::format(local,global,foo);
}
void SepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " [( PAGE_LENGTH | CUSTOM_LENGTH ) <decimal> ] THICKNESS <integer> <\"color name\"|\"#RRGGBB\"> MARGINS <decimal X> <decimal Y>";
}

void SepMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " PAGE_LENGTH CUSTOM_LENGTH THICKNESS MARGINS";
}
/* ------------------ */
/*
 * Scene Depth Meta
 */

Rc SceneObjectMeta::parse(QStringList &argv, int index, Where &here)
{
  static QRegularExpression rx("^(BRING_TO_FRONT|SEND_TO_BACK)$");
  if (argv.size() - index == 3 && argv[index].contains(rx)) {
    bool good, ok;
    float x = argv[index+1].toFloat(&good);
    float y = argv[index+2].toFloat(&ok);
    good &= ok;
    if (good) {
      _value[pushed].direction   = SceneObjectDirection(tokenMap[argv[index]]);
      _value[pushed].scenePos[0] = x;
      _value[pushed].scenePos[1] = y;
      _value[pushed].armed = true;            // not used
      _here[pushed] = here;
      return SceneItemZValueDirectionRc;
    }
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Expected BRING_TO_FRONT|SEND_TO_BACK <decimal X pos> <decimal Y pos> but got \"%1\" %2")
                                            .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}

QString SceneObjectMeta::format(bool local, bool global)
{
  QString foo;
  if (_value[pushed].direction == SendToBack) {
    foo = "SEND_TO_BACK";
  } else {
    foo = "BRING_TO_FRONT";
  }
  foo += QString(" %1 %2").arg(double(_value[pushed].scenePos[0]))
                          .arg(double(_value[pushed].scenePos[1]));

  return LeafMeta::format(local,global,foo);
}

void SceneObjectMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( BRING_TO_FRONT | SEND_TO_BACK ) <decimal X pos> <decimal Y pos>";
}

void SceneObjectMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " BRING_TO_FRONT SEND_TO_BACK";
}
/* ------------------ */

StudStyleMeta::StudStyleMeta() : LeafMeta()
{
  studStyleMap["NONE"]                    = StylePlain;
  studStyleMap["PLAIN"]                   = StylePlain;
  studStyleMap["THIN_LINE_LOGO"]          = StyleThinLineLogo;
  studStyleMap["OUTLINE_LOGO"]            = StyleOutlineLogo;
  studStyleMap["SHARP_TOP_LOGO"]          = StyleSharpTopLogo;
  studStyleMap["ROUNDED_TOP_LOGO"]        = StyleRoundedTopLogo;
  studStyleMap["FLATTENED_LOGO"]          = StyleFlattenedLogo;
  studStyleMap["HIGH_CONTRAST"]           = StyleHighContrast;
  studStyleMap["HIGH_CONTRAST_WITH_LOGO"] = StyleHighContrastWithLogo;
  type[0] = static_cast<StudStyleEnc>(lcGetProfileInt(LC_PROFILE_STUD_STYLE));
  type[1] = static_cast<StudStyleEnc>(lcGetProfileInt(LC_PROFILE_STUD_STYLE));
}

Rc StudStyleMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = OkRc;
  static QRegularExpression rx("^(NONE|PLAIN|THIN_LINE_LOGO|OUTLINE_LOGO|SHARP_TOP_LOGO|ROUNDED_TOP_LOGO|FLATTENED_LOGO|HIGH_CONTRAST|HIGH_CONTRAST_WITH_LOGO)$");
  if (argv.size() - index == 1) {
    if (!argv[index].contains(rx)) {
      bool ok;
      int styleId = argv[index].toInt(&ok);
      if (ok && styleId >= StylePlain && styleId <= StyleHighContrastWithLogo) {
        type[pushed] = StudStyleEnc(styleId);
      } else {
        rc = FailureRc;
      }
    } else {
      type[pushed]  = StudStyleEnc(studStyleMap[argv[index]]);;
    }
    if (rc == OkRc) {
      _here[pushed] = here;
    }
  } else {
    rc = FailureRc;
  }
  if (rc == FailureRc && reportErrors) {
    QString const message = QMessageBox::tr("Expected PLAIN, THIN_LINE_LOGO, OUTLINE_LOGO, SHARP_TOP_LOGO, ROUNDED_TOP_LOGO, FLATTENED_LOGO, HIGH_CONTRAST, HIGH_CONTRAST_WITH_LOGO or 1 - 7, but got \"%1\" in \"%2\"")
                                            .arg(argv[index], argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return rc;
}

QString StudStyleMeta::format(bool local, bool global)
{
  QString foo;
  switch (type[pushed])
  {
  case StyleThinLineLogo:
    foo = "THIN_LINE_LOGO";
    break;
  case StyleOutlineLogo:
    foo = "OUTLINE_LOGO";
    break;
  case StyleSharpTopLogo:
    foo = "SHARP_TOP_LOGO";
    break;
  case StyleRoundedTopLogo:
    foo = "ROUNDED_TOP_LOGO";
    break;
  case StyleFlattenedLogo:
    foo = "FLATTENED_LOGO";
    break;
  case StyleHighContrast:
    foo = "HIGH_CONTRAST";
    break;
  case StyleHighContrastWithLogo:
    foo = "HIGH_CONTRAST_WITH_LOGO";
    break;
  default: /*StylePlain*/
    foo = "PLAIN";
    break;
  }
  return LeafMeta::format(local,global,foo);
}

void StudStyleMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( PLAIN | THIN_LINE_LOGO | OUTLINE_LOGO | SHARP_TOP_LOGO | ROUNDED_TOP_LOGO | FLATTENED_LOGO | HIGH_CONTRAST | HIGH_CONTRAST_WITH_LOGO | <stud style integer 0-7> )";
}

void StudStyleMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " PLAIN THIN_LINE_LOGO OUTLINE_LOGO SHARP_TOP_LOGO ROUNDED_TOP_LOGO FLATTENED_LOGO HIGH_CONTRAST HIGH_CONTRAST_WITH_LOGO";
}

/* ------------------ */

ColorMeta::ColorMeta() : LeafMeta()
{
  _value[0] = 0;
  _value[1] = 0;
  _format   = 2; // String: 0-255,0-255,0-255,0-255
}

Rc ColorMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = OkRc;
  if (argv.size() - index == 1) {
    static QRegularExpression hexRx("\\s*(0x|#)([\\da-fA-F]+)\\s*$");
    QColor color;
    if (argv[index].contains(hexRx)) {
      color = QColor(argv[index]);
      _format = argv[index].length() == 7 ? QColor::HexRgb : QColor::HexArgb;
    } else {
      color = QColor(getRGBAFromString(argv[index]));
    }
    if (color.isValid())
      _value[pushed] = getRGBA(color.red(), color.green(), color.blue(), color.alpha());
    else
      rc = FailureRc;
    if (rc == OkRc)
      _here[pushed] = here;
  } else {
    rc = FailureRc;
  }
  if (rc == FailureRc && reportErrors) {
    emit gui->messageSig(LOG_ERROR, QMessageBox::tr("Invalid color meta command, expected "
                                                    "<\"0x|#><[AA]RRGGBB\">, or 0-255,0-255,0-255,0-255, "
                                                    "but got \"%1\" in \"%2\"") .arg(argv[index], argv.join(" ")));
  }
  return rc;
}

QString ColorMeta::format(bool local, bool global)
{
  QString foo = QString("\"%1\"").arg(_format == 2 ? getRGBAString(_value[pushed]) :
                                      _format == QColor::HexRgb ? QColor(_value[pushed]).name(QColor::HexRgb) :
                                                 QColor(_value[pushed]).name(QColor::HexArgb));
  return LeafMeta::format(local,global,foo);
}

void ColorMeta::getRGBA(quint32 color, int& r, int& g, int& b, int& a)
{
  // Colors are stored in RGBA format.
  r = color & 0xFF;
  g = (color >> 8) & 0xFF;
  b = (color >> 16) & 0xFF;
  a = (color >> 24) & 0xFF;
}

quint32 ColorMeta::getRGBA(int r, int g, int b, int a)
{
  return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

quint32 ColorMeta::getRGBAFromString(const QString &value)
{
  if (!value.isEmpty()) {
    int n, r, g, b, a;
    n = sscanf(value.toLatin1().constData(), "%d,%d,%d,%d", &r, &g, &b, &a);
    if (n == 3 || n == 4)
      return getRGBA(r, g, b, (n == 3 ? 255 : a));
  }
  return 0;
}

QString ColorMeta::getRGBAString(const quint32 rgba)
{
  int r, g, b, a;
  getRGBA(rgba, r, g, b, a);
  char v[128];
  sprintf(v, "%d,%d,%d,%d", r, g, b, a);
  return QString(v);
}

void ColorMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <\"#[AA]RRGGBB\"> | <0-255, 0-255, 0-255, 0-255>";
}

void ColorMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

/*
 *  INSERT (
 *   PICTURE "name" (SCALE x)
 *   ARROW HDX HDY TLX TLY HD HFX HFY
 *   TEXT "text" "font" color (PLACEMENT)
 *   RICH_TEXT | HTML_TEXT "text" (PLACEMENT)
 *   PAGE
 *   BOM
 *   MODEL
 *   DISPLAY_MODEL
 *   COVER_PAGE
 *   ROTATE_ICON)
 *   OFFSET x y
 */

void InsertMeta::initPlacement()
{
    RectPlacement placement = _value.rectPlacement;
   _value.placement         = PlacementEnc(  placementDecode[placement][0]);
   _value.justification     = PlacementEnc(  placementDecode[placement][1]);
   _value.preposition       = PrepositionEnc(placementDecode[placement][2]);
}

Rc InsertMeta::parse(QStringList &argv, int index, Where &here)
{
  InsertData insertData;
  bool displayModel = false;
  Rc rc = OkRc;

  if (argv.size() - index == 1) {
    if (argv[index] == "PAGE") {
      rc = InsertPageRc;
    } else if (argv[index] == "MODEL") {
      rc = InsertFinalModelRc;
    } else if ((displayModel = argv[index] == "DISPLAY_MODEL")) {
      rc = InsertDisplayModelRc;
    } else if (argv[index] == "COVER_PAGE") {
      rc = InsertCoverPageRc;
    }
  } else if (argv.size() - index == 2) {
    if (argv[index] == "COVER_PAGE") {
      rc = InsertCoverPageRc;
    }
  }

  if (rc != OkRc && !Gui::abortProcess()) {
    if (Gui::pageProcessRunning != PROC_NONE) {
      QString line;
      bool errorFound = false;
      here.setModelIndex(lpub->ldrawFile.getSubmodelIndex(here.modelName));
      Where start(here.modelName,here.modelIndex,here.lineNumber);
      Where top(here.modelName,here.modelIndex,0);
      lpub->ldrawFile.skipHeader(top.modelName,top.lineNumber);
      static QRegularExpression errorRx("(^[1-5]\\s+)|(\\bBEGIN SUB\\b)|(\\b0 STEP\\b|\\b0 ROTSTEP\\b)");
      static QRegularExpression stepRx("0 STEP|0 ROTSTEP");
      QRegularExpressionMatch match;
      for (; start.lineNumber > top.lineNumber; start--) {
        line = lpub->ldrawFile.readLine(start.modelName,start.lineNumber);
        match = errorRx.match(line);
        if (match.hasMatch()) {
          errorFound = !match.captured(3).contains(stepRx);
          break;
        }
      }

      if (!errorFound) {
        start = here;
        errorRx.setPattern("^[1-5]\\s+|(\\bBEGIN SUB\\b)");
        if (lpub->ldrawFile.size(here.modelName) > here.lineNumber) {
          start++;                                            // advance past current line
        }
        if (displayModel) {                                     // check for substitute command
          errorFound = Gui::stepContains(start, errorRx, line, 1, displayModel/*allow type 1-5 lines if display model*/);
        } else {
          top = start;
          static QRegularExpression dispModRx("^0\\s+!?(?:LPUB)*\\s?(INSERT DISPLAY_MODEL)[^\n]*");
          if (! Gui::stepContains(top, dispModRx)) {           // Check if step contain display model
            line = QObject::tr("type 1-5 line");
            errorFound = Gui::stepContains(start, errorRx); // check for type 1-5 parts
          }
        }
      }

      if (errorFound) {
        rc = FailureRc;
        QString const message = QObject::tr("Step with INSERT %1 meta command cannot contain %2. Invalid type at line %3")
                                            .arg(argv[index], line).arg(start.lineNumber+1);
        emit gui->parseErrorSig(message, here, Preferences::InsertErrors, false/*option*/, false/*override*/, 3/*critical*/);
        Gui::setAbortProcess(true);
      }
    }
    _here[0] = here;
    return rc;
  }

  if (argv.size() - index > 1 && argv[index] == "PICTURE") {
    insertData.type = InsertData::InsertPicture;
    insertData.picName = argv[++index];
    ++index;
    if (argv.size() - index >= 2 && argv[index] == "SCALE") {
      bool good;
      insertData.picScale = double(argv[++index].toFloat(&good));
      ++index;

      if (! good) {
        rc = FailureRc;
      }
    }
  } else if (argv.size() - index > 3 && argv[index] == "TEXT") {
    insertData.type = InsertData::InsertText;
    insertData.placementCommand = argv[index+1] == "PLACEMENT";
    if (insertData.placementCommand) {
      bool local = argv[index+2] == "LOCAL";
      PlacementMeta plm;
      plm.parse(argv,local ? index+3 : index+2,here);
      insertData.defaultPlacement = false;
      insertData.placement     = plm.value().placement;
      insertData.justification = plm.value().justification;
      insertData.relativeTo    = plm.value().relativeTo;
      insertData.preposition   = plm.value().preposition;
      insertData.rectPlacement = plm.value().rectPlacement;
      insertData.offsets[0]    = plm.value().offsets[XX];
      insertData.offsets[1]    = plm.value().offsets[YY];
    } else {
      insertData.text       = argv[++index];
      insertData.textFont   = argv[++index];
      insertData.textColor  = argv[++index];
      ++index;
    }

  } else if (argv.size() - index >= 2 && (argv[index] == "RICH_TEXT" || argv[index] == "HTML_TEXT")) {
    insertData.type = InsertData::InsertRichText;
    insertData.placementCommand = argv[index+1] == "PLACEMENT";
    if (insertData.placementCommand) {
        bool local = argv[index+2] == "LOCAL";
        PlacementMeta plm;
        plm.parse(argv,local ? index+3 : index+2,here);
        insertData.defaultPlacement = false;
        insertData.placement     = plm.value().placement;
        insertData.justification = plm.value().justification;
        insertData.relativeTo    = plm.value().relativeTo;
        insertData.preposition   = plm.value().preposition;
        insertData.rectPlacement = plm.value().rectPlacement;
        insertData.offsets[0]    = plm.value().offsets[XX];
        insertData.offsets[1]    = plm.value().offsets[YY];
    } else {
        insertData.text   = argv[++index];
        ++index;
    }
  } else if (argv[index] == "ROTATE_ICON") {
    insertData.type = InsertData::InsertRotateIcon;
    ++index;
  } else if (argv.size() - index >= 8 && argv[index] == "ARROW") {
    insertData.type = InsertData::InsertArrow;
    bool good, ok;
    insertData.arrowHead.setX(argv[++index] .toDouble(&good));
    insertData.arrowHead.setY(argv[++index] .toDouble(&ok));
    good &= ok;
    insertData.arrowTail.setX(argv[++index] .toDouble(&ok));
    good &= ok;
    insertData.arrowTail.setY(argv[++index] .toDouble(&ok));
    good &= ok;
    insertData.haftingDepth = argv[++index] .toDouble(&ok);
    good &= ok;
    insertData.haftingTip.setX(argv[++index].toDouble(&ok));
    good &= ok;
    insertData.haftingTip.setY(argv[++index].toDouble(&ok));
    good &= ok;
    if ( ! good) {
        rc = FailureRc;
      }
    ++index;

  } else if (argv[index] == "BOM") {
    if (argv.size() - index >= 2) {
        insertData.bomToEndOfSubmodel = argv[index + 1] == "FOR_SUBMODEL";
        index++;
      }
    insertData.type = InsertData::InsertBom;
    insertData.where.modelName = here.modelName;
    insertData.where.lineNumber = here.lineNumber;
    ++index; // move index to end for next block
  }

  if (rc == OkRc) {
    if (!insertData.placementCommand) {
      if (argv.size() - index == 3) {
        if (argv[index] == "OFFSET") {
          bool ok[2];
          insertData.offsets[0] = argv[++index].toFloat(&ok[0]);
          insertData.offsets[1] = argv[++index].toFloat(&ok[1]);
          if ( ! ok[0] || ! ok[1]) {
            rc = FailureRc;
          }
        }
      } else if (argv.size() - index > 0) {
        rc = FailureRc;
      }
    }
  }

  if (rc == OkRc) {
    _value   = insertData;
    _here[0] = here;

    return InsertRc;
  } else {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Malformed Insert metacommand \"%1\"\n").arg(argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
    return FailureRc;
  }
}

QString InsertMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value.type) {
    case InsertData::InsertPicture:
      foo += " PICTURE \"" + _value.picName + "\"";
      if (_value.picScale > 0.0) {
          foo += QString(" SCALE %1") .arg(_value.picScale);
        }
      break;
    case InsertData::InsertText:
      foo += QString("TEXT \"%1\" \"%2\" \"%3\"") .arg(_value.text, _value.textFont, _value.textColor);
      break;
    case InsertData::InsertRichText:
      foo += QString("RICH_TEXT \"%1\"") .arg(_value.text);
      break;
    case InsertData::InsertRotateIcon:
      foo += " ROTATE_ICON";
      break;
    case InsertData::InsertArrow:
      foo += " ARROW";
      foo += QString(" %1") .arg(_value.arrowHead.rx());
      foo += QString(" %1") .arg(_value.arrowHead.ry());
      foo += QString(" %1") .arg(_value.arrowTail.rx());
      foo += QString(" %1") .arg(_value.arrowTail.ry());
      foo += QString(" %1") .arg(_value.haftingDepth);
      foo += QString(" %1") .arg(_value.haftingTip.rx());
      foo += QString(" %1") .arg(_value.haftingTip.ry());
      break;
    case InsertData::InsertBom:
      foo += " BOM";
      break;
    }

  if (_value.offsets[0] != 0.0f || _value.offsets[1] != 0.0f) {
    foo += QString(" OFFSET %1 %2")
                   .arg(double(_value.offsets[0]),0,'f',4)
                   .arg(double(_value.offsets[1]),0,'f',4);
  }

  return LeafMeta::format(local,global,foo);
}

void InsertMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( PICTURE <\"file path\"> ) | ( TEXT <\"text\"> <\"#RRGGBB\"> ) | ( RICH_TEXT <\"text\"> ) | ( HTML_TEXT <\"text\"> ) | "
                    "( TEXT PLACEMENT ( TOP | BOTTOM | LEFT | CENTER | RIGHT | TOP_LEFT | TOP_RIGHT | BOTTOM_LEFT | BOTTOM_RIGHT ) (PAGE | PAGE_HEADER | PAGE_FOOTER) ( INSIDE | OUTSIDE )) | "
                    "( PAGE | BOM | MODEL | DISPLAY_MODEL | COVER_PAGE [ FRONT | BACK ] | ROTATE_ICON ) ( OFFSET <decimal X> <decimal Y> )";
}

void InsertMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " BEGIN BOM COVER_PAGE FRONT BACK DISPLAY_MODEL FOR_SUBMODEL HTML_TEXT LOCAL MODEL OFFSET PAGE PICTURE RICH_TEXT ROTATE_ICON SCALE SUB TEXT "
                    "PLACEMENT TOP BOTTOM LEFT CENTER RIGHT TOP_LEFT TOP_RIGHT BOTTOM_LEFT BOTTOM_RIGHT PAGE PAGE_HEADER PAGE_FOOTER INSIDE OUTSIDE";
}
/* ------------------ */

Rc AlignmentMeta::parse(QStringList &argv, int index, Where &here)
{
  if (argv.size() - index == 1) {
    if (argv[index] == "LEFT") {
      _value[pushed] = Qt::AlignLeft;
    } else if (argv[index] == "CENTER") {
      _value[pushed] = Qt::AlignCenter;
    } else if (argv[index] == "RIGHT") {
      _value[pushed] = Qt::AlignRight;
    }
    _here[pushed] = here;
    return OkRc;
  } else {
    return FailureRc;
  }
}

QString AlignmentMeta::format(bool local, bool global)
{
  QString foo;

  switch (_value[pushed]) {
    case Qt::AlignLeft:
      foo = " LEFT";
      break;
    case Qt::AlignCenter:
      foo = " CENTER";
      break;
    default:
      foo = " RIGHT";
    }

  return LeafMeta::format(local,global,foo);
}

void AlignmentMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( LEFT | CENTER | RIGHT )";
}

void AlignmentMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " LEFT CENTER RIGHT";
}
/* ------------------ */

void TextMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent,name);
  font     .init(this,"FONT");
  color    .init(this,"COLOR");
  alignment.init(this,"ALIGNMENT");
}

/* ------------------ */

Rc ArrowHeadMeta::parse(QStringList &argv, int index, Where &here)
{
  if (argv.size() - index == 4) {
    qreal head[4];
    bool  good, ok;

    head[0] = argv[index  ].toDouble(&good);
    head[1] = argv[index+1].toDouble(&ok);
    good &= ok;
    head[2] = argv[index+2].toDouble(&ok);
    good &= ok;
    head[3] = argv[index+3].toDouble(&ok);
    good &= ok;

    if ( ! good) {
      return FailureRc;
    }

    _value[pushed][0] = head[0];
    _value[pushed][1] = head[1];
    _value[pushed][2] = head[2];
    _value[pushed][3] = head[3];
    _here[pushed] = here;

    return OkRc;
  }
  return FailureRc;
}

QString ArrowHeadMeta::format(bool local, bool global)
{
  QString foo;
  foo  = QString(" %1 %2 %3 %4") .arg(_value[pushed][0]);
  foo += QString(" %1 %2 %3 %4") .arg(_value[pushed][1]);
  foo += QString(" %1 %2 %3 %4") .arg(_value[pushed][2]);
  foo += QString(" %1 %2 %3 %4") .arg(_value[pushed][3]);
  return LeafMeta::format(local,global,foo);
}

void ArrowHeadMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <TipX> <HaftingInsideX> <HaftingOutsideX> <HaftingOutsideY>";
}

void ArrowHeadMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

Rc ArrowEndMeta::parse(QStringList &argv, int index, Where &here)
{
  if (argv.size() - index == 1) {
    if (argv[0] == "SQUARE") {
      _value[pushed] = false;
    } else if (argv[0] == "ROUND") {
      _value[pushed] = true;
    } else {
      return FailureRc;
    }
    _here[pushed] = here;
    return OkRc;
  }
  return FailureRc;
}

QString ArrowEndMeta::format(bool local, bool global)
{
  QString foo;

  if (_value[pushed]) {
      foo = "ROUND";
    } else {
      foo = "SQUARE";
    }
  return LeafMeta::format(local,global,foo);
}

void ArrowEndMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( SQUARE | ROUND )";
}

void ArrowEndMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " SQUARE ROUND";
}
/* ------------------ */

AutoEdgeColorMeta::AutoEdgeColorMeta() : BranchMeta()
{
  contrast.setRange(0.0f,1.0f);
  contrast.setFormats(1,2,"#9.99");
  contrast.setValue(lpub->GetPartEdgeContrast());
  saturation.setRange(0.0f,1.0f);
  saturation.setFormats(1,2,"#9.99");
  saturation.setValue(lpub->GetPartColorLightDarkIndex());
  enable.setValue(lpub->GetAutomateEdgeColor());
}

void AutoEdgeColorMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  contrast     .init(this,"CONTRAST");
  saturation   .init(this,"SATURATION");
  enable       .init(this,"ENABLE");
}

/* ------------------ */

HighContrastColorMeta::HighContrastColorMeta() : BranchMeta()
{
  lightDarkIndex.setRange(0.0f,1.0f);
  lightDarkIndex.setFormats(1,2,"#9.99");
  lightDarkIndex.setValue(lpub->GetPartColorLightDarkIndex());
  studCylinderColor.setValue(lpub->GetStudCylinderColor());
  studCylinderColorEnabled.setValue(lpub->GetStudCylinderColorEnabled());
  partEdgeColor.setValue(lpub->GetPartEdgeColor());
  partEdgeColorEnabled.setValue(lpub->GetPartEdgeColorEnabled());
  blackEdgeColor.setValue(lpub->GetBlackEdgeColor());
  blackEdgeColorEnabled.setValue(lpub->GetBlackEdgeColorEnabled());
  darkEdgeColor.setValue(lpub->GetDarkEdgeColor());
  darkEdgeColorEnabled.setValue(lpub->GetDarkEdgeColorEnabled());
}

void HighContrastColorMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  lightDarkIndex    .init(this,"COLOR_LIGHT_DARK_INDEX");
  studCylinderColor .init(this,"STUD_CYLINDER_COLOR");
  studCylinderColorEnabled.init(this,"STUD_CYLINDER_COLOR_ENABLED");
  partEdgeColor     .init(this,"EDGE_COLOR");
  partEdgeColorEnabled.init(this,"EDGE_COLOR_ENABLED");
  blackEdgeColor    .init(this,"BLACK_EDGE_COLOR");
  blackEdgeColorEnabled.init(this,"BLACK_EDGE_COLOR_ENABLED");
  darkEdgeColor     .init(this,"DARK_EDGE_COLOR");
  darkEdgeColorEnabled.init(this,"DARK_EDGE_COLOR_ENABLED");
}

/*------------------------*/

SettingsMeta::SettingsMeta() : BranchMeta()
{
  // assem image generation
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"#99999.9");
  modelScale.setValue(1.0f);

  // assem native camera position
  cameraAngles.setFormats(6,1,"###9.0");
  cameraAngles.setRange(-360.0f,360.0f);
  cameraAngles.setValues(MetaDefaults::getAssemblyCameraLatitude(),MetaDefaults::getAssemblyCameraLongitude());
  cameraAngles.setType(static_cast<int>(Options::CSI));
  cameraDistance.setRange(1.0f,FLT_MAX);
  cameraFoV.setFormats(5,4,"9.999");
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(),
                     MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setFormats(5,0,"####9");
  cameraZNear.setRange(1.0f,FLT_MAX);
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setFormats(5,0,"####9");
  cameraZFar.setRange(1.0f,FLT_MAX);
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
  isOrtho.setValue(false);
  imageSize.setFormats(4,0,"###9");
  imageSize.setRange(0.0f,FLT_MAX);
  imageSize.setValues(0.0f,0.0f);
}

void SettingsMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement        .init(this,"PLACEMENT");
  margin           .init(this,"MARGINS");
  studStyle        .init(this,"STUD_STYLE");
  highContrast     .init(this,"HIGH_CONTRAST");
  autoEdgeColor    .init(this,"AUTOMATE_EDGE_COLOR");
  fadeSteps        .init(this,"FADE_STEPS");
  highlightStep    .init(this,"HIGHLIGHT_STEP");
  preferredRenderer.init(this,"PREFERRED_RENDERER");

  // assem image scale
  modelScale.init(this,"MODEL_SCALE");
  // assem native camera position
  imageSize     .init(this,"IMAGE_SIZE");
  cameraDistance.init(this,"CAMERA_DISTANCE");
  cameraFoV     .init(this,"CAMERA_FOV");
  cameraZNear   .init(this,"CAMERA_ZNEAR");
  cameraZFar    .init(this,"CAMERA_ZFAR");
  cameraAngles  .init(this,"CAMERA_ANGLES");
  isOrtho       .init(this,"CAMERA_ORTHOGRAPHIC");
  cameraName    .init(this,"CAMERA_NAME");
  target        .init(this,"CAMERA_TARGET");
  position      .init(this,"CAMERA_POSITION");
  upvector      .init(this,"CAMERA_UPVECTOR");
}

void SettingsMeta::resetCameraFoV()
{
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(), MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
}

/* ------------------ */

StepsPliMeta::StepsPliMeta() : BranchMeta()
{
  placement.setValue(TopOutside,CsiType);
  perStep.setValue(true);
}

void StepsPliMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this,"PLACEMENT");
  margin   .init(this,"MARGINS");
  perStep  .init(this,"PER_STEP");
}

/* ------------------ */

StepPliMeta::StepPliMeta() : BranchMeta()
{
  perStep.setValue(true);
}

void StepPliMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  perStep.init(this,"PER_STEP");
}

/* ------------------ */

void PliBeginMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  ignore.init(this, "IGN",    PliBeginIgnRc);
  sub.init(this, "SUB");
}

/* ------------------ */

ShowSubModelMeta::ShowSubModelMeta() : BranchMeta()
{
  placement.setValue(LeftOutside,CsiType);
  show.setValue(Preferences::showSubmodelInCallout);
}

void ShowSubModelMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this,"PLACEMENT");
  margin.init(   this,"MARGINS");
  show.init  (   this,"SHOW");
}

/* ------------------ */

NumberMeta::NumberMeta() : BranchMeta()
{
  number = 0;
  color.setValue("black");
  // font - default
}

void NumberMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  color.init(this, "FONT_COLOR");
  font.init(this, "FONT",OkRc, "\"");
  margin.init(this, "MARGINS");
}

NumberPlacementMeta::NumberPlacementMeta() : NumberMeta()
{
  placement.setValue(TopLeftInsideCorner,PageType); // fallback - see LPubMeta
  color.setValue("black");
  // font - default
}

void NumberPlacementMeta::init(
    BranchMeta *parent,
    QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  color.init(this, "FONT_COLOR");
  font.init(this, "FONT",OkRc, "\"");
  margin.init(this, "MARGINS");
}

/* ------------------ */

PageAttributeTextMeta::PageAttributeTextMeta() : BranchMeta()
{
  placement.setValue(TopLeftInsideCorner,PageType);
  display.setValue(Preferences::displayAllAttributes);
  textColor.setValue("black");
  placement.value().placement     = TopLeft;
  placement.value().justification = Center;
  placement.value().preposition   = Inside;
  placement.value().relativeTo    = PageType;
  placement.value().offsets[0]    = 0.0f;
  placement.value().offsets[1]    = 0.0f;
}

void PageAttributeTextMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  textFont.init(this, "FONT",OkRc, "\"");
  textColor.init(this, "COLOR");
  margin.init(this, "MARGINS");
  placement.init(this, "PLACEMENT");
  content.init(this, "CONTENT");
  display.init(this, "DISPLAY");
}

/* ------------------ */

PageAttributeImageMeta::PageAttributeImageMeta() : BranchMeta()
{
  placement.setValue(TopLeftInsideCorner,PageType);
  display.setValue(Preferences::displayAllAttributes);
  BorderData borderData;
  borderData.type = BorderData::BdrNone;
  borderData.line = BorderData::BdrLnNone;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_LINE_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  picScale.setRange(-10000.0,10000.0);
  picScale.setFormats(7,4,"#99999.9");
  picScale.setValue(1.0);
  margin.setValuesInches(0.0f,0.0f);
  fill.setValue(Aspect);
}

void PageAttributeImageMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  border.init(this, "BORDER");
  margin.init(this, "MARGINS");
  picScale.init(this, "SCALE");
  file.init(this, "FILE");
  display.init(this, "DISPLAY");
  fill.init(this, "FILL");
}

/* ------------------ */

PageHeaderMeta::PageHeaderMeta() : BranchMeta()
{
  placement.setValue(TopInside,PageType);
  size.setValuesInches(8.2677f,0.3000f);
  size.setRange(0.1f,1000.0f);
  size.setFormats(6,4,"9.9999");
}

void PageHeaderMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  size.init(this, "SIZE");
}

/* ------------------ */

PageFooterMeta::PageFooterMeta() : BranchMeta()
{
  placement.setValue(BottomInside,PageType);
  size.setValuesInches(8.2677f,0.3000f);
  size.setRange(0.1f,1000);
  size.setFormats(6,4,"9.9999");
}

void PageFooterMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  size.init(this, "SIZE");
}

/* ------------------ */
/*
* Scene Object Meta - Z depth management
*/

SceneItemMeta::SceneItemMeta() : BranchMeta()
{
  assemAnnotation      .setItemObj(AssemAnnotationObj);      //  0
  assemAnnotationPart  .setItemObj(AssemAnnotationPartObj);  //  1
  assem                .setItemObj(AssemObj);                //  2
  calloutBackground    .setItemObj(CalloutBackgroundObj);    //  4
  calloutInstance      .setItemObj(CalloutInstanceObj);      //  5
  calloutPointer       .setItemObj(CalloutPointerObj);       //  6
  calloutUnderpinning  .setItemObj(CalloutUnderpinningObj);  //  7
  dividerBackground    .setItemObj(DividerBackgroundObj);    //  8
  divider              .setItemObj(DividerObj);              //  9
  dividerLine          .setItemObj(DividerLineObj);          // 10
  dividerPointer       .setItemObj(DividerPointerObj);       // 11
  pointerGrabber       .setItemObj(PointerGrabberObj);       // 12
  pliGrabber           .setItemObj(PliGrabberObj);           // 13
  submodelGrabber      .setItemObj(SubmodelGrabberObj);      // 14
  insertPicture        .setItemObj(InsertPixmapObj);         // 15
  insertText           .setItemObj(InsertTextObj);           // 16
  multiStepBackground  .setItemObj(MultiStepBackgroundObj);  // 17
  multiStepsBackground .setItemObj(MultiStepsBackgroundObj); // 18
  pageAttributePixmap  .setItemObj(PageAttributePixmapObj);  // 19
  pageAttributeText    .setItemObj(PageAttributeTextObj);    // 20
  pageBackground       .setItemObj(PageBackgroundObj);       // 21
  pageNumber           .setItemObj(PageNumberObj);           // 22
  pagePointer          .setItemObj(PagePointerObj);          // 23
  partsListAnnotation  .setItemObj(PartsListAnnotationObj);  // 24
  partsListBackground  .setItemObj(PartsListBackgroundObj);  // 25
  partsListInstance    .setItemObj(PartsListInstanceObj);    // 26
  pointerFirstSeg      .setItemObj(PointerFirstSegObj);      // 27
  pointerHead          .setItemObj(PointerHeadObj);          // 28
  pointerSecondSeg     .setItemObj(PointerSecondSegObj);     // 29
  pointerThirdSeg      .setItemObj(PointerThirdSegObj);      // 30
  rotateIconBackground .setItemObj(RotateIconBackgroundObj); // 31
  reserveBackground    .setItemObj(ReserveBackgroundObj);    // 32
  stepNumber           .setItemObj(StepNumberObj);           // 33
  subModelBackground   .setItemObj(SubModelBackgroundObj);   // 34
  subModelInstance     .setItemObj(SubModelInstanceObj);     // 35
  submodelInstanceCount.setItemObj(SubmodelInstanceCountObj);// 36
  partsListPixmap      .setItemObj(PartsListPixmapObj);      // 37
  partsListGroup       .setItemObj(PartsListGroupObj);       // 38
  stepBackground       .setItemObj(StepBackgroundObj);       // 39
}

void SceneItemMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  assemAnnotation      .init(this, "CSI_ANNOTATION");       //  0 CsiAnnotationType
  assemAnnotationPart  .init(this, "CSI_ANNOTATION_PART");  //  1 CsiPartType
  assem                .init(this, "ASSEM");                //  2 CsiType
  calloutBackground    .init(this, "CALLOUT");              //  4 CalloutType
  calloutInstance      .init(this, "CALLOUT_INSTANCE");     //  5
  calloutPointer       .init(this, "CALLOUT_POINTER");      //  6
  calloutUnderpinning  .init(this, "CALLOUT_UNDERPINNING"); //  7
  dividerBackground    .init(this, "DIVIDER");              //  8
  divider              .init(this, "DIVIDER_ITEM");         //  9
  dividerLine          .init(this, "DIVIDER_LINE");         // 10
  dividerPointer       .init(this, "DIVIDER_POINTER");      // 11 DividerPointerType
  pointerGrabber       .init(this, "POINTER_GRABBER");      // 12
  pliGrabber           .init(this, "PLI_GRABBER");          // 13
  submodelGrabber      .init(this, "SUBMODEL_GRABBER");     // 14
  insertPicture        .init(this, "PICTURE");              // 15
  insertText           .init(this, "TEXT");                 // 16 TextType
  multiStepBackground  .init(this, "MULTI_STEP");           // 17 StepGroupType
  multiStepsBackground .init(this, "MULTI_STEPS");          // 18
  pageAttributePixmap  .init(this, "ATTRIBUTE_PIXMAP");     // 19
  pageAttributeText    .init(this, "ATTRIBUTE_TEXT");       // 20
  pageBackground       .init(this, "PAGE");                 // 21 PageType
  pageNumber           .init(this, "PAGE_NUMBER");          // 22 PageNumberType
  pagePointer          .init(this, "PAGE_POINTER");         // 23 PagePointerType
  partsListAnnotation  .init(this, "PLI_ANNOTATION");       // 24
  partsListBackground  .init(this, "PLI");                  // 25 PartsListType
  partsListInstance    .init(this, "PLI_INSTANCE");         // 26
  pointerFirstSeg      .init(this, "POINTER_SEG_FIRST");    // 27
  pointerHead          .init(this, "POINTER_HEAD");         // 28
  pointerSecondSeg     .init(this, "POINTER_SEG_SECOND");   // 29
  pointerThirdSeg      .init(this, "POINTER_SEG_THIRD");    // 30
  rotateIconBackground .init(this, "ROTATE_ICON");          // 31 RotateIconType
  reserveBackground    .init(this, "RESERVE");              // 32 ReserveType
  stepNumber           .init(this, "STEP_NUMBER");          // 33 StepNumberType
  subModelBackground   .init(this, "SUBMODEL_DISPLAY");     // 34 SubModelType
  subModelInstance     .init(this, "SUBMODEL_INSTANCE");    // 35
  submodelInstanceCount.init(this, "SUBMODEL_INST_COUNT");  // 36 SubmodelInstanceCountType
  partsListPixmap      .init(this, "PLI_PART");             // 37
  partsListGroup       .init(this, "PLI_PART_GROUP");       // 38
  stepBackground       .init(this, "STEP_RECTANGLE");       // 39 [StepType]
}

/* ------------------ */

Rc EnableMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = FailureRc;
  static QRegularExpression rx("^(TRUE|FALSE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
    _value[pushed] = argv[index] == "TRUE";
    _here[pushed] = here;
    if (argv[1] == "FADE_STEPS") {
      rc = EnableFadeStepsRc;
    } else if (argv[1] == "HIGHLIGHT_STEP") {
      rc = EnableHighlightStepRc;
    } else if (argv[1] == "ASSEM") {
      if (argv[2] == "FADE_STEPS")
        rc = EnableFadeStepsAssemRc;
      else if (argv[2] == "HIGHLIGHT_STEP")
        rc = EnableHighlightStepAssemRc;
    } else if (argv[1] == "MULTI_STEP") {
      if (argv[2] == "ASSEM") {
        if (argv[3] == "FADE_STEPS")
          rc = EnableFadeStepsGroupAssemRc;
        else if (argv[3] == "HIGHLIGHT_STEP")
          rc = EnableHighlightStepGroupAssemRc;
      }
    } else if (argv[1] == "CALLOUT") {
      if (argv[2] == "ASSEM") {
        if (argv[3] == "FADE_STEPS")
          rc = EnableFadeStepsCalloutAssemRc;
        else if (argv[3] == "HIGHLIGHT_STEP")
          rc = EnableHighlightStepCalloutAssemRc;
      }
    }
  }

  if (rc == FailureRc) {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Expected TRUE or FALSE, but got \"%1\" %2") .arg(argv[index], argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
  }

  return rc;
}

QString EnableMeta::format(bool local, bool global)
{
  QString foo (_value[pushed] ? "TRUE" : "FALSE");
  return LeafMeta::format(local,global,foo);
}

void EnableMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRUE | FALSE )";
}

void EnableMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " TRUE FALSE";
}

/* ------------------ */

Rc LPubFaHiMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = FailureRc;
  static QRegularExpression rx("^(TRUE|FALSE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
    _value[pushed] = argv[index] == "TRUE";
    _here[pushed] = here;
    if (argv[1] == "FADE_STEPS") {
      rc = LPubFadeRc;
    } else if (argv[1] == "HIGHLIGHT_STEP") {
      rc = LPubHighlightRc;
    } else if (argv[1] == "ASSEM") {
      if (argv[2] == "FADE_STEPS")
        rc = LPubFadeAssemRc;
      else if (argv[2] == "HIGHLIGHT_STEP")
        rc = LPubHighlightAssemRc;
    } else if (argv[1] == "MULTI_STEP") {
      if (argv[2] == "ASSEM") {
        if (argv[3] == "FADE_STEPS")
          rc = LPubFadeGroupAssemRc;
        else if (argv[3] == "HIGHLIGHT_STEP")
          rc = LPubHighlightGroupAssemRc;
      }
    } else if (argv[1] == "CALLOUT") {
      if (argv[2] == "ASSEM") {
        if (argv[3] == "FADE_STEPS")
          rc = LPubFadeCalloutAssemRc;
        else if (argv[3] == "HIGHLIGHT_STEP")
          rc = LPubHighlightCalloutAssemRc;
      }
    }
  }

  if (rc == FailureRc) {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Expected TRUE or FALSE, but got \"%1\" %2") .arg(argv[index], argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
  }

  return rc;
}

QString LPubFaHiMeta::format(bool local, bool global)
{
  QString foo (_value[pushed] ? "TRUE" : "FALSE");
  return LeafMeta::format(local,global,foo);
}

void LPubFaHiMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " ( TRUE | FALSE )";
}

void LPubFaHiMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " TRUE FALSE";
}

/* ------------------ */

Rc FadeColorMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = FailureRc;
  static QRegularExpression hexRx("^(0x|#)([\\da-fA-F]+)$");
  if (argv.size() - index >= 1) {
    QColor parsedColor = QColor();
    QRegularExpressionMatch match = hexRx.match(argv[index]);
    if (match.hasMatch())
      parsedColor = QColor(QString("#%1").arg(match.captured(2)));
    if (parsedColor.isValid()) {
      _value[pushed].color = argv[index];
      rc = OkRc;
    } else {
      parsedColor = LDrawColor::color(argv[index]);
      if (parsedColor.isValid()) {
        _value[pushed].color = argv[index];
        rc = OkRc;
      }
    }

    if (argv.size() - index == 3 && argv[index + 1] == "USE") {
      _value[pushed].useColor = argv[index + 2] == "TRUE";
    } else {
      _value[pushed].useColor = rc == OkRc;
    }
    if (rc == OkRc)
      _here[pushed] = here;
  }

  if (rc == FailureRc) {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Expected LDraw_colour_name | (0x|#)([AA]RRGGBB) [USE TRUE|FALSE], but got \"%1\"") .arg(argv[index], argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
  }

  return rc;
}

QString FadeColorMeta::format(bool local, bool global)
{
  QString foo ("\"" + _value[pushed].color + "\"");
  if (!_value[pushed].useColor)
    foo += " USE FALSE";
  return LeafMeta::format(local,global,foo);
}

void FadeColorMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <\"LDraw_colour_name\"> | <\"0x|#><[AA]RRGGBB\"> [ USE ( TRUE | FALSE ) ]";
}

void FadeColorMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " USE TRUE FALSE";
}
/* ------------------ */

FadeStepsMeta::FadeStepsMeta() : BranchMeta()
{
  opacity.setRange(0,100);
  enable.setValue(Preferences::enableFadeSteps);
  setup.setValue(false);
  FadeColorData fdata;
  fdata.color = Preferences::validFadeStepsColour;
  fdata.useColor = Preferences::fadeStepsUseColour;
  color.setValue(fdata);
  opacity.setValue(Preferences::fadeStepsOpacity);
  lpubFade.setValue(Preferences::preferredRenderer != RENDERER_NATIVE);
}

void FadeStepsMeta::setPreferences(bool reset)
{
  FadeColorData fdata = color.value();
  bool displayPreference = false;
  if (reset) {
    Preferences::fadestepPreferences();
    displayPreference = Preferences::enableFadeSteps != enable.value();
    setup.setValue(false);
    enable.setValue(Preferences::enableFadeSteps);
    fdata.color = Preferences::validFadeStepsColour;
    fdata.useColor = Preferences::fadeStepsUseColour;
    color.setValue(fdata);
    opacity.setValue(Preferences::fadeStepsOpacity);
    lpubFade.setValue(Preferences::preferredRenderer != RENDERER_NATIVE);
  } else {
    if ((displayPreference = Preferences::enableFadeSteps != enable.value() || enable.global))
      Preferences::enableFadeSteps    = enable.value();
    Preferences::validFadeStepsColour = fdata.color;
    Preferences::fadeStepsUseColour   = fdata.useColor;
    Preferences::fadeStepsOpacity     = opacity.value();
    if (Preferences::enableFadeSteps && lpubFade.value())
      gui->setupFadeOrHighlight(true, Preferences::enableFadeSteps);
    if (enable.global) {
      Preferences::fadestepPreferences(enable.global);
      if (Preferences::enableFadeSteps != enable.value())
        gui->clearCSICache();
    }
  }

  QString const message = QObject::tr("Fade previous steps %1 %2%3.")
                              .arg(reset
                                       ? QObject::tr("reset to") : enable.global
                                             ? QObject::tr("save as") : Preferences::enableFadeSteps != enable.value()
                                                   ? QObject::tr("changed to") : QObject::tr("is"),
                                  Preferences::enableFadeSteps
                                       ? QObject::tr("ON") : QObject::tr("OFF"),
                                  Preferences::enableFadeSteps
                                       ? QObject::tr(" Opacity %1%2")
                                             .arg(Preferences::fadeStepsOpacity)
                                             .arg(Preferences::enableFadeSteps && Preferences::fadeStepsUseColour && !Preferences::validFadeStepsColour.isEmpty()
                                                      ? QObject::tr(" Use Fade Color %1").arg(Preferences::validFadeStepsColour) : "")
                                       : "");
  if (displayPreference)
    emit gui->messageSig(LOG_INFO, message);

  //qDebug() << qUtf8Printable("DEBUG: "+message);
}

void FadeStepsMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  enable.init(  this, "ENABLED");
  setup.init(   this, "SETUP");
  color.init(   this, "COLOR");
  opacity.init( this, "OPACITY");
  lpubFade.init(this, "LPUB_FADE");
}

/* ------------------ */

HighlightStepMeta::HighlightStepMeta() : BranchMeta()
{
  lineWidth.setRange(0,10);
  enable.setValue(Preferences::enableHighlightStep);
  setup.setValue(false);
  color.setValue(Preferences::highlightStepColour);
  lineWidth.setValue(Preferences::highlightStepLineWidth);
  lpubHighlight.setValue(Preferences::preferredRenderer != RENDERER_NATIVE);
}

void HighlightStepMeta::setPreferences(bool reset)
{
  bool displayPreference = false;
  if (reset) {
    Preferences::highlightstepPreferences();
    displayPreference = Preferences::enableHighlightStep != enable.value();
    setup.setValue(false);
    enable.setValue(Preferences::enableHighlightStep);
    color.setValue(Preferences::highlightStepColour);
    lineWidth.setValue(Preferences::highlightStepLineWidth);
    lpubHighlight.setValue(Preferences::preferredRenderer != RENDERER_NATIVE);
  } else {
    if ((displayPreference = Preferences::enableHighlightStep != enable.value() || enable.global))
      Preferences::enableHighlightStep  = enable.value();
    Preferences::highlightStepColour    = color.value();
    Preferences::highlightStepLineWidth = lineWidth.value();
    if (Preferences::enableHighlightStep && lpubHighlight.value())
      gui->setupFadeOrHighlight(Preferences::enableFadeSteps, true);
    if (enable.global) {
      Preferences::highlightstepPreferences(enable.global);
      if (Preferences::enableHighlightStep != enable.value())
        gui->clearCSICache();
    }
  }

  QString const message = QObject::tr("Highlight current step %1 %2%3.")
                              .arg(reset
                                       ? QObject::tr("reset to") : enable.global
                                             ? QObject::tr("save as") : Preferences::enableHighlightStep != enable.value()
                                                   ? QObject::tr("changed to") : QObject::tr("is"),
                                  Preferences::enableFadeSteps
                                      ? QObject::tr("ON") : QObject::tr("OFF"),
                                  Preferences::enableHighlightStep && !Preferences::highlightStepColour.isEmpty()
                                      ? QObject::tr(" Highlight Color %1").arg(Preferences::highlightStepColour) : "");
  if (displayPreference)
    emit gui->messageSig(LOG_INFO, message);

  //qDebug() << qUtf8Printable("DEBUG: "+message);
}

void HighlightStepMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  enable.init(       this, "ENABLED");
  setup.init(        this, "SETUP");
  color.init(        this, "COLOR");
  lineWidth.init(    this, "LINE_WIDTH");
  lpubHighlight.init(this, "LPUB_HIGHLIGHT");
}

/* ------------------ */

void RemoveMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  group.init(   this,"GROUP",RemoveGroupRc);
  parttype.init(this,"PART", RemovePartTypeRc);
  partname.init(this,"NAME", RemovePartNameRc);
}

/* ------------------ */

PartMeta::PartMeta() : BranchMeta()
{
}

void PartMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent,name);
  margin.init(this,"MARGINS");
}

/* ------------------ */

Rc SubMeta::parse(QStringList &argv, int index,Where &here)
{
  bool ok[10];
  bool ldrawType = false;
  Rc rc = FailureRc;
  int argc = argv.size() - index;
  QString originalColor;
  QString attributes = "undefined;";
  static QRegularExpression rx("^(ABS|REL|ADD)$");
  if (argc > 0) {
    if ((ldrawType = argv[argv.size() - 2] == "LDRAW_TYPE")) {
      // the last item is an ldrawType - specified when substitute is a generated part
      attributes = argv[argv.size() - 1]+":1;";
      // recalculate argc
      argc = argv.size() - (index + 2);
    }
    if ((ldrawType && argc == 1) || !ldrawType) {
      // lets try to get the original type
      bool validLine  = false;
      bool subEnd     = false;
      bool newStep    = false;
      bool configured = false;
      if (Preferences::enableFadeSteps || Preferences::enableHighlightStep) {
        QFileInfo modelInfo(here.modelName);
        configured = modelInfo.completeBaseName().endsWith(FADE_SFX) ||
                     modelInfo.completeBaseName().endsWith(HIGHLIGHT_SFX);
      }
      Where lineBelow = here + 1;          // start at 1 line below PLI BEGIN SUB meta
      int numLines = configured ? lpub->ldrawFile.configuredSubFileSize(here.modelName) :
                                  lpub->ldrawFile.size(here.modelName);
      QString originalTypeLine = configured ? lpub->ldrawFile.readConfiguredLine(lineBelow.modelName,lineBelow.lineNumber) :
                                              lpub->ldrawFile.readLine(lineBelow.modelName,lineBelow.lineNumber);
      // read each line looking for type 1 line or end of step
      for (; !validLine && !subEnd && !newStep && lineBelow <= numLines; lineBelow++) {
        validLine = originalTypeLine[0] == '1';
        subEnd    = originalTypeLine.endsWith("PLI END");
        newStep   = originalTypeLine.endsWith("STEP") ||
                    originalTypeLine.contains("ROTSTEP");
        // do we have reason to stop checking?
        if (!validLine && !subEnd && !newStep)
          originalTypeLine =  configured ? lpub->ldrawFile.readConfiguredLine(lineBelow.modelName,lineBelow.lineNumber) :
                                           lpub->ldrawFile.readLine(lineBelow.modelName,lineBelow.lineNumber);
      }

      QStringList tokens;
      split(originalTypeLine,tokens);
      if (tokens.size() == 15 && tokens[0] == "1") {
        if(!ldrawType)
          attributes = tokens[14]+":0;";    // originalType
        originalColor  = tokens[1];
      } else if (Gui::pageProcessRunning != PROC_NONE) {
        here.setModelIndex(lpub->ldrawFile.getSubmodelIndex(here.modelName));
        QString const message = QObject::tr("Invalid substitute meta command.<br>"
                                            "No valid parts between %1 and PLI END.<br>Got %2.")
                                            .arg(argv.join(" "), originalTypeLine);
        emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
      }
    }
  }
  int a = index, b = argv.size();
  if (argc >= 6)
    // a = index (4) + (type & color) = 6
    a = index+2;
  if (argc > 1) {
    // argc = 2, argv = 6
    _value.part  = argv[index];
    _value.color = argv[index+1];
  }
  if (argc == 1) {
    // sub : argv = 5
    _value.part  = argv[index];
    _value.color = originalColor;
    _value.type  = rc = PliBeginSub1Rc;
  } else if (argc == 2) {
    // sub, color : argv = 6
    _value.type  = rc = PliBeginSub2Rc;
  } else if (argc == 3) {
    // sub, color, scale : argv = 7
    _value.part  = argv[index];
    _value.color = argv[index+1];
    argv[index+2].toFloat(&ok[0]);
    if (ok[0])
      _value.type = rc = PliBeginSub3Rc;
  } else if (argc == 4) {
    // sub, color, scale, fov : argv = 8
    argv[index+2].toFloat(&ok[0]);
    argv[index+3].toFloat(&ok[1]);
    ok[0] &= ok[1];
    if (ok[0])
      _value.type = rc = PliBeginSub4Rc;
  } else if (argc == 6) {
    // sub, color, scale, fov, caX, caY : argv = 10
    int j = 0; ok[j] = false;
    for (int i = a; i < b; i++) {
        argv[i].toFloat(&ok[j]);j++; }
    for (int i = 0; i <= j; i++) {
      if (!ok[i]) { ok[0] = false; break; } }
    if (ok[0])
      _value.type = rc = PliBeginSub5Rc;
  } else if (argc == 9) {
    // sub, color, scale, fov, caX, caY, targetX,Y,Z : argv = 13
    int j = 0; ok[j] = false;
    for (int i = a; i < b; i++) {
      argv[i].toFloat(&ok[j]);j++; }
    for (int i = 0; i <= j; i++) {
      if (!ok[i]) { ok[0] = false; break; } }
    if (ok[0])
      _value.type = rc = PliBeginSub6Rc;
  } else if (argc == 10) {
    // sub, color, scale, fov, caX, caY, rotstepX,Y,Z,Transform : argv = 14
    int j = 0; --b/*exclude Transform*/; ok[j] = false;
    for (int i = a; i < b; i++) {
      argv[i].toFloat(&ok[j]);j++; }
    for (int i = 0; i <= j; i++) {
      if (!ok[i]) { ok[0] = false; break; } }
    if (ok[0] && argv[b].contains(rx))
      _value.type = rc = PliBeginSub7Rc;
  } else if (argc == 13) {
    // sub, color, scale, fov, caX, caY, targetX,Y,Z, rotstepX,Y,Z,Transform : argv = 17
    int j = 0; --b/*exclude Transform*/; ok[j] = false;
    for (int i = a; i < b; i++) {
      argv[i].toFloat(&ok[j]);j++; }
    for (int i = 0; i <= j; i++) {
      if (!ok[i]) { ok[0] = false; break; } }
    if (ok[0] && argv[b].contains(rx))
      _value.type = rc = PliBeginSub8Rc;     // target and rotstep
  }
  if (rc != FailureRc) {
    // add attributes - advance past part and color +2
    for (int i = a; i < argv.size(); i++)
      attributes.append(argv.at(i)+";");
    // append line number to end of attributes - used by Pli::partLine()
    attributes.append(QString::number(here.lineNumber));
    _value.attrs     = attributes;
    // indicate if we have an ldrawType - used by format
    _value.ldrawType = ldrawType;
    _here[0] = here;
    _here[1] = here;
  }
  return rc;
}

QString SubMeta::format(bool local, bool global)
{
  // This routine is acutally not used.
  // Substitute commands are formatted by MetaItem::substitutePLIPart
  QStringList _attributeList = _value.attrs.split(";");

  QString foo;

  if (_value.type == PliBeginSub1Rc) {
    foo = QString("%1")
            .arg(_value.part);
  } else if (_value.type == PliBeginSub2Rc) {
    foo = QString("%1 %2")
            .arg(_value.part, _value.color);
  } else if (_value.type == PliBeginSub3Rc) {
    foo = QString("%1 %2 %3")
            .arg(_value.part, _value.color)
            .arg(_attributeList[sModelScale]);
  } else if (_value.type == PliBeginSub4Rc) {
    foo = QString("%1 %2 %3 %4")
            .arg(_value.part, _value.color)
            .arg(_attributeList[sModelScale])
            .arg(_attributeList[sCameraFoV]);
  } else if (_value.type == PliBeginSub5Rc) {
    foo = QString("%1 %2 %3 %4 %5 %6")
            .arg(_value.part, _value.color)
            .arg(_attributeList[sModelScale])
            .arg(_attributeList[sCameraFoV])
            .arg(_attributeList[sCameraAngleXX])
            .arg(_attributeList[sCameraAngleYY]);
  } else if (_value.type == PliBeginSub6Rc) {
    foo = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9")
            .arg(_value.part, _value.color)
            .arg(_attributeList[sModelScale])
            .arg(_attributeList[sCameraFoV])
            .arg(_attributeList[sCameraAngleXX])
            .arg(_attributeList[sCameraAngleYY])
            .arg(_attributeList[sTargetX])
            .arg(_attributeList[sTargetY])
            .arg(_attributeList[sTargetZ]);
  } else if (_value.type == PliBeginSub7Rc) {
      foo = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
            .arg(_value.part, _value.color)
            .arg(_attributeList[sModelScale])
            .arg(_attributeList[sCameraFoV])
            .arg(_attributeList[sCameraAngleXX])
            .arg(_attributeList[sCameraAngleYY])
            .arg(_attributeList[sRotX])
            .arg(_attributeList[sRotY])
            .arg(_attributeList[sRotZ])
            .arg(_attributeList[sTransform]);
  } else { /*PliBeginSub8Rc */
    foo = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13")
            .arg(_value.part, _value.color)
            .arg(_attributeList[sModelScale])
            .arg(_attributeList[sCameraFoV])
            .arg(_attributeList[sCameraAngleXX])
            .arg(_attributeList[sCameraAngleYY])
            .arg(_attributeList[sTargetX])
            .arg(_attributeList[sTargetY])
            .arg(_attributeList[sTargetZ])
            .arg(_attributeList[sRotX])
            .arg(_attributeList[sRotY])
            .arg(_attributeList[sRotZ])
            .arg(_attributeList[sTransform]);
  }

  if (_value.ldrawType) {
      foo += QString(" LDRAW_TYPE %1").arg(_attributeList.first());
  }

  return LeafMeta::format(local,global,foo);
}

void SubMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <LDraw part name> [ <LDraw colour code> <scale decimal> <fov decimal> <camera angle latitude decimal> <camera angle longitude decimal> "
                    "<target X> <target Y> <target Z> <rotate X> <rotate Y> <rotate Z> ( ABS | REL | ADD ) [ LDRAW_TYPE <LDraw part name> ]]";
}

void SubMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " ABS REL ADD LDRAW_TYPE";
}
/* ------------------ */

void PartBeginMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  ignore.init(this, "IGN",   PartBeginIgnRc);
}

/* ------------------ */

void PartIgnMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  begin.init(this, "BEGIN");
  end  .init(this, "END", PartEndRc);
}

/* ------------------ */

Rc RotStepMeta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() >= index+3 ) {
    bool ok[3];
    double rotX = argv[index+0].toDouble(&ok[0]);
    double rotY = argv[index+1].toDouble(&ok[1]);
    double rotZ = argv[index+2].toDouble(&ok[2]);
    if (ok[0] && ok[1] && ok[2]) {
      _value.rots[0] = rotX;
      _value.rots[1] = rotY;
      _value.rots[2] = rotZ;
      static QRegularExpression rx("^(ABS|REL|ADD)$");
      if (argv.size() == index+4 && argv[index+3].contains(rx))
         _value.type   = argv[index+3];
      _value.populated = !(_value.rots[0] == 0 && _value.rots[1] == 0 && _value.rots[2] == 0);
      _here[0] = here;
      _here[1] = here;
      return index == 1 ? RotStepRc : OkRc;
    }
  } else if (argv.size() - index == 1 && argv[index] == "END") {
    _value.type.clear();
    _value.rots[0] = 0;
    _value.rots[1] = 0;
    _value.rots[2] = 0;
    _value.populated = false;
    _here[0] = here;
    _here[1] = here;

    return RotStepRc;
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Malformed ROTSETP meta \"%1\"") .arg(argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}
QString RotStepMeta::format(bool local, bool global)
{
  QString foo = QString("%1 %2 %3 %4")
      .arg(_value.rots[0]) .arg(_value.rots[1])
      .arg(_value.rots[2]) .arg(_value.type);
  return LeafMeta::format(local,global,foo);
}

void RotStepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <rotate X> <rotate Y> <rotate Z> ( ABS | REL | ADD )";
}

void RotStepMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " ABS REL ADD";
}

/* ------------------ */
Rc BuffExchgMeta::parse(QStringList &argv, int index,Where &here)
{
  if (index + 2 == argv.size()) {
    static QRegularExpression b("^[A-Z]$");
    static QRegularExpression t("^(STORE|RETRIEVE)$");
    if (argv[index].contains(b) && argv[index+1].contains(t)) {
      _value.buffer = argv[index];
      _here[0] = here;
      _here[1] = here;
      if (argv[index+1] == "RETRIEVE") {
        return BufferLoadRc;
      } else {
        return BufferStoreRc;
      }
    }
  }
  if (reportErrors) {
    QString const message = QMessageBox::tr("Malformed buffer exchange \"%1\"") .arg(argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
  }
  return FailureRc;
}
QString BuffExchgMeta::format(bool local, bool global)
{
  QString foo = _value.buffer + " " + _value.type;
  return LeafMeta::format(local,global,foo);
}

void BuffExchgMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <bufferName> ( STORE | RETRIEVE )";
}

void BuffExchgMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " STORE RETRIEVE";
}

/* ------------------ */

Rc BuildModMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc = FailureRc;
  QString missingMeta;
  static QRegularExpression actionRx("^(BEGIN|END_MOD|END|APPLY|REMOVE)$");
  if (index + 1 == argv.size()) {
    if (argv[index].contains(actionRx)) {
      if (argv[index] == "BEGIN")
        missingMeta = argv[index];
      else if (argv[index] == "END_MOD")
        rc = BuildModEndModRc;
      else if (argv[index] == "END")
        rc = BuildModEndRc;
      else if (argv[index] == "APPLY")
        missingMeta = argv[index];
      else if (argv[index] == "REMOVE")
        missingMeta = argv[index];
      _value.action = static_cast<int>(rc);
      _here[0] = here;
      _here[1] = here;
    }
  } else if (index + 2 == argv.size()) {
    static QRegularExpression keyRx("^.*$", QRegularExpression::CaseInsensitiveOption);
    if (argv[index].contains(actionRx) && argv[index + 1].contains(keyRx)) {
      if (argv[index] == "BEGIN")
        rc = BuildModBeginRc;
      else if (argv[index] == "END_MOD")
        rc = BuildModEndModRc;
      else if (argv[index] == "END")
        rc = BuildModEndRc;
      else if (argv[index] == "APPLY")
        rc = BuildModApplyRc;
      else if (argv[index] == "REMOVE")
        rc = BuildModRemoveRc;
      _value.action      = static_cast<int>(rc);
      _value.buildModKey = argv[index + 1];
      _here[0] = here;
      _here[1] = here;
    }
  }

  if (rc == FailureRc) {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Malformed build mod meta command \"%1\"") .arg(argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
  } else if (!missingMeta.isEmpty()) {
    QString const message = QMessageBox::tr("Build mod meta %1 requires a key value. Got \"%2\"") .arg(missingMeta, argv.join(" "));
    emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    rc = FailureRc;
  }

  return rc;
}

QString BuildModMeta::format(bool local, bool global)
{
  QString foo;
  switch(_value.action) {
  case BuildModBeginRc:
      foo = QLatin1String("BEGIN");
      break;
  case BuildModEndModRc:
      foo = QLatin1String("END_MOD");
      break;
  case BuildModEndRc:
      foo = QLatin1String("END");
      break;
  case BuildModApplyRc:
      foo = QLatin1String("APPLY");
      break;
  case BuildModRemoveRc:
      foo = QLatin1String("REMOVE");
      break;
  default:
      break;
  }

  if (!_value.buildModKey.isEmpty())
    foo += QString(" \"%1\"").arg(_value.buildModKey);

  return LeafMeta::format(local,global,foo);
}

void BuildModMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (( BEGIN | APPLY | REMOVE ) <buildModKey> ) | ( END_MOD | END )";
}

void BuildModMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " BEGIN APPLY REMOVE END_MOD END";
}
/* ------------------ */

AnnotationStyleMeta::AnnotationStyleMeta() : BranchMeta()
{
  margin.setValuesInches(0.03f,0.03f);      //4px @ 150DPI
  BorderData borderData;
  borderData.type = BorderData::BdrNone;
  borderData.line = BorderData::BdrLnNone;
  borderData.color = "#ffffff";
  borderData.thickness = 1.0f/64.0f;
  borderData.radius = 10;
  borderData.margin[0] = 0.0f;
  borderData.margin[1] = 0.0f;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgTransparent);
  font.setValuePoints("Arial,20,-1,5,50,0,0,0,0,0");
  color.setValue("#3a3938");
  size.setValuesInches(STYLE_SIZE_DEFAULT,STYLE_SIZE_DEFAULT);      //42px @ 150DPI
  size.setRange(0.1f,1.0f);
  size.setFormats(6,4,"9.9999");
  style.setValue(AnnotationStyle::none);
}

void AnnotationStyleMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin     .init(this,"MARGINS");
  border     .init(this,"BORDER");
  background .init(this,"BACKGROUND");
  font       .init(this,"FONT",OkRc, "\"");
  color      .init(this,"FONT_COLOR");
  size       .init(this,"SIZE");
  style      .init(this,"STYLE");
}

/* ------------------ */

Rc PliPartGroupMeta::parse(QStringList &argv, int index, Where &here)
{
  PliPartGroupData gd;
  Rc rc = OkRc;
  bool bomItem = false;

  if (argv.size() - index > 1 &&
    argv[index] == "ITEM") {
    if ((bomItem = argv[index - 2] == "BOM"))
      gd.bom = true;
    gd.type  = argv[++index];
    gd.color = argv[++index];
  }

  if (rc == OkRc) {
    if (argv.size() - index == 4 && argv[++index] == "OFFSET") {
      bool ok[2];
      gd.offset[0] = argv[++index].toDouble(&ok[0]);
      gd.offset[1] = argv[++index].toDouble(&ok[1]);
      if ( ! ok[0] || ! ok[1]) {
        rc = FailureRc;
      }
    } else if (argv.size() - index > 0) {
      rc = FailureRc;
    }
  }

  if (rc == OkRc) {
    gd.group.modelName  = here.modelName;
    gd.group.lineNumber = here.lineNumber;
    _value   = gd;
    _here[0] = here;

/* DEBUG - COMMENT TO ENABLE
#ifdef QT_DEBUG_MODE
    qDebug() << "\n"
             << "04 PLI PART GROUP ATTRIBUTES [" + _value.type + "_" + _value.color + "] - PARSE"
             << "\n0. BOM:        " <<(gd.bom ? "True" : "False")
             << "\n0. Bom Part:   " <<(gd.bom ? gd.bPart ? "Yes" : "No" : "N/A")
             << "\n1. Type:       " << gd.type
             << "\n2. Color:      " << gd.color
             << "\n3. ZValue:     " << gd.zValue
             << "\n4. OffsetX:    " << gd.offset[0]
             << "\n5. OffsetY:    " << gd.offset[1]
             << "\n6. Group Model:" << gd.group.modelName
             << "\n7. Group Line: " << gd.group.lineNumber
             << "\n8. Meta Model: " << _here[0].modelName
             << "\n9. Meta Line:  " << _here[0].lineNumber
             ;
#endif
//*/
    if (bomItem)
      return BomPartGroupRc;
    else
      return PliPartGroupRc;
  } else {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Malformed PLI Group metacommand \"%1\"\n").arg(argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
    return FailureRc;
  }
}


QString PliPartGroupMeta::format(bool local, bool global)
{
  QString foo;
  foo += " ITEM \"" + _value.type + "\" \"" + _value.color + "\"";

  if (_value.offset[0] != 0.0 || _value.offset[1] != 0.0) {
    foo += QString(" OFFSET %1 %2")
                   .arg(double(_value.offset[0]),0,'f',4)
                   .arg(double(_value.offset[1]),0,'f',4);
  }

  return LeafMeta::format(local,global,foo);
}

void PliPartGroupMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <LDraw part name> <LDraw colour code> OFFSET <decimal X> <decimal Y>";
}

void PliPartGroupMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " OFFSET";
}
/* ------------------ */

PliSortOrderMeta::PliSortOrderMeta() : BranchMeta()
{
  primary.setValue(SortOptionName[PartColour]);
  secondary.setValue(SortOptionName[PartCategory]);
  tertiary.setValue(SortOptionName[PartSize]);
}

void PliSortOrderMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  primary.init(this, "PRIMARY");
  secondary.init(this, "SECONDARY");
  tertiary.init(this, "TERTIARY");
  primaryDirection.init(this, "PRIMARY_DIRECTION");
  secondaryDirection.init(this, "SECONDARY_DIRECTION");
  tertiaryDirection.init(this, "TERTIARY_DIRECTION");
}

PliSortMeta::PliSortMeta() : BranchMeta()
{
  sortOption.setValue("Part Size");
}

void PliSortMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  sortOption.init(this, "SORT_OPTION");
}

/* ------------------ */

PliPartElementMeta::PliPartElementMeta() : BranchMeta()
{
  display.setValue               (false);
  legoElements.setValue          (true);
  bricklinkElements.setValue     (false);
  userElements.setValue          (false);   // default is to use BrickLink's LEGO Elements
  userElementsLDrawKey.setValue  (true);
  localElements.setValue         (false);   // kept localElements for backwards compatability
}

void PliPartElementMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  display.init(this, "DISPLAY");
  legoElements.init(this, "LEGO");
  bricklinkElements.init(this, "BRICKLINK");
  userElements.init(this, "USER_ELEMENTS_FILE");
  userElementsLDrawKey.init(this, "USER_ELEMENTS_USE_LDRAW_KEY");
  localElements.init(this, "LOCAL_LEGO_ELEMENTS_FILE"); // kept localElements for backwards compatability
}

/* ------------------ */

PliAnnotationMeta::PliAnnotationMeta() : BranchMeta()
{
  titleAnnotation.setValue           (true);
  freeformAnnotation.setValue        (false);
  titleAndFreeformAnnotation.setValue(false);
  fixedAnnotations.setValue          (true);
  display.setValue                   (false);
  enableStyle.setValue               (true);
  axleStyle.setValue                 (true);
  beamStyle.setValue                 (true);
  cableStyle.setValue                (true);
  connectorStyle.setValue            (true);
  elementStyle.setValue              (true);
  extendedStyle.setValue             (false);
  hoseStyle.setValue                 (true);
  panelStyle.setValue                (true);
}

void PliAnnotationMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  titleAnnotation.init(this, "USE_TITLE");
  freeformAnnotation.init(this, "USE_FREE_FORM");
  titleAndFreeformAnnotation.init(this, "USE_TITLE_AND_FREE_FORM");
  fixedAnnotations.init(this, "FIXED_ANNOTATIONS");
  display.init(this, "DISPLAY");
  enableStyle.init(this, "ENABLE_STYLE");
  axleStyle.init(this, "AXLE");
  beamStyle.init(this, "BEAM");
  cableStyle.init(this, "CABLE");
  connectorStyle.init(this, "CONNECTOR");
  elementStyle.init(this, "ELEMENT");
  extendedStyle.init(this, "EXTENDED");
  hoseStyle.init(this, "HOSE");
  panelStyle.init(this, "PANEL");
}

/* ------------------ */

CsiAnnotationMeta::CsiAnnotationMeta() : BranchMeta()
{
  placement.setValue                (BottomLeftOutsideCorner,CsiPartType);
  display.setValue                  (false);
  axleDisplay.setValue              (true);
  beamDisplay.setValue              (true);
  cableDisplay.setValue             (true);
  connectorDisplay.setValue         (true);
  extendedDisplay.setValue          (false);
  hoseDisplay.setValue              (true);
  panelDisplay.setValue             (true);
}

void CsiAnnotationMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  display.init(this, "DISPLAY");
  axleDisplay.init(this, "AXLE");
  beamDisplay.init(this, "BEAM");
  cableDisplay.init(this, "CABLE");
  connectorDisplay.init(this, "CONNECTOR");
  extendedDisplay.init(this, "EXTENDED");
  hoseDisplay.init(this, "HOSE");
  panelDisplay.init(this, "PANEL");
  icon.init(this, "ICON");
}

/* ------------------ */

CsiPartMeta::CsiPartMeta() : BranchMeta()
{
  placement.setValue(CenterCenter,CsiType);
  margin.setValuesInches(0.0f,0.0f);
  loc.setValuesInches(0.0f,0.0f);
  loc.setRange(0.0f,10000.0f);
  loc.setFormats(6,4,"9.9999");
  size.setValuesInches(0.0f,0.0f);
  size.setRange(0.0f,1000.0f);
  size.setFormats(6,4,"9.9999");
}

void CsiPartMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  margin.init(this, "MARGINS");
  loc.init(this, "LOC");
  size.init(this, "SIZE");
}

/* ------------------ */

LoadUnoffPartsMeta::LoadUnoffPartsMeta() : BranchMeta()
{
  enableSetting.setValue(LDrawFile::_hasUnofficialParts);
  enabled.setValue      (LDrawFile::_loadUnofficialParts);
}

void LoadUnoffPartsMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  enableSetting.init(this, "ENABLE_SETTING");
  enabled.init(this, "ENABLED");
}

/* ------------------ */

SubModelMeta::SubModelMeta() : PliMeta()
{
  showStepNum.setRange(1,100000);
  placement.setValue(RightTopOutside,StepNumberType);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_LINE_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgColor,"#ffffff");
  margin.setValuesInches(0.0f,0.0f);
  // instance - default
  // annotate - default
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"#99999.9");
  modelScale.setValue(.50);
  show.setValue(Preferences::showSubmodels);
  showTopModel.setValue(Preferences::showTopModel);
  showSubmodelInCallout.setValue(Preferences::showSubmodelInCallout);
  showInstanceCount.setValue(Preferences::showInstanceCount);
  ldgliteParms.setValue("-l3");
  ldviewParms.setValue("");
  povrayParms.setValue("+A");
  // includeSubs.setValue(false);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_01);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_02);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_03);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_04);
  part.margin.setValuesInches(0.05f,0.03f);
  RotStepData rotStepData;
  rotStepData.rots[0] = 0.0;
  rotStepData.rots[1] = 0.0;
  rotStepData.rots[2] = 0.0;
  rotStepData.type = "REL";
  rotStep.setValue(rotStepData);
  margin.setValuesInches(DEFAULT_MARGIN,DEFAULT_MARGIN);
  pack.setValue(true);
  cameraAngles.setFormats(6,1,"###9.0");
  cameraAngles.setRange(-360.0f,360.0f);
  cameraAngles.setValues(MetaDefaults::getSubmodelCameraLatitude(),MetaDefaults::getSubmodelCameraLongitude());
  cameraAngles.setType(static_cast<int>(Options::SMI));
  cameraDistance.setRange(1.0f,FLT_MAX);
  cameraFoV.setFormats(5,4,"9.999");
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(),
                     MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setFormats(5,0,"####9");
  cameraZNear.setRange(1.0f,FLT_MAX);
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setFormats(5,0,"####9");
  cameraZFar.setRange(1.0f,FLT_MAX);
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
  isOrtho.setValue(false);

  // movable pli part groups
  enablePliPartGroup.setValue(false);
}

void SubModelMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  showStepNum          .init(this,"SHOW_STEP_NUM");
  placement            .init(this,"PLACEMENT");
  constrain            .init(this,"CONSTRAIN");
  border               .init(this,"BORDER");
  background           .init(this,"BACKGROUND");
  margin               .init(this,"MARGINS");
  instance             .init(this,"INSTANCE_COUNT");
  includeSubs          .init(this,"INCLUDE_SUBMODELS");
  modelScale           .init(this,"MODEL_SCALE");
  show                 .init(this,"SHOW");
  showTopModel         .init(this,"SHOW_TOP_MODEL");
  showSubmodelInCallout.init(this,"SHOW_SUBMODEL_IN_CALLOUT");
  studStyle            .init(this,"STUD_STYLE");
  highContrast         .init(this,"HIGH_CONTRAST");
  autoEdgeColor        .init(this,"AUTOMATE_EDGE_COLOR");
  showInstanceCount    .init(this,"SHOW_INSTANCE_COUNT");
  ldviewParms          .init(this,"LDVIEW_PARMS");
  ldviewEnvVars        .init(this,"LDVIEW_ENV_VARS");
  ldgliteParms         .init(this,"LDGLITE_PARMS");
  ldgliteEnvVars       .init(this,"LDGLITE_ENV_VARS");
  povrayParms          .init(this,"POVRAY_PARMS");
  povrayEnvVars        .init(this,"POVRAY_ENV_VARS");
  subModelColor        .init(this,"SUBMODEL_BACKGROUND_COLOR");
  part                 .init(this,"PART");
  rotStep              .init(this,"SUBMODEL_ROTATION");
  cameraFoV            .init(this,"CAMERA_FOV");
  cameraZNear          .init(this,"CAMERA_ZNEAR");
  cameraZFar           .init(this,"CAMERA_ZFAR");
  cameraAngles         .init(this,"CAMERA_ANGLES");
  cameraDistance       .init(this,"CAMERA_DISTANCE");
  target               .init(this,"CAMERA_TARGET");
  position             .init(this,"CAMERA_POSITION");
  upvector             .init(this,"CAMERA_UPVECTOR");
  preferredRenderer    .init(this,"PREFERRED_RENDERER");
}

/* ------------------ */

RotateIconMeta::RotateIconMeta() : BranchMeta()
{
  placement.setValue(RightOutside,CsiType);   // right outside single step
  BorderData borderData;
  borderData.type = BorderData::BdrRound;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_LINE_THICKNESS;
  borderData.radius = 10;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgTransparent);
  margin.setValuesInches(0.0f,0.0f);
  BorderData arrowData;
  arrowData.type = BorderData::BdrSquare;
  arrowData.line = BorderData::BdrLnSolid;
  arrowData.color = "Blue";
  arrowData.thickness = DEFAULT_LINE_THICKNESS;
  arrowData.margin[0] = DEFAULT_MARGIN;
  arrowData.margin[1] = DEFAULT_MARGIN;
  arrow.setValueInches(arrowData);
  display.setValue(true);
  size.setValuesInches(0.52f,0.52f);
  size.setRange(0.1f,3.0f);
  size.setFormats(6,4,"9.9999");
  picScale.setRange(-10000.0,10000.0);
  picScale.setFormats(7,4,"#99999.9");
  picScale.setValue(1.0);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_01);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_02);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_03);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_04);
}

void RotateIconMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  size            .init(this,"SIZE");
  arrow           .init(this,"ARROW");
  placement       .init(this,"PLACEMENT");
  border          .init(this,"BORDER");
  background      .init(this,"BACKGROUND");
  margin          .init(this,"MARGINS");
  display         .init(this,"DISPLAY");
  picScale        .init(this,"SCALE");
}

/*---------------------------------------------------------------
 * The Top Level LPub Metas
 *---------------------------------------------------------------*/

PageMeta::PageMeta() : BranchMeta()
{
  size.setValuesInches(8.2677f,11.6929f);
  size.setRange(1,1000);
  size.setFormats(6,4,"9.9999");
  size.setValueSizeID("A4");
  orientation.setValue(Portrait);

  BorderData borderData;
  borderData.type = BorderData::BdrNone;
  borderData.line = BorderData::BdrLnNone;
  borderData.color = "Black";
  borderData.thickness = 0;
  borderData.radius = 0;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);

  background.setValue(BackgroundData::BgSubmodelColor);
  dpn.setValue(true);
  togglePnPlacement.setValue(false);
  number.placement.setValue(BottomRightInsideCorner,PageType);
  number.color.setValue("black");
  number.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  instanceCount.placement.setValue(TopLeftOutsideCorner,PageNumberType); //fooWas LeftBottomOutside,PageNumberType
  instanceCount.color.setValue("black");
  instanceCount.font.setValuePoints("Arial,48,-1,255,75,0,0,0,0,0");

  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_01);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_02);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_03);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_04);

  countInstanceOverride.setRange(0,10000);
  countInstanceOverride.setValue(0);

  //text placement
  textPlacement.setValue(false);
  textPlacementMeta.setValue(CenterCenter,PageType);

  /* PAGE ATTRIBUTE FORMAT
   *
   * Front Cover Default Attribute Placements
   *************************************
   *   Logo                            *  (Bottom of Header) [Independent]
   *                                   *
   *   ModelName                       *  (Top Left of Title) [Dependent]
   *   Title                           *  (Left of Page) [Anchor]
   *   Author                          *  (Bottom Left of Title) [Dependent]
   *   Parts                           *  (Bottom Left of Author) [Dependent]
   *   Model Description               *  (Bottom Left of Parts) [Dependent]
   *   Publsiher Description           *  (Bottom Left of Model Description) [Dependent]
   *
   *   Cover Image                     *  (Center of page) [Independent]
   *                                   *
   *************************************


   * Header/Footer Default Attribute Placements
   * ***********************************
   * URL (Top Left of Page)            Email (Top Right of Page)
   * ***********************************
   * *                                 *
   * *                                 *
   * *                                 *
   * *                                 *
   * ***********************************
   * Copyright (Bottom Left of Page)   Author (Left Bottom of PageNumber)
   * ***********************************


   * Back Cover Default Attribute Placements
   * **********************************
   *  Logo                            *  (Bottom of Header) [Independent]
   *                                  *
   *  Title                           *  (Center of Page) [Anchor]
   *  Author                          *  (Bottom of Title) [Dependent]
   *  Copyright                       *  (Bottom of Author) [Dependent]
   *  URL                             *  (Bottom of Copyright) [Dependent]
   *  Email                           *  (Bottom of URL) [Dependent]
   *  LEGO Disclaimer                 *  (Bottom of Email) [Dependent]
   *  LPub3D Plug                     *  (Bottom of LEGO Disclaimer) [Dependent]
   *  LPub3D Plug Image               *  (Bottom of Plug) [Dependent]
   *                                  *
   ************************************

   * Page Attribute relative Tos
   *
   *  FRONT COVER PAGE
   *  documentLogoFront.placement  (BottomOutside,      PageHeaderType)      [LineEdit]
   *  modelName.placement          (TopLeftOutside,     PageTitleType)       [LineEdit]
   *  titleFront.placement         (LeftInside,         PageType)            [LineEdit]
   *  authorFront.placement        (BottomLeftOutside,  PageTitleType)       [LineEdit]
   *  Parts.placement              (BottomLeftOutside,  PageAuthorType)      [LineEdit]
   *  modelDesc.placement          (BottomLeftOutside,  PageModelDescType)   [TextEdit]
   *  publishDesc.placement        (BottomLeftOutside,  PagePublishDescType) [TextEdit]
   *
   *  BACK COVER PAGE
   *  documentLogoBack.placement   (BottomOutside,      PageHeaderType)      [LineEdit]
   *  titleBack.placement          (CenterCenter,       PageType)            [LineEdit]
   *  authorBack.placement         (BottomOutside,      PageTitleType)       [LineEdit]
   *  copyrightBack.placement      (BottomOutside,      PageAuthorType)      [LineEdit]
   *  urlBack.placement            (BottomOutside,      PageCopyrightType)   [LineEdit]
   *  emailBack.placement          (BottomOutside,      PageURLType)         [LineEdit]
   *  disclaimer.placement         (BottomOutside,      PageEmailType)       [LineEdit]
   *  plug.placement               (BottomOutside,      PageDisclaimerType)  [TextEdit]
   *  plugImage.placement          (BottomOutside,      PagePlugType)        [TextEdit]
   *
   *  HEADER
   *  url.placement                (TopLeftInsideCorner, PageType)           [LineEdit]
   *  email.placement              (TopRightInsideCorner,PageType)           [LineEdit]
   *
   *  FOOTER
   *  copyright.placement      (BottomLeftInsideCorner,  PageType)           [LineEdit]
   *  author.placement         (LeftBottomOutside,       PageNumberType)     [LineEdit]
   *
   *  NOT PLACED
   *  category                     (TopLeftInsideCorner, PageType)
   */

  // FRONT COVER PAGE

  //publisher documentLogoFront IMAGE
  documentLogoFront.placement.setValue(BottomOutside,PageHeaderType);
  documentLogoFront.type = PageDocumentLogoType;
  documentLogoFront.file.setValue(Preferences::documentLogoFile);

  //model id/name text
  modelName.placement.setValue(TopLeftOutside,PageTitleType);
  modelName.type = PageModelNameType;
  modelName.textFont.setValuePoints("Arial,20,-1,255,75,0,0,0,0,0");
  modelName.setValue(LDrawFile::_name);

  //model titleFront text
  titleFront.placement.setValue(LeftInside,PageType);
  titleFront.type = PageTitleType;
  titleFront.textFont.setValuePoints("Arial,32,-1,255,75,0,0,0,0,0");
  titleFront.setValue(LDrawFile::_file);

  //publisher authorFront text
  authorFront.placement.setValue(BottomLeftOutside,PageTitleType);
  authorFront.type = PageAuthorType;
  authorFront.textFont.setValuePoints("Arial,20,-1,255,75,0,0,0,0,0");
  authorFront.setValue(QObject::tr("Model instructions by %1").arg(Preferences::defaultAuthor));

  //model number of Parts text
  parts.placement.setValue(BottomLeftOutside,PageAuthorType);
  parts.type = PagePartsType;
  parts.textFont.setValuePoints("Arial,20,-1,255,75,0,0,0,0,0");
  parts.setValue(QString("%1 %2").arg(QString::number(LDrawFile::_partCount), LDrawFile::_partCount == 1 ? QObject::tr("Part") : QObject::tr("Parts")));

  //model description text
  modelDesc.placement.setValue(BottomLeftOutside,PagePartsType);
  modelDesc.type = PageModelDescType;
  modelDesc.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  modelDesc.setValue(Preferences::publishDescription);

  //publisher description text
  publishDesc.placement.setValue(BottomLeftOutside,PageModelDescType);
  publishDesc.type = PagePublishDescType;
  publishDesc.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  publishDesc.setValue(Preferences::publishDescription);

  //publisher cover IMAGE
  coverImage.placement.setValue(CenterCenter,PageType);
  coverImage.type = PageCoverImageType;

  // CONTENT (HEADER/FOOTER) PAGES

  //publisher url text
  url.placement.setValue(TopLeftInsideCorner,PageType);
  url.type = PageURLType;
  url.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  url.setValue(Preferences::defaultURL);

  //publisher email text
  email.placement.setValue(TopRightInsideCorner,PageType);
  email.type = PageEmailType;
  email.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  email.setValue(Preferences::defaultEmail);

  //publisher copyright text
  copyright.placement.setValue(BottomLeftInsideCorner,PageType);
  copyright.type = PageCopyrightType;
  copyright.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  copyright.setValue(QObject::tr("%1 by %2").arg(Preferences::copyright, Preferences::defaultAuthor));

  //publisher author text
  author.placement.setValue(LeftBottomOutside,PageNumberType);
  author.type = PageAuthorType;
  author.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  author.setValue(Preferences::defaultAuthor);

  // BACK COVER PAGE

  //publisher documentLogoBack IMAGE
  documentLogoBack.placement.setValue(BottomOutside,PageHeaderType);
  documentLogoBack.picScale.setValue(0.5);
  documentLogoBack.type = PageDocumentLogoType;
  documentLogoBack.file.setValue(Preferences::documentLogoFile);

  //model titleBack text
  titleBack.placement.setValue(CenterCenter,PageType);
  titleBack.type = PageTitleType;
  titleBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  titleBack.setValue(LDrawFile::_file);

  //publisher authorBack text
  authorBack.placement.setValue(BottomOutside,PageTitleType);
  authorBack.type = PageAuthorType;
  authorBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  authorBack.setValue(QObject::tr("Model instructions by %1").arg(Preferences::defaultAuthor));

  //publisher copyrightBack text
  copyrightBack.placement.setValue(BottomOutside,PageAuthorType);
  copyrightBack.type = PageCopyrightType;
  copyrightBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  copyrightBack.setValue(QObject::tr("%1 by %2").arg(Preferences::copyright, Preferences::defaultAuthor));

  //publisher urlBack text
  urlBack.placement.setValue(BottomOutside,PageCopyrightType);
  urlBack.type = PageURLType;
  urlBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  urlBack.setValue(Preferences::defaultURL);

  //publisher emailBack text
  emailBack.placement.setValue(BottomOutside,PageURLType);
  emailBack.type = PageEmailType;
  emailBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  emailBack.setValue(Preferences::defaultEmail);

  //disclaimer text
  disclaimer.placement.setValue(BottomOutside,PageEmailType);
  disclaimer.type = PageDisclaimerType;
  disclaimer.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  disclaimer.setValue(Preferences::disclaimer);

  //plug text
  plug.placement.setValue(BottomOutside,PageDisclaimerType);
  plug.type = PagePlugType;
  plug.textFont.setValuePoints("Arial,16,-1,255,75,0,0,0,0,0");
  plug.setValue(Preferences::plug);

  //plug IMAGE
  plugImage.placement.setValue(BottomOutside,PagePlugType);
  plugImage.type = PagePlugImageType;
  plugImage.file.setValue(Preferences::plugImage);

  //model category text (NOT USED)
  category.placement.setValue(BottomLeftOutside,PageType);
  category.type = PageCategoryType;
  category.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  category.setValue(LDrawFile::_category);
}

void PageMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  size.init(this, "SIZE", PageSizeRc);
  orientation.init(this, "ORIENTATION");
  margin.init(this, "MARGINS");
  border.init(this, "BORDER");
  background.init(this, "BACKGROUND");
  dpn.init(this, "DISPLAY_PAGE_NUMBER");
  togglePnPlacement.init(this, "TOGGLE_PAGE_NUMBER_PLACEMENT");
  number.init(this, "NUMBER");
  instanceCount.init(this, "SUBMODEL_INSTANCE_COUNT");
  subModelColor.init(this, "SUBMODEL_BACKGROUND_COLOR");
  pointer.init(this, "POINTER");
  pointerAttrib.init(this, "POINTER_ATTRIBUTE");
  textPlacement.init(this, "TEXT_PLACEMENT");
  textPlacementMeta.init(this, "ENABLE_TEXT_PLACEMENT");

  scene.init(this, "SCENE");
  countInstanceOverride.init(this,"SUBMODEL_INSTANCE_COUNT_OVERRIDE");

  pageHeader.init(this, "PAGE_HEADER");
  pageFooter.init(this, "PAGE_FOOTER");

  titleFront.init(this, "DOCUMENT_TITLE_FRONT");
  titleBack.init(this, "DOCUMENT_TITLE_BACK");
  modelName.init(this, "MODEL_ID");
  modelDesc.init(this, "MODEL_DESCRIPTION");
  parts.init(this, "MODEL_PARTS");
  authorFront.init(this, "DOCUMENT_AUTHOR_FRONT");
  authorBack.init(this, "DOCUMENT_AUTHOR_BACK");
  author.init(this, "DOCUMENT_AUTHOR");
  publishDesc.init(this, "PUBLISH_DESCRIPTION");
  url.init(this, "PUBLISH_URL");
  urlBack.init(this, "PUBLISH_URL_BACK");
  email.init(this, "PUBLISH_EMAIL");
  emailBack.init(this, "PUBLISH_EMAIL_BACK");
  copyrightBack.init(this, "PUBLISH_COPYRIGHT_BACK");
  copyright.init(this, "PUBLISH_COPYRIGHT");
  documentLogoFront.init(this, "DOCUMENT_LOGO_FRONT");
  documentLogoBack.init(this, "DOCUMENT_LOGO_BACK");
  coverImage.init(this, "DOCUMENT_COVER_IMAGE");
  disclaimer.init(this, "LEGO_DISCLAIMER");
  plug.init(this, "APP_PLUG");
  plugImage.init(this, "APP_PLUG_IMAGE");
  category.init(this, "MODEL_CATEGORY" );
}

/* ------------------ */

AssemMeta::AssemMeta() : BranchMeta()
{
  placement.setValue(CenterCenter,PageType);
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"#99999.9");
  modelScale.setValue(1.0);
  ldgliteParms.setValue("-l3");  // change removed -w1 duplicate on 01-25-16 v1.3.3 r578
  ldviewParms.setValue("");
  povrayParms.setValue("+A");    // Deprecated - using Quality Settings, v2.3.7
  showStepNumber.setValue(true);

  // image generation
  cameraAngles.setFormats(6,1,"###9.0");
  cameraAngles.setRange(-360.0f,360.0f);
  cameraAngles.setValues(MetaDefaults::getAssemblyCameraLatitude(),MetaDefaults::getAssemblyCameraLongitude());                   // using LPub3D Default 0.0,0.0f
  cameraAngles.setType(static_cast<int>(Options::CSI));
  cameraDistance.setRange(1.0f,FLT_MAX);
  cameraFoV.setFormats(5,4,"9.999");
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(),
                     MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setFormats(5,0,"####9");
  cameraZNear.setRange(1.0f,FLT_MAX);
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setFormats(5,0,"####9");
  cameraZFar.setRange(1.0f,FLT_MAX);
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
  isOrtho.setValue(false);
  imageSize.setFormats(4,0,"###9");
  imageSize.setRange(0.0f,FLT_MAX);
  imageSize.setValues(0.0f,0.0f);
}

void AssemMeta::resetCameraFoV()
{
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(), MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
}

void AssemMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin.init(this,"MARGINS");
  placement.init(this,"PLACEMENT");
  modelScale.init(this,"MODEL_SCALE");
  ldviewParms.init(this,"LDVIEW_PARMS");
  ldviewEnvVars.init(this,"LDVIEW_ENV_VARS");
  ldgliteParms.init(this,"LDGLITE_PARMS");
  ldgliteEnvVars.init(this,"LDGLITE_ENV_VARS");
  povrayParms .init(this,"POVRAY_PARMS");
  povrayEnvVars .init(this,"POVRAY_ENV_VARS");
  blenderParms .init(this,"BLENDER_PARMS");
  blenderEnvVars .init(this,"BLENDER_ENV_VARS");
  studStyle.init(this,"STUD_STYLE");
  highContrast.init(this,"HIGH_CONTRAST");
  autoEdgeColor.init(this,"AUTOMATE_EDGE_COLOR");
  fadeSteps.init(this,"FADE_STEPS");
  highlightStep.init(this,"HIGHLIGHT_STEP");
  showStepNumber.init(this,"SHOW_STEP_NUMBER");
  annotation.init(this,"ANNOTATION");
  imageSize.init(this,"IMAGE_SIZE");
  cameraFoV.init(this,"CAMERA_FOV");
  cameraZNear.init(this,"CAMERA_ZNEAR");
  cameraZFar.init(this,"CAMERA_ZFAR");
  cameraAngles.init(this,"CAMERA_ANGLES");
  cameraDistance.init(this,"CAMERA_DISTANCE");
  isOrtho.init(this,"CAMERA_ORTHOGRAPHIC");
  cameraName.init(this,"CAMERA_NAME");
  target.init(this,"CAMERA_TARGET");
  position.init(this,"CAMERA_POSITION");
  upvector.init(this,"CAMERA_UPVECTOR");
  preferredRenderer.init(this,"PREFERRED_RENDERER");
}

/* ------------------ */

PliMeta::PliMeta() : BranchMeta()
{
  placement.setValue(RightTopOutside,StepNumberType);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_LINE_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgColor,"#ffffff");
  margin.setValuesInches(0.0f,0.0f);
  // instance - default
  // annotate - default
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"#99999.9");
  modelScale.setValue(1.0);
  show.setValue(true);
  ldgliteParms.setValue("-l3");
  ldviewParms.setValue("");
  povrayParms.setValue("+A");       // Deprecated - using Quality Settings, v2.3.7
  includeSubs.setValue(false);
  individualParts.setValue(false);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_01);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_02);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_03);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_04);
  part.margin.setValuesInches(0.05f,0.03f);
  instance.font.setValuePoints("Arial,36,-1,255,75,0,0,0,0,0");
  instance.margin.setValuesInches(0.0f,0.0f);

  //annotate.font.setValuePoints("Arial,36,-1,255,75,0,0,0,0,0");   // Rem at revision 226 11/06/15
  annotate.font.setValuePoints("Arial,24,-1,5,50,0,0,0,0,0");
  annotate.color.setValue("#3a3938");                               // Add at revision 285 01/07/15
  annotate.margin.setValuesInches(0.0f,0.0f);

  rectangleStyle.border.setValue(BorderData::BdrSquare,BorderData::BdrLnSolid,"#3a3938");
  rectangleStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  rectangleStyle.color.setValue("#34699d"); // #3a3938
  rectangleStyle.style.setValue(AnnotationStyle::rectangle);
  rectangleStyle.size.setValuesInches(STYLE_SIZE_DEFAULT,STYLE_SIZE_DEFAULT);
  rectangleStyle.size.setRange(0.1f,8.0f);

  squareStyle.border.setValue(BorderData::BdrSquare,BorderData::BdrLnSolid,"#3a3938");
  squareStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  squareStyle.color.setValue("#34699d");    // #3a3938
  squareStyle.style.setValue(AnnotationStyle::square);

  circleStyle.border.setValue(BorderData::BdrSquare,BorderData::BdrLnSolid,"#3a3938");
  circleStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  circleStyle.color.setValue("#34699d");    // #3a3938
  circleStyle.style.setValue(AnnotationStyle::circle);

  defaultStyle.font.setValuePoints("Arial,24,-1,5,50,0,0,0,0,0");
  defaultStyle.color.setValue("#3a3938");
  defaultStyle.margin.setValuesInches(0.0f,0.0f);

  margin.setValuesInches(DEFAULT_MARGIN,DEFAULT_MARGIN);
  pack.setValue(true);
  sort.setValue(false);
  sortBy.setValue(SortOptionName[PartSize]);

  // image generation
  cameraAngles.setFormats(6,1,"###9.0");
  cameraAngles.setRange(-360.0f,360.0f);
  cameraAngles.setValues(MetaDefaults::getPartCameraLatitude(),MetaDefaults::getPartCameraLongitude());
  cameraAngles.setType(static_cast<int>(Options::PLI));
  cameraDistance.setRange(1.0f,FLT_MAX);
  cameraFoV.setFormats(5,4,"9.999");
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(),
                     MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setFormats(5,0,"####9");
  cameraZNear.setRange(1.0f,FLT_MAX);
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setFormats(5,0,"####9");
  cameraZFar.setRange(1.0f,FLT_MAX);
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
  isOrtho.setValue(false);
  imageSize.setFormats(4,0,"###9");
  imageSize.setRange(0.0f,FLT_MAX);
  imageSize.setValues(0.0f,0.0f);

  // movable pli part groups
  enablePliPartGroup.setValue(false);
}

void PliMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement       .init(this,"PLACEMENT");
  constrain       .init(this,"CONSTRAIN");
  border          .init(this,"BORDER");
  background      .init(this,"BACKGROUND");
  margin          .init(this,"MARGINS");
  instance        .init(this,"INSTANCE_COUNT");
  annotate        .init(this,"ANNOTATE");
  modelScale      .init(this,"MODEL_SCALE");
  show            .init(this,"SHOW");
  ldviewParms     .init(this,"LDVIEW_PARMS");
  ldviewEnvVars   .init(this,"LDVIEW_ENV_VARS");
  ldgliteParms    .init(this,"LDGLITE_PARMS");
  ldgliteEnvVars  .init(this,"LDGLITE_ENV_VARS");
  povrayParms     .init(this,"POVRAY_PARMS");
  povrayEnvVars   .init(this,"POVRAY_ENV_VARS");
  includeSubs     .init(this,"INCLUDE_SUBMODELS");
  subModelColor   .init(this,"SUBMODEL_BACKGROUND_COLOR");
  subModelFont    .init(this,"SUBMODEL_FONT");
  subModelFontColor.init(this,"SUBMODEL_FONT_COLOR");
  individualParts .init(this,"SHOW_INDIVIDUAL_PARTS");
  part            .init(this,"PART");
  pliPartGroup    .init(this,"PART_GROUP");
  studStyle       .init(this,"STUD_STYLE");
  highContrast    .init(this,"HIGH_CONTRAST");
  autoEdgeColor   .init(this,"AUTOMATE_EDGE_COLOR");
  begin           .init(this,"BEGIN");
  end             .init(this,"END",           PliEndRc);
  sort            .init(this,"SORT");
  sortBy          .init(this,"SORT_BY"); // deprecated
  sortOrder       .init(this,"SORT_ORDER");
  annotation      .init(this,"ANNOTATION");
  partElements    .init(this,"PART_ELEMENTS");
  rectangleStyle  .init(this,"RECTANGLE_STYLE");
  circleStyle     .init(this,"CIRCLE_STYLE");
  squareStyle     .init(this,"SQUARE_STYLE");
  elementStyle    .init(this,"ELEMENT_STYLE");
  defaultStyle    .init(this,"DEFAULT_STYLE");
  imageSize       .init(this,"IMAGE_SIZE");
  rotStep         .init(this,"PART_ROTATION");
  cameraFoV       .init(this,"CAMERA_FOV");
  cameraZNear     .init(this,"CAMERA_ZNEAR");
  cameraZFar      .init(this,"CAMERA_ZFAR");
  cameraAngles    .init(this,"CAMERA_ANGLES");
  target          .init(this,"CAMERA_TARGET");
  position        .init(this,"CAMERA_POSITION");
  upvector        .init(this,"CAMERA_UPVECTOR");
  cameraDistance  .init(this,"CAMERA_DISTANCE");
  isOrtho         .init(this,"CAMERA_ORTHOGRAPHIC");
  enablePliPartGroup .init(this,"PART_GROUP_ENABLE");
  preferredRenderer .init(this,"PREFERRED_RENDERER");
}

void PliMeta::resetCameraFoV()
{
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(), MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
}

/* ------------------ */

BomMeta::BomMeta() : PliMeta()
{
  placement.setValue(CenterCenter,PageType);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_LINE_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgColor,"#ffffff");
  margin.setValuesInches(0.0f,0.0f);
  // instance - default
  // annotate - default
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"#99999.9");
  modelScale.setValue(1.0);
  show.setValue(true);
  ldgliteParms.setValue("-l3");
  ldviewParms.setValue("");
  povrayParms.setValue("+A");
  includeSubs.setValue(false);
  individualParts.setValue(false);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_01);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_02);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_03);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_04);
  part.margin.setValuesInches(0.05f,0.03f);
  instance.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  instance.margin.setValuesInches(0.0f,0.0f);
  //annotate.font.setValuePoints("Arial,18,-1,5,50,0,0,0,0,0");     // Rem at 2.3.7 05/01/19
  instance.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  annotate.color.setValue("#3a3938");                               // Add at revision 285 01/07/15
  annotate.margin.setValuesInches(0.0f,0.0f);

  elementStyle.border.setValue(BorderData::BdrNone,BorderData::BdrLnNone,"#ffffff");
  elementStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  elementStyle.color.setValue("#34699d");
  elementStyle.style.setValue(AnnotationStyle::element);
  elementStyle.size.setValuesInches(1.0f,STYLE_SIZE_DEFAULT);
  elementStyle.size.setRange(0.1f,1.0f);

  rectangleStyle.border.setValue(BorderData::BdrSquare,BorderData::BdrLnSolid,"#3a3938");
  rectangleStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  rectangleStyle.color.setValue("#34699d"); // #3a3938
  rectangleStyle.style.setValue(AnnotationStyle::rectangle);
  rectangleStyle.size.setValuesInches(STYLE_SIZE_DEFAULT,STYLE_SIZE_DEFAULT);
  rectangleStyle.size.setRange(0.1f,8.0f);

  squareStyle.border.setValue(BorderData::BdrSquare,BorderData::BdrLnSolid,"#3a3938");
  squareStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  squareStyle.color.setValue("#34699d");    // #3a3938
  squareStyle.style.setValue(AnnotationStyle::square);

  circleStyle.border.setValue(BorderData::BdrSquare,BorderData::BdrLnSolid,"#3a3938");
  circleStyle.background.setValue(BackgroundData::BgColor,"#ffffff");
  circleStyle.color.setValue("#34699d");    // #3a3938
  circleStyle.style.setValue(AnnotationStyle::circle);

  pack.setValue(false);
  sort.setValue(true);
  sortBy.setValue("Part Size");
  sortBy.setValue(SortOptionName[PartColour]);

  // image generation
  cameraAngles.setFormats(6,1,"###9.0");
  cameraAngles.setRange(-360.0f,360.0f);
  cameraAngles.setValues(MetaDefaults::getPartCameraLatitude(),MetaDefaults::getPartCameraLongitude());
  cameraAngles.setType(static_cast<int>(Options::PLI));
  cameraDistance.setRange(1.0f,FLT_MAX);
  cameraFoV.setFormats(5,4,"9.999");
  cameraFoV.setRange(MetaDefaults::getFOVMinRange(),
                     MetaDefaults::getFOVMaxRange());
  cameraFoV.setValue(MetaDefaults::getCameraFOV());
  cameraZNear.setFormats(5,0,"####9");
  cameraZNear.setRange(1.0f,FLT_MAX);
  cameraZNear.setValue(MetaDefaults::getNativeCameraZNear());
  cameraZFar.setFormats(5,0,"####9");
  cameraZFar.setRange(1.0f,FLT_MAX);
  cameraZFar.setValue(MetaDefaults::getNativeCameraZFar());
  isOrtho.setValue(false);
  imageSize.setFormats(4,0,"###9");
  imageSize.setRange(0.0f,FLT_MAX);
  imageSize.setValues(0.0f,0.0f);

  // movable pli part groups
  enablePliPartGroup.setValue(false);
}

void BomMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement       .init(this,"PLACEMENT");
  constrain       .init(this,"CONSTRAIN");
  border          .init(this,"BORDER");
  background      .init(this,"BACKGROUND");
  margin          .init(this,"MARGINS");
  instance        .init(this,"INSTANCE_COUNT");
  annotate        .init(this,"ANNOTATE");
  modelScale      .init(this,"MODEL_SCALE");
  show            .init(this,"SHOW");
  ldviewParms     .init(this,"LDVIEW_PARMS");
  ldviewEnvVars   .init(this,"LDVIEW_ENV_VARS");
  ldgliteParms    .init(this,"LDGLITE_PARMS");
  ldgliteEnvVars  .init(this,"LDGLITE_ENV_VARS");
  povrayParms     .init(this,"POVRAY_PARMS");
  povrayEnvVars   .init(this,"POVRAY_ENV_VARS");
  includeSubs     .init(this,"INCLUDE_SUBMODELS");
  subModelColor   .init(this,"SUBMODEL_BACKGROUND_COLOR");
  individualParts .init(this,"SHOW_INDIVIDUAL_PARTS");
  part            .init(this,"PART");
  pliPartGroup    .init(this,"PART_GROUP");
  studStyle       .init(this,"STUD_STYLE");
  highContrast    .init(this,"HIGH_CONTRAST");
  autoEdgeColor   .init(this,"AUTOMATE_EDGE_COLOR");
  begin           .init(this,"BEGIN");
  begin.ignore.rc = BomBeginIgnRc;
  end             .init(this,"END",BomEndRc);
  sort            .init(this,"SORT");
  sortBy          .init(this,"SORT_BY");  // deprecated
  sortOrder       .init(this,"SORT_ORDER");
  annotation      .init(this,"ANNOTATION");
  partElements    .init(this,"PART_ELEMENTS");
  elementStyle    .init(this,"ELEMENT_STYLE");
  rectangleStyle  .init(this,"RECTANGLE_STYLE");
  circleStyle     .init(this,"CIRCLE_STYLE");
  squareStyle     .init(this,"SQUARE_STYLE");
  defaultStyle    .init(this,"DEFAULT_STYLE");
  imageSize       .init(this,"IMAGE_SIZE");
  cameraFoV       .init(this,"CAMERA_FOV");
  cameraZNear     .init(this,"CAMERA_ZNEAR");
  cameraZFar      .init(this,"CAMERA_ZFAR");
  cameraAngles    .init(this,"CAMERA_ANGLES");
  cameraDistance  .init(this,"CAMERA_DISTANCE");
  target          .init(this,"CAMERA_TARGET");
  position        .init(this,"CAMERA_POSITION");
  upvector        .init(this,"CAMERA_UPVECTOR");
  enablePliPartGroup .init(this,"PART_GROUP_ENABLE");
  preferredRenderer .init(this,"PREFERRED_RENDERER");
}

/* ------------------ */

Rc CalloutBeginMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  int argc = argv.size() - index;

  if (argc == 0) {
    rc = CalloutBeginRc;
    mode = Unassembled;
  } else if (argc == 1 && argv[index] == "ASSEMBLED") {
    rc = CalloutBeginRc;
    mode = Assembled;
  } else if (argc == 1 && argv[index] == "ROTATED") {
    rc = CalloutBeginRc;
    mode = Rotated;
  }
  if (rc != FailureRc) {
      _here[0] = here;
      _here[1] = here;
    }
  return rc;
}

QString CalloutBeginMeta::format(bool local, bool global)
{
  QString foo;

  if (mode == Assembled) {
    foo = "ASSEMBLED";
  } else if (mode == Rotated) {
    foo = "ROTATED";
  }
  return LeafMeta::format(local,global,foo);
}

void CalloutBeginMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " [( ASSEMBLED | ROTATED | WHOLE )]";
}

void CalloutBeginMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " ASSEMBLED ROTATED WHOLE";
}

CalloutMeta::CalloutMeta() : BranchMeta()
{
  stepNum.font.setValuePoints("Arial,36,-1,255,75,0,0,0,0,0");
  stepNum.color.setValue("black");
  // stepNum.font - default
  stepNum.placement.setValue(LeftTopOutside,PartsListType);
  sep.setValueInches("Black",DEFAULT_LINE_THICKNESS,DEFAULT_MARGINS);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_LINE_THICKNESS;
  borderData.radius    = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  // subModelFont - default
  instance.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  instance.color.setValue("black");
  // instance - default
  instance.placement.setValue(RightBottomOutside, CalloutType);
  background.setValue(BackgroundData::BgSubmodelColor);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_01);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_02);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_03);
  subModelColor.setValue(DEFAULT_SUBMODEL_COLOR_04);
  subModelFontColor.setValue("black");
  placement.setValue(RightOutside,CsiType);
  // freeform
  alloc.setValue(Vertical);
  pli.placement.setValue(TopLeftOutside,CsiType);
  pli.perStep.setValue(true);
  // Submodel
  subModel.placement.setValue(RightTopOutside,StepNumberType);
  subModel.show.setValue(true);
  // Rotate Icon
  rotateIcon.placement.setValue(RightOutside,CsiType);
}

void CalloutMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin     .init(this,      "MARGINS");
  stepNum    .init(this,      "STEP_NUMBER");
  sep        .init(this,      "SEPARATOR");
  border     .init(this,      "BORDER");
  subModelFont.init(this,    "SUBMODEL_FONT");
  instance   .init(this,      "INSTANCE_COUNT");
  background .init(this,      "BACKGROUND");
  subModelColor.init(this,    "SUBMODEL_BACKGROUND_COLOR");
  subModelFontColor.init(this,"SUBMODEL_FONT_COLOR");
  placement  .init(this,      "PLACEMENT");
  freeform   .init(this,      "FREEFORM");
  alloc      .init(this,      "ALLOC");
  justifyStep.init(this,      "STEPS");
  pointer    .init(this,      "POINTER");
  divPointer .init(this,      "DIVIDER_POINTER");
  pointerAttrib.init(this,    "POINTER_ATTRIBUTE");
  divPointerAttrib.init(this, "DIVIDER_POINTER_ATTRIBUTE");

  begin      .init(this,      "BEGIN",   CalloutBeginRc);
  divider    .init(this,      "DIVIDER", CalloutDividerRc);
  end        .init(this,      "END",     CalloutEndRc);
  csi        .init(this,      "ASSEM");
  pli        .init(this,      "PLI");
  subModel   .init(this,      "SUBMODEL_DISPLAY");
  rotateIcon .init(this,      "ROTATE_ICON");
}

/*------------------------*/

PointerBaseMeta::PointerBaseMeta() : BranchMeta()
{
  placement.setValue(LeftInside,PageType);
  background.setValue(BackgroundData::BgTransparent);
  BorderData borderData;
  borderData.line      = BorderData::BdrLnNone;
  borderData.type      = BorderData::BdrSquare;
  borderData.color     = "Black";
  borderData.thickness = 0.0f;
  borderData.radius    = 0;
  borderData.margin[0] = 0;
  borderData.margin[1] = 0;
  border.setValueInches(borderData);
  margin.setValues(0,0);
}

void PointerBaseMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement    .init(this, "PLACEMENT");
  border       .init(this, "BORDER");
  background   .init(this, "BACKGROUND");
  margin       .init(this, "MARGINS");
}

/* ------------------ */

MultiStepMeta::MultiStepMeta() : BranchMeta()
{
  stepNum.placement.setValue(LeftTopOutside,PartsListType);
  stepNum.color.setValue("black");
  // stepNum.font - default
  placement.setValue(CenterCenter,PageType);
  sep.setValue("black",DEFAULT_LINE_THICKNESS,DEFAULT_MARGINS);
  // subModelFont - default
  subModelFontColor.setValue("black");
  // freeform
  alloc.setValue(Vertical);
  pli.placement.setValue(LeftTopOutside,CsiType);
  pli.perStep.setValue(true);
  showGroupStepNumber.setValue(true); // Used with pli per page
  countGroupSteps.setValue(false);    // Used by pli per page group step number
  // Submodel
  subModel.placement.setValue(RightTopOutside,StepNumberType);
  subModel.show.setValue(true);
  // Rotate Icon
  rotateIcon.placement.setValue(RightOutside,CsiType);
  adjustOnItemOffset.setValue(false);
  // Step number justify vertical axis outside placement for multiple ranges - page dividers
  justifyYAxisOutsidePlacement.setValue(false);
  // Set explicit step size
  stepSize.setValuesInches(0.0f,0.0f);
  stepSize.setRange(0.0f,1000.0f);
  stepSize.setFormats(6,4,"9.9999");
}

void MultiStepMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin.init(this,"MARGINS");
  stepNum.init(this,"STEP_NUMBER");
  placement.init(this,"PLACEMENT");
  sep.init(this,"SEPARATOR");
  justifyStep.init(this,"STEPS");

  divPointer.init(this,"DIVIDER_POINTER");
  divPointerAttrib.init(this,"DIVIDER_POINTER_ATTRIBUTE");

  subModelFont.init(this,"SUBMODEL_FONT");
  subModelFontColor.init(this,"SUBMODEL_FONT_COLOR");
  freeform.init(this,"FREEFORM");
  alloc.init(this,"ALLOC");
  csi.init(this,"ASSEM");
  pli.init(this,"PLI");
  showGroupStepNumber.init(this,"SHOW_GROUP_STEP_NUMBER");
  countGroupSteps.init(this, "COUNT_GROUP_STEPS");
  subModel.init(this,"SUBMODEL_DISPLAY");
  rotateIcon.init(this,"ROTATE_ICON");

  adjustOnItemOffset.init(this,"ADJUST_ON_ITEM_OFFSET");
  justifyYAxisOutsidePlacement.init(this,"JUSTIFY_Y_AXIS_OUTSIDE_PLACEMENT_MULTIPLE_RANGES");
  stepSize.init(this,"STEP_SIZE");

  begin.init(this,"BEGIN",  StepGroupBeginRc);
  divider.init(this,"DIVIDER",StepGroupDividerRc);
  end.init(this,"END",    StepGroupEndRc);
}

/* ------------------ */

void ResolutionMeta::init(BranchMeta *parent, const QString name)
{
  AbstractMeta::init(parent,name);
}

Rc ResolutionMeta::parse(QStringList &argv, int index, Where &here)
{
  int tokens = argv.size();
  tokens -= index;
  if (tokens == 0) {
    return FailureRc;
  }
  if (tokens == 2 && argv[index+1] == "DPI") {
    _here[0] = here;
    setResolution(argv[index].toFloat());
    setResolutionType(DPI);
  } else if (tokens == 2 && argv[index+1] == "DPCM") {
    _here[0] = here;
    setResolution(argv[index].toFloat());
    setResolutionType(DPCM);
  } else {
    return FailureRc;
  }
  setIsDefaultResolution(false);
  return ResolutionRc;
}

QString ResolutionMeta::format(bool local, bool global)
{
  QString res;
  res = QString("%1 DPI") .arg(double(resolution()),0,'f',0);
  return LeafMeta::format(local,global,res);
}

void ResolutionMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <integer> ( DPI | DPCM )";
}

void ResolutionMeta::metaKeywords(QStringList &out, QString preamble)
{
  out << preamble + " DPI DPCM";
}
/* ------------------ */

LightMeta::LightMeta() : BranchMeta()
{
  color.setValues(1.0f,1.0f,1.0f);

  specular.setFormats(9,1,"######9.9");
  specular.setRange(0.0f,9999.0f);
  specular.setValue(1.0f);

  spotConeAngle.setFormats(9,1,"######9.9");
  spotConeAngle.setRange(0.0f,FLT_MAX);
  spotConeAngle.setValue(80.0f);

  cutoffDistance.setFormats(9,1,"######9.9");
  cutoffDistance.setRange(0.0f,FLT_MAX);
  cutoffDistance.setValue(40.0f);

  povrayPower.setFormats(9,1,"######9.9");
  povrayPower.setRange(0.0f,FLT_MAX);
  povrayPower.setValue(1.0f);

  blenderPower.setFormats(9,1,"######9.9");
  blenderPower.setRange(0.0f,FLT_MAX);
  blenderPower.setValue(10.0f);

  diffuse.setFormats(4,2,"9.99");
  diffuse.setRange(0.0f,FLT_MAX);
  diffuse.setValue(1.0f);

  sunAngle.setFormats(5,1,"##9.9");
  sunAngle.setRange(0.0f,180.0f);
  sunAngle.setValue(11.4f);

  pointRadius.setFormats(9,1,"######9.9");
  pointRadius.setRange(0.0f,FLT_MAX);
  pointRadius.setValue(0.25f);

  spotRadius.setFormats(9,1,"######9.9");
  spotRadius.setRange(0.0f,FLT_MAX);
  spotRadius.setValue(0.25f);

  areaSizeX.setFormats(9,1,"######9.9");
  areaSizeX.setRange(0.0f,FLT_MAX);
  areaSizeX.setValue(250.0f);

  areaSizeY.setFormats(9,1,"######9.9");
  areaSizeY.setRange(0.0f,FLT_MAX);
  areaSizeY.setValue(250.0f);

  areaSize.setFormats(9,1,"######9.9");
  areaSize.setRange(0.0f,FLT_MAX);
  areaSize.setValue(250.0f);

  spotBlend.setFormats(4,2,"9.99");
  spotBlend.setRange(0.0f,FLT_MAX);
  spotBlend.setValue(0.15f);

  fadePower.setFormats(4,2,"9.99");
  fadePower.setRange(0.0f,FLT_MAX);
  fadePower.setValue(0.0f);

  fadeDistance.setFormats(4,2,"9.99");
  fadeDistance.setRange(0.0f,FLT_MAX);
  fadeDistance.setValue(0.0f);

  spotTightness.setFormats(4,2,"9.99");
  spotTightness.setRange(0.0f,FLT_MAX);
  spotTightness.setValue(0.0f);

  areaGridX.setRange(2,10000);
  areaGridY.setRange(2,10000);

  povrayLight = false;
  shadowless  = false;

  type.setValue("POINT");
  areaShape.setValue("SQUARE");
}

void LightMeta::setLatLong()
{
  const lcVector3 axisX(1, 0, 0);
  const lcVector3 axisY(0, 1, 0);
  const lcVector3 axisZ(0, 0, 1);
  const lcVector3 lightPosition(position.x(), position.y(), position.z());
  const lcVector3 lightTarget(target.x(), target.y(), target.z());
  lcVector3 frontVector(lightPosition - lightTarget);

  frontVector.Normalize();
  latitude = acos(lcDot(-frontVector, axisZ)) * LC_RTOD - 90.0f;

  const lcVector3 axisXY = -lcNormalize(lcVector3(frontVector.x, frontVector.y, 0.0f));
  longitude = acos(lcDot(axisXY, axisY)) * LC_RTOD;

  if (lcDot(axisXY, axisX) > 0)
    longitude = -longitude;
}

void LightMeta::init(BranchMeta *parent, QString _name)
{
  AbstractMeta::init(parent, _name);
  type.init(this,"TYPE",                    LeoCadLightTypeRc);  // Light NAME and TYPE written on same line, Convert string TYPE to type
  name.init(this,"NAME",                    LeoCadLightRc);
  areaShape.init(this,"AREA_SHAPE",              LeoCadLightRc);
  specular.init(this,"BLENDER_SPECULAR",        LeoCadLightRc);
  spotConeAngle.init(this,"SPOT_CONE_ANGLE",         LeoCadLightRc);
  cutoffDistance.init(this,"BLENDER_CUTOFF_DISTANCE", LeoCadLightRc);

  povrayPower.init(this,"POVRAY_POWER",            LeoCadLightRc);
  blenderPower.init(this,"BLENDER_POWER",           LeoCadLightRc);
  diffuse.init(this,"BLENDER_DIFFUSE",         LeoCadLightRc);

  sunAngle.init(this,"BLENDER_SUN_ANGLE",       LeoCadLightRc);
  pointRadius.init(this,"BLENDER_POINT_RADIUS",    LeoCadLightRc);
  spotRadius.init(this,"BLENDER_SPOT_RADIUS",     LeoCadLightRc);
  areaSizeX.init(this,"AREA_SIZE_X",             LeoCadLightRc);
  areaSizeY.init(this,"AREA_SIZE_Y",             LeoCadLightRc);
  areaSize.init(this,"AREA_SIZE",               LeoCadLightRc);
  spotBlend.init(this,"SPOT_BLEND",              LeoCadLightRc);
  fadePower.init(this,"POVRAY_FADE_POWER",       LeoCadLightRc);
  fadeDistance.init(this,"POVRAY_FADE_DISTANCE",    LeoCadLightRc);
  spotTightness.init(this,"POVRAY_SPOT_TIGHTNESS",   LeoCadLightRc);
  spotPenumbraAngle.init(this,"SPOT_PENUMBRA_ANGLE",     LeoCadLightRc);

  areaGridX.init(this,"POVRAY_AREA_GRID_X",      LeoCadLightRc);
  areaGridY.init(this,"POVRAY_AREA_GRID_Y",      LeoCadLightRc);
  areaWidth.init(this,"POVRAY_AREA_SIZE_X",      LeoCadLightRc);
  areaHeight.init(this,"POVRAY_AREA_SIZE_Y",      LeoCadLightRc);

  _povrayLight.init(this,"POV_RAY",                 LeoCadLightPOVRayRc);
  _shadowless.init(this,"SHADOWLESS",              LeoCadLightShadowless);

  color.init(this,"COLOR",                   LeoCadLightRc);
  target.init(this,"TARGET_POSITION",         LeoCadLightRc);
  position.init(this,"POSITION",                LeoCadLightRc);
  rotation.init(this,"ROTATION",                LeoCadLightRc);
}

/* ------------------ */

Rc NoStepMeta::parse(QStringList &argv, int index,Where &here )
{
  if (index == argv.size()) {
    return rc;
  } else {
    if (reportErrors) {
      QString const message = QMessageBox::tr("Unexpected token \"%1\" %2") .arg(argv[index], argv.join(" "));
      emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
    }
    return FailureRc;
  }
}
QString NoStepMeta::format(bool local, bool global)
{
  return LeafMeta::format(local,global,"");
}

void NoStepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

LPubMeta::LPubMeta() : BranchMeta()
{
  rotateIcon.placement.setValue(RightOutside,CsiType);
  subModel.placement.setValue(RightTopOutside,StepNumberType);
  stepNumber.placement.setValue(BottomLeftOutside,PageHeaderType);      // TopLeftInsideCorner,PageType
  stepNumber.color.setValue("black");
  contModelStepNum.setRange(0,10000);
  contModelStepNum.setValue(0);
  countInstance.setValue(CountAtModel);
  countInstanceByColour.setValue(true);
  contStepNumbers.setValue(false);
  startPageNumber.setRange(1,10000);
  startPageNumber.setValue(1);
  startStepNumber.setRange(1,10000);
  startStepNumber.setValue(1);
  parseNoStep.setValue(false);
  coverPageViewEnabled.setValue(true);
  setSubstituteAsUnofficialPart.setValue(false);
  cameraDDF.setFormats(4,2,"9.99");
  cameraDDF.setRange(1.0f,100.0f);
  cameraDDF.setValue(MetaDefaults::getCameraDDF());
  // stepNumber - default
}

void LPubMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  page                     .init(this,"PAGE");
  assem                    .init(this,"ASSEM");
  callout                  .init(this,"CALLOUT");
  multiStep                .init(this,"MULTI_STEP");
  stepNumber               .init(this,"STEP_NUMBER");
  pli                      .init(this,"PLI");
  bom                      .init(this,"BOM");
  buildMod                 .init(this,"BUILD_MOD");
  buildModEnabled          .init(this,"BUILD_MOD_ENABLED");
  finalModelEnabled        .init(this,"FINAL_MODEL_ENABLED");
  coverPageViewEnabled     .init(this,"COVER_PAGE_MODEL_VIEW_ENABLED");
  loadUnoffPartsInEditor   .init(this,"LOAD_UNOFFICIAL_PARTS_IN_EDITOR");
  setSubstituteAsUnofficialPart.init(this,"SET_SUBMODEL_SUBSTITUTE_AS_UNOFFICIAL_PART");
  pointerBase              .init(this,"POINTER_BASE");
  remove                   .init(this,"REMOVE");
  reserve                  .init(this,"RESERVE",ReserveSpaceRc);
  cameraDDF                .init(this,"CAMERA_DEFAULT_DISTANCE_FACTOR");
  partSub                  .init(this,"PART");
  resolution               .init(this,"RESOLUTION");
  insert                   .init(this,"INSERT");
  include                  .init(this,"INCLUDE", IncludeRc);
  nostep                   .init(this,"NOSTEP", NoStepRc);
  fadeSteps                .init(this,"FADE_STEPS");
  highlightStep            .init(this,"HIGHLIGHT_STEP");
  preferredRenderer        .init(this,"PREFERRED_RENDERER");
  subModel                 .init(this,"SUBMODEL_DISPLAY");
  rotateIcon               .init(this,"ROTATE_ICON");
  studStyle                .init(this,"STUD_STYLE");
  highContrast             .init(this,"HIGH_CONTRAST");
  parseNoStep              .init(this,"PARSE_NOSTEP");
  autoEdgeColor            .init(this,"AUTOMATE_EDGE_COLOR");
  countInstance            .init(this,"CONSOLIDATE_INSTANCE_COUNT");
  countInstanceByColour    .init(this,"CONSOLIDATE_INSTANCE_COUNT_BY_COLOR");
  contModelStepNum         .init(this,"MODEL_STEP_NUMBER");
  contStepNumbers          .init(this,"CONTINUOUS_STEP_NUMBERS");
  stepPli                  .init(this,"STEP_PLI");
  startStepNumber          .init(this,"START_STEP_NUMBER", StartStepNumberRc);
  startPageNumber          .init(this,"START_PAGE_NUMBER", StartPageNumberRc);

  group                    .init(this,"GROUP");
  light                    .init(this,"LIGHT");
  camera                   .init(this,"CAMERA", LeoCadCameraRc);
  model                    .init(this,"MODEL",  LeoCadModelRc);
  piece                    .init(this,"PIECE",  LeoCadPieceRc);
  synth                    .init(this,"SYNTH",  LeoCadSynthRc);

  reserve.setRange(0.0,1000000.0);
}

void LPubMeta::resetCamerasFoV()
{
  pli.resetCameraFoV();
  bom.resetCameraFoV();
  subModel.resetCameraFoV();
  assem.resetCameraFoV();
  callout.csi.resetCameraFoV();
  multiStep.csi.resetCameraFoV();
}

/* ------------------ */

void PartTypeMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  partType .init(this,"TYPE", PartTypeRc);
}

/* ------------------ */

Rc PartTypeMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    rc = OkRc;
  } else {
    rc = i.value()->parse(argv,index+1,here);
  }
  return rc;
}

/* ------------------ */

void PartNameMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  partName .init(this,"NAME", PartNameRc);
}

/* ------------------ */

Rc PartNameMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    rc = OkRc;
  } else {
    rc = i.value()->parse(argv,index+1,here);
  }
  return rc;
}

/* ------------------ */

void MLCadMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  MLCadSB .init(this,"SKIP_BEGIN", MLCadSkipBeginRc);
  MLCadSE .init(this,"SKIP_END",   MLCadSkipEndRc);
  MLCadGrp.init(this,"BTG",        MLCadGroupRc);
}

Rc MLCadMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    rc = OkRc;
  } else {
    rc = i.value()->parse(argv,index+1,here);
  }
  return rc;
}

/* ------------------ */

void LDCadMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  LDCadGrp.       init(this,"GROUP_NXT", LDCadGroupRc);
}

/* ------------------ */

Rc LDCadMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    rc = OkRc;
  } else {
    rc = i.value()->parse(argv,index+1,here);
  }
  return rc;
}

/* ------------------ */

void LeoCadMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  group   .init(this,"GROUP");
  light   .init(this,"LIGHT");
  camera  .init(this,"CAMERA", LeoCadCameraRc);
  model   .init(this,"MODEL",  LeoCadModelRc);
  piece   .init(this,"PIECE",  LeoCadPieceRc);
  synth   .init(this,"SYNTH",  LeoCadSynthRc);
}

/* ------------------ */

Rc LeoCadMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    rc = OkRc;
  } else {
    rc = i.value()->parse(argv,index+1,here);
  }
  return rc;
}

/* ------------------ */

void LeoCadGroupMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  begin .init(this,"BEGIN", LeoCadGroupBeginRc);
  end   .init(this,"END",   LeoCadGroupEndRc);
}

/* ------------------ */

Rc LeoCadGroupMeta::parse(QStringList &argv, int index,Where &here)   //
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
    rc = OkRc;
  } else {
    rc = i.value()->parse(argv,index+1,here);
  }
  return rc;
}

/* ------------------ */

void LSynthMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  begin      .init(this, "BEGIN", SynthBeginRc);
  end        .init(this, "END",   SynthEndRc);
  show       .init(this, "SHOW");
  hide       .init(this, "HIDE");
  inside     .init(this, "INSIDE");
  outside    .init(this, "OUTSIDE");
  cross      .init(this, "CROSS");
  synthesized.init(this, "SYNTHESIZED");
}

/* ------------------ */

Meta::Meta() : BranchMeta()
{
  QString empty;
  init(nullptr,empty);
}

void Meta::init(BranchMeta * /* unused */, QString /* unused */)
{
  preamble = "0 ";

  LPub       .init(this,"!LPUB");
  step       .init(this,"STEP",        StepRc);
  clear      .init(this,"CLEAR",       ClearRc);
  fade       .init(this,"!FADE",       FadeRc);
  silhouette .init(this,"!SILHOUETTE", SilhouetteRc);
  texmap     .init(this,"!TEXMAP",     TexMapRc);
  colour     .init(this,"!COLOUR",     ColourRc);
  rotStep    .init(this,"ROTSTEP");
  bfx        .init(this,"BUFEXCHG");
  MLCad      .init(this,"MLCAD");
  LDCad      .init(this,"!LDCAD");
  LeoCad     .init(this,"!LEOCAD");
  LSynth     .init(this,"SYNTH");

  /*
   * The token map translates known keywords to values
   * used by LPub to identify things like placement and such
   */

  if (tokenMap.size() == 0) {
    tokenMap["TOP_LEFT"]             = TopLeft;
    tokenMap["TOP"]                  = Top;
    tokenMap["TOP_RIGHT"]            = TopRight;
    tokenMap["RIGHT"]                = Right;
    tokenMap["BOTTOM_RIGHT"]         = BottomRight;
    tokenMap["BOTTOM"]               = Bottom;
    tokenMap["BOTTOM_LEFT"]          = BottomLeft;
    tokenMap["LEFT"]                 = Left;
    tokenMap["CENTER"]               = Center;

    tokenMap["INSIDE"]               = Inside;
    tokenMap["OUTSIDE"]              = Outside;

    tokenMap["PAGE"]                 = PageType;
    tokenMap["ASSEM"]                = CsiType;
    tokenMap["MULTI_STEP"]           = StepGroupType;
    tokenMap["STEP_GROUP"]           = StepGroupType;
    tokenMap["STEP_NUMBER"]          = StepNumberType;
    tokenMap["PLI"]                  = PartsListType;
    tokenMap["PAGE_NUMBER"]          = PageNumberType;
    tokenMap["CALLOUT"]              = CalloutType;
    tokenMap["SUBMODEL_INST_COUNT"]  = SubmodelInstanceCountType;
    tokenMap["SUBMODEL_DISPLAY"]     = SubModelType;
    tokenMap["ROTATE_ICON"]          = RotateIconType;
    tokenMap["PAGE_POINTER"]         = PagePointerType;
    tokenMap["DIVIDER_POINTER"]      = DividerPointerType;
    tokenMap["CSI_ANNOTATION"]       = AssemAnnotationObj;
    tokenMap["CSI_ANNOTATION_PART"]  = AssemAnnotationPartObj;
    tokenMap["CALLOUT_INSTANCE"]     = CalloutInstanceObj;
    tokenMap["CALLOUT_POINTER"]      = CalloutPointerObj;
    tokenMap["CALLOUT_UNDERPINNING"] = CalloutUnderpinningObj;
    tokenMap["DIVIDER"]              = DividerObj;
    tokenMap["DIVIDER_ITEM"]         = DividerBackgroundObj;
    tokenMap["DIVIDER_LINE"]         = DividerLineObj;
    tokenMap["MULTI_STEPS"]          = MultiStepsBackgroundObj;
    tokenMap["POINTER_GRABBER"]      = PointerGrabberObj;
    tokenMap["PLI_GRABBER"]          = PliGrabberObj;
    tokenMap["SUBMODEL_GRABBER"]     = SubmodelGrabberObj;
    tokenMap["PICTURE"]              = InsertPixmapObj;
    tokenMap["ATTRIBUTE_PIXMAP"]     = PageAttributePixmapObj;
    tokenMap["ATTRIBUTE_TEXT"]       = PageAttributeTextObj;
    tokenMap["PLI_ANNOTATION"]       = PartsListAnnotationObj;
    tokenMap["PLI_INSTANCE"]         = PartsListInstanceObj;
    tokenMap["POINTER_SEG_FIRST"]    = PointerFirstSegObj;
    tokenMap["POINTER_HEAD"]         = PointerHeadObj;
    tokenMap["POINTER_SEG_SECOND"]   = PointerSecondSegObj;
    tokenMap["POINTER_SEG_THIRD"]    = PointerThirdSegObj;
    tokenMap["SUBMODEL_INSTANCE"]    = SubModelInstanceObj;
    tokenMap["PLI_PART"]             = PartsListPixmapObj;
    tokenMap["PLI_PART_GROUP"]       = PartsListGroupObj;
    tokenMap["STEP_RECTANGLE"]       = StepBackgroundObj;

    tokenMap["DOCUMENT_TITLE"]       = PageTitleType;
    tokenMap["MODEL_ID"]             = PageModelNameType;
    tokenMap["DOCUMENT_AUTHOR"]      = PageAuthorType;
    tokenMap["PUBLISH_URL"]          = PageURLType;
    tokenMap["MODEL_DESCRIPTION"]    = PageModelDescType;
    tokenMap["PUBLISH_DESCRIPTION"]  = PagePublishDescType;
    tokenMap["PUBLISH_COPYRIGHT"]    = PageCopyrightType;
    tokenMap["PUBLISH_EMAIL"]        = PageEmailType;
    tokenMap["LEGO_DISCLAIMER"]      = PageDisclaimerType;
    tokenMap["MODEL_PARTS"]          = PagePartsType;
    tokenMap["APP_PLUG"]             = PagePlugType;
    tokenMap["DOCUMENT_LOGO"]        = PageDocumentLogoType;
    tokenMap["DOCUMENT_COVER_IMAGE"] = PageCoverImageType;
    tokenMap["APP_PLUG_IMAGE"]       = PagePlugImageType;
    tokenMap["PAGE_HEADER"]          = PageHeaderType;
    tokenMap["PAGE_FOOTER"]          = PageFooterType;
    tokenMap["MODEL_CATEGORY"]       = PageCategoryType;

    tokenMap["SINGLE_STEP"]          = SingleStepType;
    tokenMap["TEXT"]                 = TextType;
    tokenMap["STEP"]                 = StepType;
    tokenMap["RANGE"]                = RangeType;
    tokenMap["RESERVE"]              = ReserveType;
    tokenMap["COVER_PAGE"]           = CoverPageType;

    tokenMap["AREA"]                 = ConstrainData::PliConstrainArea;
    tokenMap["SQUARE"]               = ConstrainData::PliConstrainSquare;
    tokenMap["WIDTH"]                = ConstrainData::PliConstrainWidth;
    tokenMap["HEIGHT"]               = ConstrainData::PliConstrainHeight;
    tokenMap["COLS"]                 = ConstrainData::PliConstrainColumns;

    tokenMap["HORIZONTAL"]           = Horizontal;
    tokenMap["VERTICAL"]             = Vertical;

    tokenMap["ASPECT"]               = Aspect;
    tokenMap["STRETCH"]              = Stretch;
    tokenMap["TILE"]                 = Tile;

    tokenMap["PORTRAIT"]             = Portrait;
    tokenMap["LANDSCAPE"]            = Landscape;

    tokenMap["SORT_BY"]              = SortByOptions;

    tokenMap["BASE_TOP_LEFT"]        = TopLeftInsideCorner;
    tokenMap["BASE_TOP"]             = TopInside;
    tokenMap["BASE_TOP_RIGHT"]       = TopRightInsideCorner;
    tokenMap["BASE_LEFT"]            = LeftInside;
    tokenMap["BASE_CENTER"]          = CenterCenter;
    tokenMap["BASE_RIGHT"]           = RightInside;
    tokenMap["BASE_BOTTOM_LEFT"]     = BottomLeftInsideCorner;
    tokenMap["BASE_BOTTOM"]          = BottomInside;
    tokenMap["BASE_BOTTOM_RIGHT"]    = BottomRightInsideCorner;

    tokenMap["PAGE_LENGTH"]          = SepData::LenPage;
    tokenMap["CUSTOM_LENGTH"]        = SepData::LenCustom;

    tokenMap["PRIMARY"]              = SortPrimary;
    tokenMap["SECONDARY"]            = SortSecondary;
    tokenMap["TERTIARY"]             = SortTetriary;
    tokenMap["PRIMARY_DIRECTION"]    = PrimaryDirection;
    tokenMap["SECONDARY_DIRECTION"]  = SecondaryDirection;
    tokenMap["TERTIARY_DIRECTION"]   = TertiaryDirection;

    tokenMap["Part Size"]            = PartSize;
    tokenMap["Part Color"]           = PartColour;
    tokenMap["Part Category"]        = PartCategory;
    tokenMap["Part Element"]         = PartElement;
    tokenMap["No Sort"]              = NoSort;
    tokenMap["Ascending"]            = SortAscending;
    tokenMap["Descending"]           = SortDescending;

    tokenMap["BRING_TO_FRONT"]       = BringToFront;
    tokenMap["SEND_TO_BACK"]         = SendToBack;

    tokenMap["JUSTIFY_CENTER"]            = JustifyCenter;
    tokenMap["JUSTIFY_CENTER_HORIZONTAL"] = JustifyCenterHorizontal;
    tokenMap["JUSTIFY_CENTER_VERTICAL"]   = JustifyCenterVertical;
    tokenMap["JUSTIFY_LEFT"]              = JustifyLeft;
  }

  if (groupRegExMap.size() == 0)
  {
    groupRegExMap[MLCadGroupRc] = QRegularExpression("^\\s*0\\s+(MLCAD)\\s+(BTG)\\s+(.*)$");
    groupRegExMap[LDCadGroupRc] = QRegularExpression("^\\s*0\\s+!?(LDCAD)\\s+(GROUP_NXT)\\s+\\[ids=([\\d\\s\\,]+)\\].*$");
    groupRegExMap[LeoCadGroupBeginRc] = QRegularExpression("^\\s*0\\s+!?(LPUB|LEOCAD)\\s+(GROUP BEGIN)\\s+Group\\s+(.*)$", QRegularExpression::CaseInsensitiveOption);
    groupRegExMap[LeoCadGroupEndRc] = QRegularExpression("^\\s*0\\s+!?(LPUB|LEOCAD)\\s+(GROUP)\\s+(END)$");
  }
}

Rc Meta::parse( QString &line,Where &here, bool reportErrors)
{
  AbstractMeta::reportErrors = reportErrors;

  auto parseGroupMeta = [&line]()
  {
    QHash<Rc, QRegularExpression>::const_iterator i = groupRegExMap.constBegin();
    while (i != groupRegExMap.constEnd()) {
      QRegularExpressionMatch match = i.value().match(line);
      if (match.hasMatch())
        return QStringList() << match.captured(1) << match.captured(2) << match.captured(3);
      ++i;
    }
    return QStringList();
  };

  QStringList argv = parseGroupMeta();

  if (argv.isEmpty()) {

    processSpecialCases(line,here);

    /* Parse the input line into argv[] */

    split(line,argv);

    if (argv.size() > 0) {
      argv.removeFirst();
    }
    if (argv.size()) {
      if (argv[0] == "LPUB") {
        argv[0] = "!LPUB";
      }

      if (argv[0] == "PLIST") {
        return  LPub.pli.parse(argv,1,here);
      }
    }
  }

  if (argv.size() > 0 && list.contains(argv[0])) {

    /* parse it up */

    Rc rc = this->BranchMeta::parse(argv,0,here);

    if (rc == FailureRc) {
      if (reportErrors) {
        QString const message = QMessageBox::tr("Parse failed %1:%2\n%3").arg(here.modelName) .arg(here.lineNumber) .arg(line);
        emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false,false);
      }
    }
    return rc;
  }

  return OkRc;
}

bool Meta::preambleMatch(QString &line, QString &preamble)
{
  QStringList argv;

  /* Parse the input line into argv[] */

  split(line,argv);

  if (argv.size() > 0) {
    argv.removeFirst();
  }

  if (argv.size() && list.contains(argv[0])) {
    return BranchMeta::preambleMatch(argv,0,preamble);
  } else {
    return false;
  }
}

void Meta::pop()
{
  BranchMeta::pop();
}

void Meta::doc(QStringList &out)
{
  QString key;
  QStringList keys = list.keys();
  keys.sort();
  Q_FOREACH (key, keys) {
    if (key == "!COLOUR") {
      out << "0 !COLOUR \n0 // Fade previous steps custom colour command\n0 !COLOUR \"LPub3D_Fade_<LDraw_colour_name>\" CODE <100 + LDraw colour code> VALUE <\"#RRGGBB\"> EDGE <\"#RRGGBB\"> ALPHA <opacity 0-255>\n"
             "0 // Highlight current step custom colour command\n0 !COLOUR \"LPub3D_Highlight_<LDraw_colour_name>\" CODE <110 + LDraw colour code> VALUE <\"#RRGGBB\"> EDGE <\"#RRGGBB\"> ALPHA <opacity 255>";
      continue;
    } else if (key == "!FADE") {
      out << "0 !FADE \n0 // Fade previous steps block opening command\n0 !FADE <fade percent integer> <LDraw colour code>\n0 // Block closing command\n0 !FADE";
      continue;
    } else if (key == "!SILHOUETTE") {
      out << "0 !SILHOUETTE \n0 // Highlight current step block opening command\n0 !SILHOUETTE <edge line width decimal> <LDraw colour code>\n0 // Block closing command\n0 !SILHOUETTE";
      continue;
    }
    list[key]->doc(out, "0 " + key);
  }
}

void Meta::metaKeywords(QStringList &out, bool highlighter)
{
  QElapsedTimer timer;
  timer.start();
  const QString miscellaneousKeyWords =
          "Ascending Category Color Descending Element No Part Size Sort "
          "CAMERA_DISTANCE_NATIVE DOCUMENT_LOGO DOCUMENT_TITLE "
          "RANGE SINGLE_STEP STEP_GROUP VIEW_ANGLE";
  const QString ldrawKeywords =
          "Author: Name: !CATEGORY !CMDLINE !HELP !HELP !HISTORY !KEYWORDS !LDRAW_ORG "
          "!LICENSE !THEME ~MOVED 48_PRIMITIVE 8_PRIMITIVE ALPHA AUTHOR BFC CCW CERTIFY "
          "CLEAR CODE COLOUR EDGE FILE LDRAW LDRAW_ORG MODEL NAME NOFILE NOSTEP OFFICIAL "
          "ORIGINAL PART PART ALIAS PART PHYSICAL_COLOUR PAUSE PRIMITIVE PRINT SAVE SHORTCUT "
          "SHORTCUT ALIAS SHORTCUT PHYSICAL_COLOUR STEP SUBPART TO UNOFFICIAL UN-OFFICIAL "
          "UNOFFICIAL_48_PRIMITIVE UNOFFICIAL_8_PRIMITIVE UNOFFICIAL_PART UNOFFICIAL_PART "
          "ALIAS UNOFFICIAL_PART PHYSICAL_COLOUR UNOFFICIAL_PRIMITIVE UNOFFICIAL_SHORTCUT "
          "UNOFFICIAL_SHORTCUT ALIAS UNOFFICIAL_SHORTCUT PHYSICAL_COLOUR UNOFFICIAL_SUBPART "
          "VALUE WRITE";
  const QString mlcadKeyWords =
          "ABS ADD AXLE BACKGROUND BGT BUFEXCHG CENTER CONFIG END FLEXHOSE GHOST GROUP "
          "HIDE MLCAD REL RETRIEVE ROTATION ROTSTEP RUBBER_BELT SKIP_BEGIN SKIP_END "
          "SPRING STORE";
  const QString ldcadKeyWords =
          "!LDCAD CONTENT GENERATED GROUP_DEF GROUP_NXT GROUP_OBJ MARKER PATH_ANCHOR PATH_CAP "
          "PATH_LENGTH PATH_POINT PATH_SKIN SCRIPT SNAP_CLEAR SNAP_CLP SNAP_CYL SNAP_FGR SNAP_GEN "
          "SNAP_INCL SNAP_SPH SPRING_ANCHOR SPRING_CAP SPRING_POINT SPRING_SECTION";
  const QString leocadKeyWords =
          "!LEOCAD AUTHOR BACKGROUND BEGIN CAMERA COLOR POVRAY_AREA_GRID_X_KEY POVRAY_AREA_GRID_Y_KEY AREA_SIZE_KEY "
          "AREA_SIZE_X_KEY AREA_SIZE_Y_KEY COLOR_KEY COMMENT CUTOFF_DISTANCE_KEY DESCRIPTION POVRAY_SPOT_FALLOFF_KEY END FOV "
          "GRADIENT GROUP HIDDEN IMAGE MODEL NAME ORTHOGRAPHIC PIECE RADIUS_KEY AREA_SHAPE_KEY PIVOT POVRAY_FADE_POWER_KEY "
          "POSITION_KEY POVRAY_POWER_KEY POVRAY_FADE_DISTANCE_KEY BLENDER_POINT_RADIUS_KEY BLENDER_SPOT_RADIUS_KEY "
          "POVRAY_SPOT_TIGHTNESS_KEY SIZE_KEY BLENDER_SPECULAR_KEY SPOT_CONE_ANGLE_KEY STEP_HIDE BLENDER_POWER_KEY "
          "BLENDER_SUN_ANGLE_KEY TARGET_POSITION_KEY SPOT_PENUMBRA_ANGLE_KEY TYPE_KEY UP_VECTOR UP_VECTOR_KEY ZFAR ZNEAR";
  const QString lsynthKeyWords =
          "BEGIN CHAIN CROSS ELECTRIC_CABLE END FIBER_OPTIC_CABLE FLEX_CABLE "
          "FLEXIBLE_AXLE FLEXIBLE_TUBE HIDE INSIDE OUTSIDE PLASTIC_TREAD PNEUMATIC_TUBE "
          "RIBBED_TUBE RIGID_TUBE RUBBER_BAND RUBBER_TREAD SHOW SYNTH SYNTHESIZED";
  const QString highlighterKeyWords =
          "CAMERA_DISTANCE_NATIVE DOCUMENT_LOGO DOCUMENT_TITLE RANGE SINGLE_STEP VIEW_ANGLE";
  QStringList keys = list.keys();
  keys.sort();
  QStringList metaKeywords, cleanedKeywords;
  Q_FOREACH (const QString &key, keys) {
    list[key]->metaKeywords(metaKeywords, key);
  }
  if (highlighter)
    metaKeywords << highlighterKeyWords;
  else
    metaKeywords
    << ldrawKeywords
    << leocadKeyWords
    << ldcadKeyWords
    << mlcadKeyWords
    << lsynthKeyWords
    << miscellaneousKeyWords
    ;
  Q_FOREACH (const QString &keywordString, metaKeywords) {
    cleanedKeywords << keywordString.split(" ");
  }
  cleanedKeywords.removeDuplicates();
  cleanedKeywords.sort();
  out << cleanedKeywords;

  QString const message = QObject::tr("Meta command keywords generated. %1")
                              .arg(LPub::elapsedTime(timer.elapsed()));
  Preferences::setMessageLogging(DEFAULT_LOG_LEVEL);
  logInfo() << message;
  Preferences::setMessageLogging();
}

void Meta::processSpecialCases(QString &line, Where &here) {
  static QRegularExpression parseRx("\\s+(VIEW_ANGLE|MODEL_PIECES|BLENDER_DIRECTIONAL_ANGLE|COLOR_RGB|CAMERA_DISTANCE_NATIVE)\\s+");
  QRegularExpressionMatch match = parseRx.match(line);
  if (!match.hasMatch())
    return;

  /* Legacy LPub backward compatibilty. Replace VIEW_ANGLE with CAMERA_ANGLES */
  else if (match.captured(1) == QLatin1String("VIEW_ANGLE")) {
    line.replace(match.captured(1),"CAMERA_ANGLES");
    return;
  }

  /* Legacy LPub backward compatibilty. Replace MODEL_PIECES with MODEL_PARTS */
  else if (match.captured(1) == QLatin1String("MODEL_PIECES")) {
    line.replace(match.captured(1),"MODEL_PARTS");
    return;
  }

  /* LPub LeoCAD light compatibilty. Replace _DIRECTIONAL_ with _SUN_ */
  else if (match.captured(1) == QLatin1String("BLENDER_DIRECTIONAL_ANGLE")) {
    line.replace(match.captured(1),"BLENDER_SUN_ANGLE");
    return;
  }

  /* LPub LeoCAD light compatibilty. Replace COLOR_RGB with COLOR */
  else if (match.captured(1) == QLatin1String("COLOR_RGB")) {
    line.replace(match.captured(1),"COLOR");
    return;
  }

  /* Native camera distance deprecated. Command ignored if not GLOBAL */
  else if (match.captured(1) == QLatin1String("CAMERA_DISTANCE_NATIVE")) {
    if (Gui::parsedMessages.contains(here)) {
      line = "0 // IGNORED";
    } else if (Gui::pageProcessRunning == PROC_WRITE_TO_TMP) {
      parseRx.setPattern("\\s+(ASSEM|PLI|BOM|SUBMODEL|LOCAL)\\s+");
      if (line.contains(parseRx)) {
        QString const message = QObject::tr("CAMERA_DISTANCE_NATIVE meta command is no longer supported for %1 type. "
                                            "Only application at GLOBAL scope is permitted. "
                                            "Reclassify or remove this command and use MODEL_SCALE to implicate camera distance. "
                                            "This command will be ignored. %2")
                                            .arg(match.captured(1), line);
        here.setModelIndex(lpub->ldrawFile.getSubmodelIndex(here.modelName));
        emit gui->parseErrorSig(message,here,Preferences::ParseErrors,false/*option*/,false/*override*/);
        line = "0 // IGNORED";
      }
    }
    return;
  }
}
