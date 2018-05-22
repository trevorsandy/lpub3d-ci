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

#include <QtWidgets>

#include "nativepov.h"
#include "lpub.h"
#include "lpub_preferences.h"
#include "render.h"
#include "version.h"
#include "math.h"
#include "paths.h"
#include "QsLog.h"

#include "lc_file.h"
#include "pieceinf.h"
#include "view.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif

NativePov nativePov;

bool NativePov::CreateNativePovFile(const NativeOptions& Options)
{
          // Options
          m_InputFileName = Options.InputFileName;
          m_OutputFileName = Options.OutputFileName;
          m_PovGenCommand = Options.PovGenCommand;
          m_ImageType = Options.ImageType;
          m_ImageWidth = Options.ImageWidth;
          m_ImageHeight = Options.ImageHeight;
          m_Latitude = Options.Latitude;
          m_Longitude = Options.Longitude;
          m_CameraDistance = Options.CameraDistance;
          m_TransBackground = Options.TransBackground;
          m_Orthographic = Options.Orthographic;

          Project* PovGenProject = new Project();

          if (PovGenProject->Load(Options.InputFileName))
          {
            gApplication->SetProject(PovGenProject);
            gMainWindow->UpdateAllViews();
          }
          else
          {
            emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not load Native POV CSI ldr file."));
            delete PovGenProject;
            return false;
          }

         QString Type = Options.ImageType == Render::CSI ? "CSI" : "PLI";

         lcCamera* Camera = gMainWindow->GetActiveView()->mCamera;
         //Camera->SetOrtho(Options.Orthographic);
         Camera->SetAngles(Options.Latitude,Options.Longitude);
         Camera->Zoom(Options.CameraDistance,PovGenProject->mModels[0]->GetCurrentStep(),true);

         lcArray<lcModelPartsEntry> ModelParts;

         PovGenProject->GetModelParts(ModelParts);

         if (ModelParts.IsEmpty())
         {
                 emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Nothing to export - %1 parts list is empty.").arg(Type));
                 delete PovGenProject;
                 return false;
         }

        QString SaveFileName = PovGenProject->GetExportFileName(Options.OutputFileName, QLatin1String("pov"), QMessageBox::tr("Export POV-Ray"), QMessageBox::tr("POV-Ray Files (*.pov);;All Files (*.*)"));

	if (SaveFileName.isEmpty())
	{
		delete PovGenProject;
		return false;
	}

	lcDiskFile POVFile(SaveFileName);

	if (!POVFile.Open(QIODevice::WriteOnly))
	{
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not open %1 pov file '%2' for writing.").arg(Type).arg(SaveFileName));
		delete PovGenProject;
		return false;
	}
	m_pPOVFile = &POVFile;

	writePovHeader(Options.ImageType, Options.PovGenCommand);

	char Line[1024];

	lcPiecesLibrary* Library = lcGetPiecesLibrary();
	std::map<PieceInfo*, std::pair<char[LC_PIECE_NAME_LEN], int>> PieceTable;
	int NumColors = gColorList.GetSize();
	std::vector<std::array<char, LC_MAX_COLOR_NAME>> ColorTable(NumColors);

	enum
	{
		LGEO_PIECE_LGEO  = 0x01,
		LGEO_PIECE_AR    = 0x02,
		LGEO_PIECE_SLOPE = 0x04
	};

	enum
	{
		LGEO_COLOR_SOLID       = 0x01,
		LGEO_COLOR_TRANSPARENT = 0x02,
		LGEO_COLOR_CHROME      = 0x04,
		LGEO_COLOR_PEARL       = 0x08,
		LGEO_COLOR_METALLIC    = 0x10,
		LGEO_COLOR_RUBBER      = 0x20,
		LGEO_COLOR_GLITTER     = 0x40
	};

	QString LGEOPath = Preferences::lgeoPath;

	if (LGEOPath.isEmpty())
		POVFile.WriteLine("global_settings {\n  assumed_gamma 1.0\n}\n\n");

	POVFile.WriteLine("#declare lg_quality = POVQual;\n#if (lg_quality = 3)\n#declare lg_quality = 4;\n#end\n\n");

	writePovDeclare("lg_studs","POVStuds");

	writePovBlankLine();

	if (!LGEOPath.isEmpty())
	{
		lcDiskFile TableFile(QFileInfo(QDir(LGEOPath), QLatin1String("lg_elements.lst")).absoluteFilePath());

		if (!TableFile.Open(QIODevice::ReadOnly))
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not find LGEO lg_elements.lst file in folder '%1'.").arg(LGEOPath));
			delete PovGenProject;
			return false;
		}

		while (TableFile.ReadLine(Line, sizeof(Line)))
		{
			char Src[1024], Dst[1024], Flags[1024];

			if (*Line == ';')
				continue;

			if (sscanf(Line,"%s%s%s", Src, Dst, Flags) != 3)
				continue;

			strcat(Src, ".dat");

			PieceInfo* Info = Library->FindPiece(Src, nullptr, false, false);
			if (!Info)
				continue;

			if (strchr(Flags, 'L'))
			{
				std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];
				Entry.second |= LGEO_PIECE_LGEO;
				sprintf(Entry.first, "lg_%s", Dst);
			}

			if (strchr(Flags, 'A'))
			{
				std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];
				Entry.second |= LGEO_PIECE_AR;
				sprintf(Entry.first, "ar_%s", Dst);
			}

			if (strchr(Flags, 'S'))
			{
				std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];
				Entry.second |= LGEO_PIECE_SLOPE;
				Entry.first[0] = 0;
			}
		}

		lcDiskFile ColorFile(QFileInfo(QDir(LGEOPath), QLatin1String("lg_colors.lst")).absoluteFilePath());

		if (!ColorFile.Open(QIODevice::ReadOnly))
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not find LGEO lg_colors.lst file in folder '%1'.").arg(LGEOPath));
			delete PovGenProject;
			return false;
		}

		while (ColorFile.ReadLine(Line, sizeof(Line)))
		{
			char Name[1024], Flags[1024];
			int Code;

			if (*Line == ';')
				continue;

			if (sscanf(Line,"%d%s%s", &Code, Name, Flags) != 3)
				continue;

			int Color = lcGetColorIndex(Code);
			if (Color >= NumColors)
				continue;

			strcpy(ColorTable[Color].data(), Name);
		}
	}

	if (!LGEOPath.isEmpty())
	{
		POVFile.WriteLine("#include \"lg_defs.inc\"\n#include \"lg_color.inc\"\n\n");

		for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
		{
			PieceInfo* Info = ModelParts[PartIdx].Info;

			for (int CheckIdx = 0; CheckIdx < ModelParts.GetSize(); CheckIdx++)
			{
				if (ModelParts[CheckIdx].Info != Info)
					continue;

				if (CheckIdx != PartIdx)
					break;

				auto Search = PieceTable.find(Info);

				if (Search != PieceTable.end())
				{
					const std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = Search->second;
					if (Entry.first[0])
					{
						sprintf(Line, "#include \"%s.inc\"\n", Entry.first);
						POVFile.WriteLine(Line);
					}
				}

				break;
			}
		}

		POVFile.WriteLine("\n");
	}

	for (int ColorIdx = 0; ColorIdx < gColorList.GetSize(); ColorIdx++)
	{
		lcColor* Color = &gColorList[ColorIdx];

		for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
		{
			int PartColorIdx;
			PartColorIdx = ModelParts[PartIdx].ColorIndex;

			if (PartColorIdx == ColorIdx)
			{
				if (ColorTable[ColorIdx][0])
				{
					  sprintf(Line, "#ifndef (POVColor_%d) // %s\n"
							"#declare POVColor_%d = #if (version >= 3.1) material { #end texture { %s } #if (version >= 3.1) } #end\n"
							"#end\n",
							ColorIdx, ColorTable[ColorIdx].data(), ColorIdx, ColorTable[ColorIdx].data());
				}
				else
				if (lcIsColorTranslucent(ColorIdx))
				{
					  sprintf(Line, "#ifndef (POVColor_%d) // %s\n"
							"#declare POVColor_%d = #if (version >= 3.1) material { #end texture { pigment { rgb <%f, %f, %f> filter 0.9 } finish { ambient 0.3 diffuse 0.2 reflection 0.25 phong 0.3 phong_size 60 } } #if (version >= 3.1) } #end\n"
							"#end\n",
							ColorIdx, Color->SafeName, ColorIdx, Color->Value[0], Color->Value[1], Color->Value[2]);
				}
				else
				{
					  sprintf(Line, "#ifndef (POVColor_%d) // %s\n"
							"#declare POVColor_%d = #if (version >= 3.1) material { #end texture { pigment { rgb <%f, %f, %f> } finish { ambient 0.1 phong 0.2 phong_size 20 } } #if (version >= 3.1) } #end\n"
							"#end\n",
							ColorIdx, Color->SafeName, ColorIdx, Color->Value[0], Color->Value[1], Color->Value[2]);
				}

				POVFile.WriteLine(Line);
			}
		}

		if (!ColorTable[ColorIdx][0])
			sprintf(ColorTable[ColorIdx].data(), "lc_%s", Color->SafeName);
	}

	POVFile.WriteLine("\n");

	lcArray<const char*> ColorTablePointer;
	ColorTablePointer.SetSize(NumColors);
	for (int ColorIdx = 0; ColorIdx < NumColors; ColorIdx++)
		ColorTablePointer[ColorIdx] = ColorTable[ColorIdx].data();

	for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
	{
		PieceInfo* Info = ModelParts[PartIdx].Info;
		lcMesh* Mesh = Info->GetMesh();
		std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[Info];

		if (!Mesh || Entry.first[0])
			continue;

		char Name[LC_PIECE_NAME_LEN];
		char* Ptr;

		strcpy(Name, Info->mFileName);
		while ((Ptr = strchr(Name, '-')))
			*Ptr = '_';
		while ((Ptr = strchr(Name, '.')))
			*Ptr = '_';

		sprintf(Entry.first, "lc_%s", Name);

		Mesh->ExportPOVRay(POVFile, Name, &ColorTablePointer[0]);

		sprintf(Line, "#declare lc_%s_clear = lc_%s\n\n", Name, Name);
		POVFile.WriteLine(Line);
	}

        m_Position = Camera->mPosition;
        m_Target = Camera->mTargetPosition;
        m_Up = Camera->mUpVector;
        m_Fov = Camera->m_fovy;

        writePovCamera();

	const lcModelProperties& Properties = PovGenProject->mModels[0]->GetProperties();
	sprintf(Line, "background { color rgb <%1g, %1g, %1g> }\n\n", Properties.mBackgroundSolidColor[0], Properties.mBackgroundSolidColor[1], Properties.mBackgroundSolidColor[2]);
	POVFile.WriteLine(Line);

	lcVector3 Min(FLT_MAX, FLT_MAX, FLT_MAX);
	lcVector3 Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const lcModelPartsEntry& ModelPart : ModelParts)
	{
		lcVector3 Points[8];

		lcGetBoxCorners(ModelPart.Info->GetBoundingBox(), Points);

		for (int PointIdx = 0; PointIdx < 8; PointIdx++)
		{
			lcVector3 Point = lcMul31(Points[PointIdx], ModelPart.WorldMatrix);

			m_Min = lcMin(Point, Min);
			m_Max = lcMax(Point, Max);
		}
	}

	lcVector3 Center = (m_Min + m_Max) / 2.0f;
	m_Radius = (m_Max - Center).Length() / 25.0f;
	m_Center = lcVector3(Center[1], Center[0], Center[2]) / 25.0f;

	writePovModelBounds();

	writePovLights();

	for (int PartIdx = 0; PartIdx < ModelParts.GetSize(); PartIdx++)
	{
		std::pair<char[LC_PIECE_NAME_LEN], int>& Entry = PieceTable[ModelParts[PartIdx].Info];
		int ColorIdx;

		ColorIdx = ModelParts[PartIdx].ColorIndex;
		const char* Suffix = lcIsColorTranslucent(ColorIdx) ? "_clear" : "";

		const float* f = ModelParts[PartIdx].WorldMatrix;

		if (Entry.second & LGEO_PIECE_SLOPE)
		{
			sprintf(Line, "merge {\n"
				      " object {\n"
				      "   %s%s\n"
				      "   #if (version >= 3.1) material #else texture #end { POVColor_%d }\n"
				      " }\n"
				      " object {\n"
				      "   %s_slope\n"
				      "   #if (version >= 3.1) material { #end\n"
				      "     texture {\n"
				      "         %s\n"
				      "         #if (POVQual > 1) normal { bumps 0.3 scale 0.02 } #end\n"
				      "    }\n"
				      "   #if (version >= 3.1) } #end\n"
				      " }\n"
				      " matrix <%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f>\n}\n",
				      Entry.first, Suffix, ColorIdx, Entry.first, ColorTable[ColorIdx].data(),
				      -f[5], -f[4], -f[6], -f[1], -f[0], -f[2], f[9], f[8], f[10], f[13] / 25.0f, f[12] / 25.0f, f[14] / 25.0f);
		}
		else
		{
			sprintf(Line, "object {\n %s%s\n #if (version >= 3.1) material #else texture #end { POVColor_%d }\n matrix <%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f>\n}\n",
				      Entry.first, Suffix, ColorIdx, -f[5], -f[4], -f[6], -f[1], -f[0], -f[2], f[9], f[8], f[10], f[13] / 25.0f, f[12] / 25.0f, f[14] / 25.0f);
		}

		POVFile.WriteLine(Line);
	}

	POVFile.Close();

	return true;
}

void NativePov::writePovDeclare(
	const QString &name,
	const QString &value,
	const QString &commentName /*= NULL*/)
{
	if (!commentName.isEmpty())
	{
		sprintf(m_Line, "#declare %s = %s;\t// %s\n",
			name.toLatin1().constData(),
			value.toLatin1().constData(),
			commentName.toLatin1().constData());
	}
	else
	{
		sprintf(m_Line, "#declare %s = %s;\n",
			name.toLatin1().constData(),
			value.toLatin1().constData());
	}
	m_pPOVFile->WriteLine(m_Line);
}

bool NativePov::writePovHeader(const Render::Mt &ImageType, const QString &PovGenCommand)
{
        QString Type = ImageType == Render::CSI ? "CSI" : "PLI";

        static const QString fmtDateTime("ddd MMM d hh:mm:ss yyyy");

        m_pPOVFile->WriteLine("// Generated by: " VER_PRODUCTNAME_STR " Native NativePover (C) 2018 Trevor SANDY\n");
        m_pPOVFile->WriteLine("// See: " VER_COMPANYDOMAIN_STR "\n");
        sprintf(m_Line, "// Date: %s\n", QDateTime::currentDateTime().toString(fmtDateTime).toLatin1().constData());
        m_pPOVFile->WriteLine(m_Line);
        sprintf(m_Line, "// %s Command: %s\n\n", Type.toLatin1().constData(), PovGenCommand.toLatin1().constData());
        m_pPOVFile->WriteLine(m_Line);
        m_pPOVFile->WriteLine("// This file was automatically generated from an LDraw file by " VER_PRODUCTNAME_STR "\n\n");

        m_pPOVFile->WriteLine("#version 3.6;\n\n");

        writePovDeclare("POVQual","3","Quality (0 = Bounding Box; 1 = No Refraction; 2 = Normal; 3 = Stud Logos)");
        writePovDeclare("POVStuds","1","Show studs? (1 = YES; 0 = NO)");
        writePovDeclare("POVRefls","1","Reflections? (1 = YES; 0 = NO)");
        writePovDeclare("POVShads","1","Shadows? (1 = YES; 0 = NO)");

        writePovBlankLine();

        return true;
}

void NativePov::writePovModelBounds()
{
        m_pPOVFile->WriteLine("// Model bounds information\n");
        writePovDeclare("POVMinX", QString::number(m_Min[1]));
        writePovDeclare("POVMinY", QString::number(m_Min[0]));
        writePovDeclare("POVMinZ", QString::number(m_Min[2]));
        writePovDeclare("POVMaxX", QString::number(m_Max[1]));
        writePovDeclare("POVMaxY", QString::number(m_Max[0]));
        writePovDeclare("POVMaxZ", QString::number(m_Max[2]));
        writePovDeclare("POVCenterX", QString::number(m_Center[1]));
        writePovDeclare("POVCenterY", QString::number(m_Center[0]));
        writePovDeclare("POVCenterZ", QString::number(m_Center[2]));
        writePovDeclare("POVCenter", "<POVCenterX,POVCenterY,POVCenterZ>");
        writePovDeclare("POVRadius", QString::number(m_Radius));

        writePovBlankLine();
}

void NativePov::writePovCamera()
{
	m_pPOVFile->WriteLine("// Camera Settings\n");
	writePovDeclare("POVCameraLoc", QString("< %1,%2,%3 >").arg(m_Position[1] / 25.0f).arg(m_Position[0] / 25.0f).arg(m_Position[2] / 25.0f));
	writePovDeclare("POVCameraLookAt", QString("< %1,%2,%3 >").arg(m_Target[1] / 25.0f).arg(m_Target[0] / 25.0f).arg(m_Target[2] / 25.0f));
	writePovDeclare("POVCameraSky", QString("< %1,%2,%3 >").arg(m_Up[1]).arg(m_Up[0]).arg(m_Up[2]));
	writePovDeclare("POVCameraFov", QString::number(m_Fov));

	writePovBlankLine();

        m_pPOVFile->WriteLine("// Camera\n");
        m_pPOVFile->WriteLine("#ifndef (POVSkipCamera)\n"
                              "camera {\n"
                              "\t#declare POVCamAspect = image_width/image_height;\n"
                              "\tperspective\n"
                              "\tright x * image_width / image_height\n"
                              "\tlocation POVCameraLoc\n"
                              "\tsky POVCameraSky\n"
                              "\tlook_at POVCameraLookAt\n"
                              "\tangle POVCameraFov * POVCamAspect\n"
                              "}\n"
                              "#end\n\n");

//        m_pPOVFile->WriteLine("#ifndef (POVSkipCamera)\n"
//                              "camera {\n"
//                              "\t#declare POVCamAspect = image_width/image_height;\n"
//                              "\tperspective\n"
//                              "\tright x * image_width / image_height\n"
//                              "\tlocation POVCameraLoc\n"
//                              "\tsky POVCameraSky\n"
//                              "\tlook_at POVCameraLookAt\n"
//                              "\tangle POVCameraFov * POVCamAspect\n"
//                              "}\n"
//                              "#end\n\n");

        writePovBlankLine();
}

void NativePov::writePovLight(float lat, float lon, int num)
{
        PovVector3 lightVector(0.0f, 0.0f, 2.0f);
        PovVector3 lightLoc;
        float latMatrix[16];
        float lonMatrix[16];
        float lightMatrix[16];
        float tempMatrix[16];
        float flipMatrix[16];
        float latRad = (float)deg2rad(lat);
        float lonRad = (float)deg2rad(-lon);

        PovVector3::initIdentityMatrix(latMatrix);
        PovVector3::initIdentityMatrix(lonMatrix);
        PovVector3::initIdentityMatrix(flipMatrix);
        latMatrix[5] = (float)cos(latRad);
        latMatrix[6] = (float)-sin(latRad);
        latMatrix[9] = (float)sin(latRad);
        latMatrix[10] = (float)cos(latRad);
        lonMatrix[0] = (float)cos(lonRad);
        lonMatrix[2] = (float)sin(lonRad);
        lonMatrix[8] = (float)-sin(lonRad);
        lonMatrix[10] = (float)cos(lonRad);
        flipMatrix[5] = -1.0f;
        flipMatrix[10] = -1.0f;
        multMatrix(lonMatrix, latMatrix, tempMatrix);
        multMatrix(flipMatrix, tempMatrix, lightMatrix);
        lightVector.transformPoint(lightMatrix, lightLoc);
        sprintf(m_Line,
                "#ifndef (POVSkipLight_%d)\n"
                "light_source {\t// Latitude,Longitude: %s,%s,POVRadius*2\n"
                "	<%s*POVRadius,%s*POVRadius,%s*POVRadius> + POVCenter\n"
                "	color rgb <1,1,1>\n"
                "}\n"
                "#end\n",
                num,
                QString::number(lat).toLatin1().constData(),
                QString::number(lon).toLatin1().constData(),
                QString::number(lightLoc[0]).toLatin1().constData(),
                QString::number(lightLoc[1]).toLatin1().constData(),
                QString::number(lightLoc[2]).toLatin1().constData());

        m_pPOVFile->WriteLine(m_Line);

        writePovBlankLine();
}

void NativePov::writePovLights()
{
       m_pPOVFile->WriteLine("// Lights\n");
       writePovLight(45.0, 0.0, 1);
       writePovLight(30.0, 120.0, 2);
       writePovLight(60.0, -120.0, 3);
}

void NativePov::writePovBlankLine()
{
       m_pPOVFile->WriteLine("\n");
}

/**************************************************
  Utility functions:
***************************************************/

void NativePov::multMatrix(const float* left, const float* right, float* result)
{
	result[0] = left[0] * right[0] + left[4] * right[1] +
		left[8] * right[2] + left[12] * right[3];
	result[1] = left[1] * right[0] + left[5] * right[1] +
		left[9] * right[2] + left[13] * right[3];
	result[2] = left[2] * right[0] + left[6] * right[1] +
		left[10] * right[2] + left[14] * right[3];
	result[3] = left[3] * right[0] + left[7] * right[1] +
		left[11] * right[2] + left[15] * right[3];
	result[4] = left[0] * right[4] + left[4] * right[5] +
		left[8] * right[6] + left[12] * right[7];
	result[5] = left[1] * right[4] + left[5] * right[5] +
		left[9] * right[6] + left[13] * right[7];
	result[6] = left[2] * right[4] + left[6] * right[5] +
		left[10] * right[6] + left[14] * right[7];
	result[7] = left[3] * right[4] + left[7] * right[5] +
		left[11] * right[6] + left[15] * right[7];
	result[8] = left[0] * right[8] + left[4] * right[9] +
		left[8] * right[10] + left[12] * right[11];
	result[9] = left[1] * right[8] + left[5] * right[9] +
		left[9] * right[10] + left[13] * right[11];
	result[10] = left[2] * right[8] + left[6] * right[9] +
		left[10] * right[10] + left[14] * right[11];
	result[11] = left[3] * right[8] + left[7] * right[9] +
		left[11] * right[10] + left[15] * right[11];
	result[12] = left[0] * right[12] + left[4] * right[13] +
		left[8] * right[14] + left[12] * right[15];
	result[13] = left[1] * right[12] + left[5] * right[13] +
		left[9] * right[14] + left[13] * right[15];
	result[14] = left[2] * right[12] + left[6] * right[13] +
		left[10] * right[14] + left[14] * right[15];
	result[15] = left[3] * right[12] + left[7] * right[13] +
		left[11] * right[14] + left[15] * right[15];
}

void PovVector3::transformPoint(const float *matrix, lcVector3 &newPoint) const
{
/*
        x' = a*x + b*y + c*z + X
        y' = d*x + e*y + f*z + Y
        z' = g*x + h*y + i*z + Z
*/
	newPoint[0] = matrix[0]*x + matrix[4]*y + matrix[8]*z + matrix[12];
	newPoint[1] = matrix[1]*x + matrix[5]*y + matrix[9]*z + matrix[13];
	newPoint[2] = matrix[2]*x + matrix[6]*y + matrix[10]*z + matrix[14];
}

float PovVector3::sm_identityMatrix[16] =
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

PovVector3::PovVector3()
{
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}

PovVector3::PovVector3(const float _x, const float _y, const float _z)
  : lcVector3(_x,_y,_z) {}

void PovVector3::initIdentityMatrix(float *matrix)
{
        memcpy(matrix, sm_identityMatrix, sizeof(sm_identityMatrix));
}
