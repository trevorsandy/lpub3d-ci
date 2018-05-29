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
          QString Type = Options.ImageType == Render::CSI ? "CSI" : "PLI";

          loadPovSettings();

          // Options
          m_InputFileName   = Options.InputFileName;
          m_OutputFileName  = Options.OutputFileName;
          m_PovGenCommand   = Options.PovGenCommand;
          m_ImageType       = Options.ImageType;
          m_height          = Options.ImageWidth;
          m_width           = Options.ImageHeight;
          m_Latitude        = Options.Latitude;
          m_Longitude       = Options.Longitude;
          m_CameraDistance  = Options.CameraDistance;
          m_TransBackground = Options.TransBackground;
          m_Orthographic    = Options.Orthographic;

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

         // load requireds
         if (m_xmlMap)
           loadPovLDrawPovXml();

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

	if (!writePovHeader())
	{
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not write %1 pov file header.").arg(Type));
		delete PovGenProject;
		return false;
	}

	// Model bounds
	writePovModelBounds();

        // Camera settings
        lcCamera* Camera = gMainWindow->GetActiveView()->mCamera;
        Camera->SetAngles(Options.Latitude,Options.Longitude);
        Camera->Zoom(Options.CameraDistance,PovGenProject->mModels[0]->GetCurrentStep(),true);

        m_Position = Camera->mPosition;
        m_Target = Camera->mTargetPosition;
        m_Up = Camera->mUpVector;
        m_fov = Camera->m_fovy;

        writePovCameraSettings();

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

	char Line[1024];

        if (m_topInclude.size() > 0)
        {
                sprintf(Line, "#include \"%s\"\n\n", m_topInclude.toLatin1().constData());
                POVFile.WriteLine(Line);
        }

        writePovCamera();

	if (!writePovLights())
	{
		delete PovGenProject;
		return false;
	}

	writeSeamMacro();

	const lcModelProperties& Properties = PovGenProject->mModels[0]->GetProperties();
	sprintf(Line, "background { color rgb <%1g, %1g, %1g> }\n\n", Properties.mBackgroundSolidColor[0], Properties.mBackgroundSolidColor[1], Properties.mBackgroundSolidColor[2]);
	POVFile.WriteLine(Line);

	POVFile.WriteLine("#declare lg_quality = POVQual;\n"
			  "#if (lg_quality = 3)\n"
			  "#declare lg_quality = 4;\n"
			  "#end\n\n");

	writePovDeclare("lg_studs","POVStuds");

	writePovBlankLine();

	if (!LGEOPath.isEmpty())
	{
		// Get lg_elements.lst items and enter into piece table
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

		// Get lg_colors.lst file and enter into Color table
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

	// Declare part (s)
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

	// Old location fof model bounds

	// Define csi/pli objects
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

void NativePov::writePovDeclare(
	const char *name,
	const std::string &value,
	const char *commentName /*= NULL*/)
{
	if (commentName != NULL)
	{
		sprintf(m_Line, "#declare %s = %s;\t// %s\n", name, value.c_str(),commentName);
	}
	else
	{
		sprintf(m_Line, "#declare %s = %s;\n",  name, value.c_str());
	}
	m_pPOVFile->WriteLine(m_Line);
}

void NativePov::writePovDeclare(
	const char *name,
	const char *value,
	const char *commentName /*= NULL*/)
{
	writePovDeclare(name, std::string(value), commentName);
}

void NativePov::writePovDeclare(
	const char *name,
	double value,
	const char *commentName /*= NULL*/)
{
	writePovDeclare(name, QString::number(value).toLatin1().constData(), commentName);
}

void NativePov::writePovDeclare(
	const char *name,
	float value,
	const char *commentName /*= NULL*/)
{
	writePovDeclare(name, QString::number(value).toLatin1().constData(), commentName);
}

void NativePov::writePovDeclare(
	const char *name,
	long value,
	const char *commentName /*= NULL*/)
{
	writePovDeclare(name, QString::number(value).toLatin1().constData(), commentName);
}

void NativePov::writePovDeclare(
	const char *name,
	bool value,
	const char *commentName /*= NULL*/)
{
	writePovDeclare(name, value ? 1l : 0l, commentName);
}

bool NativePov::writePovHeader()
{
        QString Type = m_ImageType == Render::CSI ? "CSI" : "PLI";

        static const QString fmtDateTime("ddd MMM d hh:mm:ss yyyy");

        sprintf(m_Line, "// Generated by: %s v%s %s\n"
                        "// See: %s\n"
                        "// Date: %s\n"
                        "// %s Command: %s\n\n",
                m_appName.c_str(),
                m_appVersion.c_str(),
                m_appCopyright.c_str(),
                m_appUrl.c_str(),
                QDateTime::currentDateTime().toString(fmtDateTime).toLatin1().constData(),
                Type.toLatin1().constData(), m_PovGenCommand.toLatin1().constData()
                );
        m_pPOVFile->WriteLine(m_Line);

        sprintf(m_Line, "// This file was automatically generated from an LDraw file by %s.\n\n", m_renderer.c_str());
        m_pPOVFile->WriteLine(m_Line);

//        const char *author = m_pTopModel->getAuthor();
//        if (author != NULL)
//	{
//		sprintf(m_Line, "// LDraw File Author: %s\n", author);
//		m_pPOVFile->WriteLine(m_Line);
//	}

        QString fileVersion;
        switch (m_fileVersion)
        {
        case 0:
          fileVersion = POV_FILE_VER_01;
          break;
        case 1:
          fileVersion = POV_FILE_VER_02;
          break;
        case 2:
          fileVersion = POV_FILE_VER_03;
          break;
        default:
          fileVersion = POV_FILE_VER_01;
        }
        sprintf(m_Line, "#version %g;\n\n", fileVersion.toDouble());
        m_pPOVFile->WriteLine(m_Line);

	writePovDeclare("POVQual",m_quality,"Quality (0 = Bounding Box; 1 = No Refraction; 2 = Normal; 3 = Stud Logo)");
	writePovDeclare("POVSW",m_seamWidth, "Seam Width (0 for no seams)");
	writePovDeclare("POVStuds",!m_hideStuds,"Show studs? (1 = YES; 0 = NO)");
	writePovDeclare("POVRefls",m_refls,"Reflections? (1 = YES; 0 = NO)");
	writePovDeclare("POVShads",m_shads,"Shadows? (1 = YES; 0 = NO)");
	if (m_edges)
	{
		writePovDeclare("POVSkipEdges", false, "Skip Edges? (1 = YES; 0 = NO)");
	}
	writePovBlankLine();
	writePovDeclare("POVAmb", m_ambient);
	writePovDeclare("POVDif", m_diffuse);
	writePovDeclare("POVRefl", m_refl);
	writePovDeclare("POVPhong", m_phong);
	writePovDeclare("POVPhongS", m_phongSize);
	writePovDeclare("POVTRefl", m_transRefl);
	writePovDeclare("POVTFilt", m_transFilter);
	writePovDeclare("POVIoR", m_transIoR);
	writePovDeclare("POVRubberRefl", m_rubberRefl);
	writePovDeclare("POVRubberPhong", m_rubberPhong);
	writePovDeclare("POVRubberPhongS", m_rubberPhongSize);
	writePovDeclare("POVChromeRefl", m_chromeRefl);
	writePovDeclare("POVChromeBril", m_chromeBril);
	writePovDeclare("POVChromeSpec", m_chromeSpec);
	writePovDeclare("POVChromeRough", m_chromeRough);
	writePovDeclare("POVIPov", m_inlinePov, "Use inline POV code from LDraw file? (1 = YES; 0 = NO)");
	if (m_edges)
	{
		writePovDeclare("POVEdgeRad", m_edgeRadius);
		writePovDeclare("POVEdgeR", "0.0");
		writePovDeclare("POVEdgeG", "0.0");
		writePovDeclare("POVEdgeB", "0.0");
	}
	if (m_xmlMap)
	{
		writePovDeclare("POVOrigVer", "version", "DO NOT MODIFY");
	}
	writePovBlankLine();

	return true;
}

void NativePov::writePovModelBounds()
{
        m_pPOVFile->WriteLine("// Model bounds information\n");

        getPovModelBoundingBox();
        getPovModelRadius();
        getPovModelCenter();
        getPovModelFov();

        // LDView Settings
        getPovModelBounds();

        writePovDeclare("POVMinX", m_boundingMin[0]);
        writePovDeclare("POVMinY", m_boundingMin[1]);
        writePovDeclare("POVMinZ", m_boundingMin[2]);
        writePovDeclare("POVMaxX", m_boundingMax[0]);
        writePovDeclare("POVMaxY", m_boundingMax[1]);
        writePovDeclare("POVMaxZ", m_boundingMax[2]);
        writePovDeclare("POVCenterX", m_center[0]);
        writePovDeclare("POVCenterY", m_center[1]);
        writePovDeclare("POVCenterZ", m_center[2]);
        writePovDeclare("POVCenter", "<POVCenterX,POVCenterY,POVCenterZ>");
        writePovDeclare("POVRadius", m_radius);

        writePovBlankLine();
}

void NativePov::writePovCameraSettings()
{
        m_pPOVFile->WriteLine("// Camera Settings\n");
        // LeoCAD Settings
        std::string cameraLocString = QString(QString("< %1,%2,%3 >")
                                              .arg(m_Position.y / 25.0f)
                                              .arg(m_Position.x / 25.0f)
                                              .arg(m_Position.z / 25.0f)).toLatin1().constData();
        std::string cameraLookAtString = QString(QString("< %1,%2,%3 >")
                                              .arg(m_Target.y / 25.0f)
                                              .arg(m_Target.x / 25.0f)
                                              .arg(m_Target.z / 25.0f)).toLatin1().constData();
        std::string cameraSkyString = QString(QString("< %1,%2,%3 >")
                                              .arg(m_Up.y)
                                              .arg(m_Up.x)
                                              .arg(m_Up.z)).toLatin1().constData();
        std::string cameraFovString = QString("%1")
                                              .arg(m_fov).toLatin1().constData();
        // LDView Settings
        getCameraStrings(cameraLocString, cameraLookAtString, cameraSkyString, cameraFovString);

	writePovDeclare("POVCameraLoc", cameraLocString, "Camera Location vector (Position)");
	writePovDeclare("POVCameraLookAt", cameraLookAtString, "Camera look-at point vector (Target)");
	writePovDeclare("POVCameraSky", cameraSkyString, "Camera sky vector (<0,-1,0> will usually work for standard \"up\")");
	writePovDeclare("POVCameraFov", cameraFovString, "Camera field of view float (Fov)");

	writePovBlankLine();
}

void NativePov::writePovCamera()
{
        m_pPOVFile->WriteLine("// Camera\n");
        sprintf(m_Line, "#ifndef (POVSkipCamera)\n"
                        "camera {\n"
                        "\t#declare POVCamAspect = %s;\n"
                        "\tlocation POVCameraLoc\n"
                        "\tlook_at POVCameraLookAt\n"
                        "\tsky POVCameraSky\n"
                        "\tangle POVCameraFov\n"
                        "\tright POVCamAspect * < 1,0,0 >\n"      // For LeoCAD mesh, do not flip x coordinate
                        "}\n"
                        "#end\n\n",
               getPovAspectRatio().toLatin1().constData());

// LeoCAD camera string
//       sprintf(m_Line, "#ifndef (POVSkipCamera)\n"
//                       "camera {\n"
//                       "\t#declare POVCamAspect = image_width/image_height;\n"
//                       "\tperspective\n"
//                       "\tlocation POVCameraLoc\n"
//                       "\tlook_at POVCameraLookAt\n"
//                       "\tsky POVCameraSky\n"
//                       "\tangle POVCameraFov * POVCamAspect\n"
//                       "\tright x * image_width/image_height\n"
//                       "}\n"
//                       "#end\n\n");

        m_pPOVFile->WriteLine(m_Line);

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
        // Flip to LDraw coordinate system      - For LeoCAD mesh, do not flip lights
//        flipMatrix[5] = -1.0f;
//        flipMatrix[10] = -1.0f;
        PovVector3::multMatrix(lonMatrix, latMatrix, tempMatrix);
        PovVector3::multMatrix(flipMatrix, tempMatrix, lightMatrix);
        lightVector.transformPoint(lightMatrix, lightLoc);
        sprintf(m_Line,
                "#ifndef (POVSkipLight_%d)\n"
                "light_source {\t// Latitude %s, Longitude %s, POVRadius*2\n"
                "	<%s*POVRadius,%s*POVRadius,%s*POVRadius> + POVCenter\n"
                "	color rgb <1,1,1>\n"
                "}\n"
                "#end\n",
                num,
                QString::number(lat).toLatin1().constData(),
                QString::number(lon).toLatin1().constData(),
                QString::number(lightLoc.x).toLatin1().constData(),
                QString::number(lightLoc.y).toLatin1().constData(),
                QString::number(lightLoc.z).toLatin1().constData());
        m_pPOVFile->WriteLine(m_Line);

        writePovBlankLine();
}

bool NativePov::writePovLights()
{
       m_pPOVFile->WriteLine("// Lights\n");
       QStringList lights = m_lights.split(",", QString::SkipEmptyParts);
       bool fieldOk[3];
       for (QString light: lights){
              QStringList lightEntryFields = light.split(" ",QString::SkipEmptyParts);
              float latitude = QString("%1").arg(lightEntryFields.at(2)).toFloat(&fieldOk[0]);
              float longitude = QString("%1").arg(lightEntryFields.at(4)).toFloat(&fieldOk[1]);
              int lightNumber = QString("%1").arg(lightEntryFields.at(0)).replace(".","").toInt(&fieldOk[2]);

              if (fieldOk[0] && fieldOk[1] && fieldOk[2])
              {
                     writePovLight(latitude,longitude,lightNumber);
              }
              else
              {
                     emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Could not create light '%1'.").arg(light));
                     return false;
              }
       }
       return true;
}

void NativePov::writePovBlankLine()
{
       m_pPOVFile->WriteLine("\n");
}

void NativePov::loadPovSettings()
{
	// Initilization settings - this could be here or in constructor
	m_width  = 4.0f;
	m_height = 3.0f;
	m_fov    = 25.0f;
	m_xPan   = 0.0f;
	m_yPan   = 0.0f;
	m_radius = 0.0f;
	char buildDate[5] =
	{
		__DATE__[7],
		__DATE__[8],
		__DATE__[9],
		__DATE__[10],
		0
	};

	m_appCopyright = "Copyright (C) ";
	m_appCopyright += buildDate;
	m_appCopyright += " Trevor SANDY";
	m_appName = VER_FILEDESCRIPTION_STR;
	m_renderer = "LPub3D Native POV file Generator";
	m_appUrl = VER_COMPANYDOMAIN_STR;
	m_appVersion = VER_PRODUCTVERSION_STR "." VER_REVISION_STR "." VER_BUILD_STR;

	// Initialize m_rotationMatrix
	float flipMatrix[16];
	float tempMatrix[16];
	PovVector3::initIdentityMatrix(flipMatrix);
	// Flip to LDraw coordinate system      - For LeoCAD mesh, do not flip rotationMatrix
//	flipMatrix[5]  = -1.0f;
//	flipMatrix[10] = -1.0f;
	// 2/3 view (30 degrees latitude, 45 degrees longitude)
	tempMatrix[0]  = (float)(sqrt(2.0) / 2.0);
	tempMatrix[1]  = (float)(sqrt(2.0) / 4.0);
	tempMatrix[2]  = (float)(-sqrt(1.5) / 2.0);
	tempMatrix[3]  = 0.0f;
	tempMatrix[4]  = 0.0f;
	tempMatrix[5]  = (float)(sin(M_PI / 3.0));
	tempMatrix[6]  = 0.5f;
	tempMatrix[7]  = 0.0f;
	tempMatrix[8]  = (float)(sqrt(2.0) / 2.0);
	tempMatrix[9]  = (float)(-sqrt(2.0) / 4.0);
	tempMatrix[10] = (float)(sqrt(1.5) / 2.0);
	tempMatrix[11] = 0.0f;
	tempMatrix[12] = 0.0f;
	tempMatrix[13] = 0.0f;
	tempMatrix[14] = 0.0f;
	tempMatrix[15] = 1.0f;
	PovVector3::multMatrix(flipMatrix, tempMatrix, m_rotationMatrix);

	// Parameter settings
	m_quality = Preferences::quality;
	m_refls = Preferences::reflections;
	m_shads = Preferences::shadows;
	m_findReplacements = Preferences::findReplacements;
	m_xmlMap = Preferences::xmlMap;
	m_xmlMapPath = Preferences::xmlMapPath;
	m_inlinePov = Preferences::inlinePov;
	m_smoothCurves = Preferences::smoothCurves;
	m_hideStuds = Preferences::hideStuds;
	m_unmirrorStuds = Preferences::unmirrorStuds;
	m_selectedAspectRatio = Preferences::selectedAspectRatio;
	m_customAspectRatio = Preferences::customAspectRatio;
	m_edgeRadius = Preferences::edgeRadius;
	m_ambient = Preferences::ambient;
	m_diffuse = Preferences::diffuse;
	m_lights = Preferences::lights;
	m_refl = Preferences::refl;
	m_phong = Preferences::phong;
	m_phongSize = Preferences::phongSize;
	m_transRefl = Preferences::transRefl;
	m_transFilter = Preferences::transFilter;;
	m_transIoR = Preferences::transIoR;
	m_rubberRefl = Preferences::rubberRefl;
	m_rubberPhong = Preferences::rubberPhong;
	m_rubberPhongSize = Preferences::rubberPhongSize;
	m_chromeRefl = Preferences::chromeRefl;
	m_chromeBril = Preferences::chromeBril;
	m_chromeSpec = Preferences::chromeSpecular;
	m_chromeRough = Preferences::chromeRoughness;
	m_fileVersion = Preferences::fileVersion;

	m_primSub = Preferences::primitiveSubstitution;
	m_seamWidth = Preferences::seamWidth;
	m_conditionalEdges = Preferences::conditionalEdgeLines;
	m_edges = gApplication->mPreferences.mDrawEdgeLines;

	if (!Preferences::topInclude.isEmpty())
	{
		m_topInclude = Preferences::topInclude;
	}
	else
	{
		m_topInclude = "";
	}

	if (!Preferences::bottomInclude.isEmpty())
	{
		m_bottomInclude = Preferences::bottomInclude;
	}
	else
	{
		m_bottomInclude = "";
	}
}

void NativePov::getCameraStrings(
	std::string &locationString,
	std::string &lookAtString,
	std::string &skyString,
	std::string &fovString)
{
// Steps to define camera settings:
/* NativePov::CreateNativePovFile(const NativeOptions& Options) - main
 * NativePov::loadPovSettings()             - init vars, set m_rotationMatrix [Might need to chang this default]
 * NativePov::writePovModelBounds()         - set bounding, radius, center and fov
 *      NativePov::getPovModelBoundingBox() - use LeoCAD function
 *      NativePov::getPovModelRadius()      - 25.0 divisor
 *      NativePov::getPovModelCenter()      - 25.0 divisor
 *      NativePov::getPovModelFov()         - std calc
 *      NativePov::getPovModelBounds()      - process lat, lon, cd, AutoCamera, set m_camera
 * NativePov::writePovCameraSettings()      - set loc, lookat, sky, and fov
 *      NativePov::getCameraStrings(cameraLocString, cameraLookAtString, cameraSkyString, cameraFovString)
 */
	float tmpMatrix[16];
	float matrix[16];
	float centerMatrix[16];
	float positionMatrix[16];
	float cameraMatrix[16];
	float otherMatrix[16] = {1,0,0,0,0,-1,0,0,0,0,-1,0,0,0,0,1};
	char tmpString[1024];
	PovVector3 directionVector = PovVector3(0.0f, 0.0f, 1.0f);
	PovVector3 locationVector;
	PovVector3 lookAtVector;
	PovVector3 upVector = PovVector3(0.0f, -1.0f, 0.0f);
	double direction[3];
	double up[3];
	double location[3];
	LDLFacing facing;
	double lookAt[3];
	double tempV[3];

	PovVector3 cameraPosition = m_camera.getPosition();

	PovVector3::initIdentityMatrix(positionMatrix);
	positionMatrix[12] =  cameraPosition[0] - m_xPan;
	positionMatrix[13] = -cameraPosition[1] + m_yPan;
	positionMatrix[14] = -cameraPosition[2];
	PovVector3::initIdentityMatrix(centerMatrix);
	centerMatrix[12] = m_center[0];
	centerMatrix[13] = m_center[1];
	centerMatrix[14] = m_center[2];
	PovVector3::multMatrix(otherMatrix, m_rotationMatrix, tmpMatrix);
	PovVector3::invertMatrix(tmpMatrix, cameraMatrix);
	PovVector3::multMatrix(centerMatrix, cameraMatrix, tmpMatrix);
	PovVector3::multMatrix(tmpMatrix, positionMatrix, matrix);

	facing = m_camera.getFacing();
	facing[0] = -facing[0];
	facing.getInverseMatrix(cameraMatrix);
	PovVector3::multMatrix(matrix, cameraMatrix, tmpMatrix);
	memcpy(matrix, tmpMatrix, sizeof(matrix));
	cleanupFloats(matrix);
	locationVector = PovVector3(matrix[12], matrix[13], matrix[14]);
	location[0] = (double)matrix[12];
	location[1] = (double)matrix[13];
	location[2] = (double)matrix[14];
	cleanupFloats(matrix);
	// Note that the location accuracy isn't nearly as important as the
	// directional accuracy, so we don't have to re-do this string prior
	// to putting it on the clipboard in the POV code copy.
	sprintf(tmpString, "< %s,%s,%s >",
		QString::number(location[0] / 300000.0f, 'f', 4).toLatin1().constData(),                // Temp setting for LeoCAD mesh calc
		QString::number(location[1] / 300000.0f, 'f', 4).toLatin1().constData(),
		QString::number(location[2] / 300000.0f, 'f', 4).toLatin1().constData());
	locationString = tmpString;

	matrix[12] = matrix[13] = matrix[14] = 0.0f;
	directionVector = directionVector.transformPoint(matrix);
	upVector = upVector.transformPoint(matrix);
	// Grab the values prior to normalization.  That will make the
	// normalization more accurate in double precision.
	directionVector.upConvert(direction);
	lookAtVector = locationVector + directionVector *
		locationVector.Length();
	upVector.upConvert(up);
	directionVector = directionVector.normalize();
	upVector = upVector.normalize();
	cleanupFloats(directionVector, 3);
	cleanupFloats(upVector, 3);
	PovVector3::doubleNormalize(up);
	PovVector3::doubleNormalize(direction);
	PovVector3::doubleMultiply(direction, tempV,
	PovVector3::doubleLength(location));
	PovVector3::doubleAdd(location, tempV, lookAt);
	// Re-do the strings with higher accuracy, so they'll be
	// accepted by POV-Ray.
	sprintf(tmpString, "< %s,%s,%s >",
		QString::number(up[0] / 25.0f, 'f', 20).toLatin1().constData(),        // For LeoCAD mesh, use x, z, y [0, 2, 1] coordinates and 25.0f divisor
		QString::number(up[2] / 25.0f, 'f', 20).toLatin1().constData(),
		QString::number(up[1] / 25.0f, 'f', 20).toLatin1().constData());
	skyString = tmpString;
	sprintf(tmpString, "< %s,%s,%s >",
		QString::number(lookAt[0], 'f', 20).toLatin1().constData(),
		QString::number(lookAt[1], 'f', 20).toLatin1().constData(),
		QString::number(lookAt[2], 'f', 20).toLatin1().constData());
	lookAtString = tmpString;
	sprintf(tmpString, "%s",
		QString::number(getPovModelFov(), 'f', 6).toLatin1().constData());
	fovString = tmpString;
}

QString NativePov::getPovAspectRatio()
{
      switch (m_selectedAspectRatio)
      {
      case 0:
              return "image_width/image_height";
      case 1:
              m_width = 5;
              m_height = 4;
              return "5/4";
      case 2:
              m_width = 3;
              m_height = 2;
              return "3/2";
      case 3:
              m_width = 5;
              m_height = 3;
              return "5/3";
      case 4:
              m_width = 16;
              m_height = 9;
              return "16/9";
      case 5:
              m_width = 235;
              m_height = 100;
              return "2.35";
      case 6:
              {
                  return QString("%1/%2").arg(QString::number(m_width))
                                         .arg(QString::number(m_height));
              }
      case 7:
              m_width = m_customAspectRatio;
              m_height = 1.0f;
              return QString("%1/%2").arg(QString::number(m_width))
                                     .arg(QString::number(m_height));
      default:
              m_width = 4;
              m_height = 3;
              return "4/3";
      }
}

void NativePov::getPovModelBounds()
{
       char cameraGlobe[1024];
       sprintf(cameraGlobe,"%f,%f,%f",m_Latitude,m_Longitude,m_CameraDistance);

       LDLCamera camera;
       LDLAutoCamera *pAutoCamera = new LDLAutoCamera;

       //pAutoCamera->setModel(pMainModel);  // rem this
       pAutoCamera->setModelCenter(m_center);
       pAutoCamera->setRotationMatrix(m_rotationMatrix);
       pAutoCamera->setCamera(camera);
       pAutoCamera->setCameraGlobe(cameraGlobe);
       //pAutoCamera->setDistanceMultiplier(distanceMultiplier);
       // Width and height are only needed for aspect ratio, not
       // absolute size.
       pAutoCamera->setWidth(m_width);
       pAutoCamera->setHeight(m_height);
       pAutoCamera->setFov(m_fov);
       pAutoCamera->setScanConditionalControlPoints(false);

       pAutoCamera->zoomToFit();
       m_camera = pAutoCamera->getCamera();
       pAutoCamera->release();

}

void NativePov::getPovModelBoundingBox()
{
        lcVector3 Min(FLT_MAX, FLT_MAX, FLT_MAX);
        lcVector3 Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const lcModelPartsEntry& ModelPart : ModelParts)
        {
                lcVector3 Points[8];

                lcGetBoxCorners(ModelPart.Info->GetBoundingBox(), Points);

                for (int PointIdx = 0; PointIdx < 8; PointIdx++)
                {
                        lcVector3 Point = lcMul31(Points[PointIdx], ModelPart.WorldMatrix);

                        Min = lcMin(Point, Min);
                        Max = lcMax(Point, Max);
                }
        }
        m_boundingMin = Min;
        m_boundingMax = Max;
}

void NativePov::getPovModelRadius() // based on LDModel::calcMaxRadius
{
	// LDView Settings
	float matrix[16];
	PovVector3::initIdentityMatrix(matrix);
	m_radius = 0;
	m_center = (m_boundingMin + m_boundingMax) / 2.0f;

	scanPoints(this,
		  (LDLScanPointCallback)&scanRadiusSquaredPoint, matrix);
	m_radius = (float)sqrt(m_radius) / 25.0f;                              // For LeoCAD mesh, use 25.0f divisor

	// LeoCAD Settings
//	m_radius = (m_boundingMax - m_center).Length() / 25.0f;
}

void NativePov::getPovModelCenter()
{
	lcVector3 Center = (m_boundingMin + m_boundingMax) / 2.0f;
	// LDView Settings
	//m_center = PovVector3(Center.x, Center.y, Center.z);
	m_center = PovVector3(Center.y, Center.x, Center.z) / 25.0f;          // For LeoCAD mesh, use y, x, z [1, 0, 2] coordinates and 25.0f divisor
}

float NativePov::getPovModelFov()
{
	// LDView Setting
	if (m_width > m_height)
	{
		return (float)(2.0 * rad2deg(atan(tan(deg2rad(m_fov / 2.0)) *
			(double)m_width / (double)m_height)));
	}

	return m_fov;  // default = 25.0f
}

void NativePov::scanRadiusSquaredPoint(
	const PovVector3 &point,
	bool conditionalLine)
{
	if (!conditionalLine)
	{
		m_radius = (m_center - point).LengthSquared();
	}
}

void NativePov::scanPoints(
	POVObject *scanner,
	LDLScanPointCallback scanPointCallback,
	const float *matrix) const
{
	PovVector3 boxPoints[8];
	PovVector3 point;
	int i;

	boxPoints[0] = m_boundingMin;
	boxPoints[4] = m_boundingMax;
	// Bottom square
	boxPoints[1] = boxPoints[0];
	boxPoints[1][0] = boxPoints[4][0];
	boxPoints[2] = boxPoints[0];
	boxPoints[2][1] = boxPoints[4][1];
	boxPoints[3] = boxPoints[4];
	boxPoints[3][2] = boxPoints[0][2];
	// Top square
	boxPoints[5] = boxPoints[4];
	boxPoints[5][0] = boxPoints[0][0];
	boxPoints[6] = boxPoints[4];
	boxPoints[6][1] = boxPoints[0][1];
	boxPoints[7] = boxPoints[0];
	boxPoints[7][2] = boxPoints[4][2];
	for (i = 0; i < 8; i++)
	{
		boxPoints[i].transformPoint(matrix, point);
		((*scanner).*scanPointCallback)(point, false);
	}
}

// NOTE: static function
void NativePov::cleanupFloats(float *array, int count /*= 16*/)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (fabs(array[i]) < 1e-6)
		{
			array[i] = 0.0f;
		}
	}
}

// Seam and edge functions
void NativePov::writeSeamMacro()
{
	m_pPOVFile->WriteLine("\n#macro POVSeamMatrix(Width, Height, Depth, CenterX, CenterY, CenterZ)\n"
			      "#local aw = 0;\n"
			      "#local ah = 0;\n"
			      "#local ad = 0;\n"
			      "#local ax = 0;\n"
			      "#local ay = 0;\n"
			      "#local az = 0;\n"
			      "#if (Width != 0)\n"
			      "#local aw = 1-POVSW/Width;\n"
			      "#end\n"
			      "#if (Height != 0)\n"
			      "#local ah = 1-POVSW/Height;\n"
			      "#end\n"
			      "#if (Depth != 0)\n"
			      "#local ad = 1-POVSW/Depth;\n"
			      "#end\n"
			      "#if (Width != 0 & CenterX != 0)\n"
			      "#local ax = POVSW/(Width / CenterX);\n"
			      "#end\n"
			      "#if (Height != 0 & CenterY != 0)\n"
			      "#local ay = POVSW/(Height / CenterY);\n"
			      "#end\n"
			      "#if (Depth != 0 & CenterZ != 0)\n"
			      "#local az = POVSW/(Depth / CenterZ);\n"
			      "#end\n"
			      "#if (aw <= 0)\n"
			      "#local aw = 1;\n"
			      "#local ax = 0;\n"
			      "#end\n"
			      "#if (ah <= 0)\n"
			      "#local ah = 1;\n"
			      "#local ay = 0;\n"
			      "#end\n"
			      "#if (ad <= 0)\n"
			      "#local ad = 1;\n"
			      "#local az = 0;\n"
			      "#end\n"
			      "matrix <aw,0,0,0,ah,0,0,0,ad,ax,ay,az>\n"
			      "#end\n\n");
}

// Mapping functions
void NativePov::loadXmlMatrices(TiXmlElement *matrices)
{
	TiXmlElement *element;

	for (element = matrices->FirstChildElement(); element != NULL;
		element = element->NextSiblingElement())
	{
		m_xmlMatrices[element->Value()] = element->GetText();
	}
}

void NativePov::loadPovCodes(
	TiXmlElement *element,
	PovMapping &mapping)
{
	TiXmlElement *child;

	for (child = element->FirstChildElement("POVCode"); child != NULL;
		child = child->NextSiblingElement("POVCode"))
	{
		std::string code = child->GetText();

		mapping.povCodes.push_back(code);
	}
}

void NativePov::loadPovFilenames(
	TiXmlElement *element,
	PovMapping &mapping,
	const std::string &povVersion /*= std::string()*/)
{
	TiXmlElement *child;

	for (child = element->FirstChildElement("POVFilename"); child != NULL;
		child = child->NextSiblingElement("POVFilename"))
	{
		std::string filename = child->GetText();

		mapping.povFilenames.push_back(filename);
		if (povVersion.size() > 0)
		{
			m_includeVersions[filename] = povVersion;
		}
	}
}

void NativePov::loadPovDependency(
	TiXmlElement *element,
	PovMapping &mapping)
{
	std::string name = element->GetText();

	if (name.size() > 0)
	{
		TiXmlElement *dependencyElement =
			m_dependenciesElement->FirstChildElement(name);

		if (dependencyElement != NULL)
		{
			std::string povVersion;

			loadPovDependencies(dependencyElement, mapping);
			TiXmlElement *child = element->FirstChildElement("POVVersion");
			if (child != NULL)
			{
				povVersion = child->GetText();
			}
			loadPovFilenames(dependencyElement, mapping, povVersion);
			loadPovCodes(dependencyElement, mapping);
		}
	}
}

void NativePov::loadPovDependencies(
	TiXmlElement *element,
	PovMapping &mapping)
{
	if (m_dependenciesElement != NULL)
	{
		TiXmlElement *child;

		for (child = element->FirstChildElement("Dependency"); child != NULL;
			child = child->NextSiblingElement("Dependency"))
		{
			loadPovDependency(child, mapping);
		}
	}
}

std::string NativePov::loadPovMapping(
	TiXmlElement *element,
	const char *ldrawElementName,
	PovMapping &mapping)
{
	TiXmlElement *child = element->FirstChildElement("POVName");
	std::string ldrawValue;

	if (child == NULL)
	{
		return "";
	}
	for (; child != NULL; child = child->NextSiblingElement("POVName"))
	{
		PovName name;
		TiXmlAttribute *attr;

		name.name = child->GetText();
		for (attr = child->FirstAttribute(); attr != NULL; attr = attr->Next())
		{
			name.attributes[attr->Name()] = attr->Value();
		}
		mapping.names.push_back(name);
	}
	child = element->FirstChildElement("IoR");
	if (child != NULL)
	{
		mapping.ior = child->GetText();
	}
	child = element->FirstChildElement(ldrawElementName);
	if (child == NULL)
	{
		return "";
	}
	ldrawValue = child->GetText();
	loadPovDependencies(element, mapping);
	loadPovFilenames(element, mapping);
	return ldrawValue;
}

void NativePov::loadXmlColors(TiXmlElement *colors)
{
	TiXmlElement *element;

	for (element = colors->FirstChildElement(); element != NULL;
		element = element->NextSiblingElement())
	{
		PovMapping colorMapping;
		std::string ldrawValue = loadPovMapping(element, "LDrawNumber",
			colorMapping);

		if (ldrawValue.size() > 0)
		{
			m_xmlColors[(ulong)atoi(ldrawValue.c_str())] = colorMapping;
		}
	}
}

void NativePov::loadXmlElements(TiXmlElement *elements)
{
	TiXmlElement *element;

	for (element = elements->FirstChildElement(); element != NULL;
		element = element->NextSiblingElement())
	{
		PovElement povElement;
		std::string ldrawFilename = loadPovMapping(element, "LDrawFilename",
			povElement);

		if (ldrawFilename.size() > 0)
		{
			TiXmlElement *child = element->FirstChildElement("MatrixRef");
			std::string matrixString;
			float *m;

			if (child)
			{
				matrixString = m_xmlMatrices[child->GetText()];
			}
			if (matrixString.size() == 0)
			{
				child = element->FirstChildElement("Matrix");

				if (child)
				{
					matrixString = child->GetText();
				}
			}
			m = povElement.matrix;
			if (sscanf(matrixString.c_str(),
				"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,", &m[0],
				&m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8], &m[9],
				&m[10], &m[11], &m[12], &m[13], &m[14], &m[15]) != 16)
			{
				PovVector3::initIdentityMatrix(m);
			}
			m_xmlElements[ldrawFilename] = povElement;
		}

	}
}

void NativePov::loadPovLDrawPovXml()
{
	std::string filename;

	if (m_xmlMapPath.size() > 0)
	{
		filename = m_xmlMapPath.toLatin1().constData();
	}
	else
	{
		filename = QDir::toNativeSeparators(QString("%1/extras/%2").arg(Preferences::lpubDataPath).arg(VER_LGEO_XML_FILE)).toLatin1().constData();
	}
	TiXmlDocument doc(filename);

	if (doc.LoadFile())
	{
		TiXmlHandle hDoc(&doc);
		TiXmlElement *root =
			hDoc.FirstChildElement("LDrawPOV").Element();
		TiXmlElement *element;

		if (root == NULL)
		{
			return;
		}
		m_dependenciesElement = root->FirstChildElement("Dependencies");
		element = root->FirstChildElement("Colors");
		if (element != NULL)
		{
			loadXmlColors(element);
		}
		element = root->FirstChildElement("Matrices");
		if (element != NULL)
		{
			loadXmlMatrices(element);
		}
		element = root->FirstChildElement("Elements");
		if (element != NULL)
		{
			loadXmlElements(element);
		}
	}
}

/**************************************************
  Utility functions:
***************************************************/

// PovVector3::PovVector3(void) -- Constructor
// Expects:
//	  void
PovVector3::PovVector3(void)
 : lcVector3(0.0f,0.0f,0.0f)
{}

// PovVector3::PovVector3(const float _x, const float _y, const float _z) -- Constructor
// Expects:
//        _x, _y, _z	: 3 floats to store in the PovVector3
PovVector3::PovVector3(const float _x, const float _y, const float _z)
  : lcVector3(_x,_y,_z)
{}

// PovVector3::PovVector3(const float *) -- Constructor
// Expects:
//	   v	: An array of 3 floats to store in the PovVector3
PovVector3::PovVector3(const float *v)
  :lcVector3(v[0],v[1],v[2])
{}

// PovVector3::PovVector3(const PovVector3&) -- Constructor
// Expects:
//	   v	: A PovVector3 vector of 3 floats to store in the PovVector3
PovVector3::PovVector3(const PovVector3& v)
  :lcVector3(v[0],v[1],v[2])
{}

// PovVector3::operator/(float) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar divide.
// Returns:
//		The PovVector3 divided by the scalar "right".
PovVector3 PovVector3::operator/(float right) const
{
	// Note: this is done so we can perform 3 multiplies and a divide instead of
	// three divides.  My understanding is that FP divides can take
	// significantly longer than FP multiplies.
	float mult = 1.0f / right;

	return PovVector3(x*mult, y*mult, z*mult);
}

// PovVector3::operator*=(const PovVector3&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the cross product *=
// Returns:
//		*this, after it has been set to be the cross product of "*this" cross
//		"right"
PovVector3& PovVector3::operator*=(const PovVector3& right)
{
	x = y * right.z - z * right.y;
	y = z * right.x - x * right.z;
	z = x * right.y - y * right.x;
	return *this;
}

// PovVector3::operator*=(const PovVector3&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar *=
// Returns:
//		*this, after it has been multiplied by the scalar "right".
PovVector3& PovVector3::operator*=(float right)
{
	x *= right;
	y *= right;
	z *= right;
	return *this;
}

// PovVector3::operator*(float) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar multiply.
// Returns:
//		The PovVector3 multiplied by the scalar "right".
PovVector3 PovVector3::operator*(float right) const
{
	return PovVector3(x*right, y*right, z*right);
}

// PovVector3::operator*(const PovVector3&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the cross product.
// Returns:
//		The cross product of "*this" cross "right".
PovVector3 PovVector3::operator*(const PovVector3& right) const
{
	// <a, b, c> x <d, e, f> = <bf-ce, cd-af, ae-bd>
	// <x1, y1, z1> x <x2, y2, z2> = <y1*z2-y2*z1, z1*x2-z2*x1, x1*y2-x2*y1>
	return PovVector3(
	    y * right.z - z * right.y,
	    z * right.x - x * right.z,
	    x * right.y - y * right.x);
}

// PovVector3::operator=(const PovVector3&) -- Overloaded Operator
// Expects:
//		right	: The vector copy the value of.
// Returns:
//		*this, which has the value of "right".
PovVector3& PovVector3::operator=(const PovVector3& right)
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}

// PovVector3::operator=(const lcVector3&) -- Overloaded Operator
// Expects:
//		right	: The vector copy the value of.
// Returns:
//		*this, which has the value of "right".
PovVector3& PovVector3::operator=(const lcVector3& right)
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}

void PovVector3::initIdentityMatrix(float *matrix)
{
	memcpy(matrix, sm_identityMatrix, sizeof(sm_identityMatrix));
}

double PovVector3::doubleLength(const double *v)
{
	return sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
}

void PovVector3::doubleAdd(const double *v1, const double *v2, double *v3)
{
	v3[0] = v1[0] + v2[0];
	v3[1] = v1[1] + v2[1];
	v3[2] = v1[2] + v2[2];
}

void PovVector3::doubleMultiply(const double *v1, double *v2, double n)
{
	v2[0] = v1[0] * n;
	v2[1] = v1[1] * n;
	v2[2] = v1[2] * n;
}

void PovVector3::doubleNormalize(double *v)
{
	double invLength = 1.0 / sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));

	v[0] *= invLength;
	v[1] *= invLength;
	v[2] *= invLength;
}

PovVector3& PovVector3::normalize(void)
{
	return *this *= 1.0f / Length();
}

// Note: static method
void PovVector3::calcRotationMatrix(
	float latitude,
	float longitude,
	float *rotationMatrix)
{
	float leftMatrix[16];
	float rightMatrix[16];
	//float resultMatrix[16];
	float cosTheta;
	float sinTheta;
	PovVector3::initIdentityMatrix(leftMatrix);
	PovVector3::initIdentityMatrix(rightMatrix);
	latitude = (float)deg2rad(latitude);
	longitude = (float)deg2rad(longitude);

	// First, apply latitude by rotating around X.
	cosTheta = (float)cos(latitude);
	sinTheta = (float)sin(latitude);
	rightMatrix[5] = cosTheta;
	rightMatrix[6] = sinTheta;
	rightMatrix[9] = -sinTheta;
	rightMatrix[10] = cosTheta;
	PovVector3::multMatrix(leftMatrix, rightMatrix, rotationMatrix);

	memcpy(leftMatrix, rotationMatrix, sizeof(leftMatrix));
	PovVector3::initIdentityMatrix(rightMatrix);

	// Next, apply longitude by rotating around Y.
	cosTheta = (float)cos(longitude);
	sinTheta = (float)sin(longitude);
	rightMatrix[0] = cosTheta;
	rightMatrix[2] = -sinTheta;
	rightMatrix[8] = sinTheta;
	rightMatrix[10] = cosTheta;
	PovVector3::multMatrix(leftMatrix, rightMatrix, rotationMatrix);
}

// Convert ourselves into a double array.
void PovVector3::upConvert(double *doubleVector)
{
	doubleVector[0] = (double)x;
	doubleVector[1] = (double)y;
	doubleVector[2] = (double)z;
}

PovVector3 PovVector3::mult(float* matrix) const
{
	PovVector3 tempVec;

	tempVec[0] = x*matrix[0] + y*matrix[1] +
	 z*matrix[2] + matrix[3];
	tempVec[1] = x*matrix[4] + y*matrix[5] +
	 z*matrix[6] + matrix[7];
	tempVec[2] = x*matrix[8] + y*matrix[9] +
	 z*matrix[10] + matrix[11];
	return tempVec;
}

void PovVector3::print(FILE* outFile) const
{
	fprintf(outFile, "%f %f %f", x, y, z);
}

float PovVector3::dot(const PovVector3& right) const
{
	return x * right.x + y * right.y + z * right.z;
}

float PovVector3::determinant(const float *matrix)
{
	float det;

	det = matrix[0] * (matrix[5] * matrix[10] - matrix[6] * matrix[9]);
	det -= matrix[4] * (matrix[1] * matrix[10] - matrix[2] * matrix[9]);
	det += matrix[8] * (matrix[1] * matrix[6] - matrix[2] * matrix[5]);
	return det;
}

float PovVector3::invertMatrix(const float *matrix, float *inverseMatrix)
{
	float det = determinant(matrix);

	if (fEq2(det, 0.0f, 1e-8))
	{
		memset(inverseMatrix, 0, sizeof(float) * 16);
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Matrix inversion failed.\n"));
	}
	else
	{
		det = 1.0f / det;

		inverseMatrix[0]  =  (matrix[5] * matrix[10] - matrix[6] * matrix[9]) *
			det;
		inverseMatrix[1]  = -(matrix[1] * matrix[10] - matrix[2] * matrix[9]) *
			det;
		inverseMatrix[2]  =  (matrix[1] * matrix[6]  - matrix[2] * matrix[5]) *
			det;
		inverseMatrix[4]  = -(matrix[4] * matrix[10] - matrix[6] * matrix[8]) *
			det;
		inverseMatrix[5]  =  (matrix[0] * matrix[10] - matrix[2] * matrix[8]) *
			det;
		inverseMatrix[6]  = -(matrix[0] * matrix[6]  - matrix[2] * matrix[4]) *
			det;
		inverseMatrix[8]  =  (matrix[4] * matrix[9]  - matrix[5] * matrix[8]) *
			det;
		inverseMatrix[9]  = -(matrix[0] * matrix[9]  - matrix[1] * matrix[8]) *
			det;
		inverseMatrix[10] =  (matrix[0] * matrix[5]  - matrix[1] * matrix[4]) *
			det;

		inverseMatrix[12] = -(matrix[12] * matrix[0] + matrix[13] * matrix[4] +
							matrix[14] * matrix[8]);
		inverseMatrix[13] = -(matrix[12] * matrix[1] + matrix[13] * matrix[5] +
							matrix[14] * matrix[9]);
		inverseMatrix[14] = -(matrix[12] * matrix[2] + matrix[13] * matrix[6] +
							matrix[14] * matrix[10]);

		inverseMatrix[3] = inverseMatrix[7] = inverseMatrix[11] = 0.0;
		inverseMatrix[15] = 1.0;
	}
	return det;
}

void PovVector3::multMatrix(const float* left, const float* right, float* result)
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

// CLASSES

void PovVector3::transformPoint(const float *matrix, PovVector3 &newPoint) const
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

PovVector3 PovVector3::transformPoint(const float *matrix) const
{
	PovVector3 newPoint;

	transformPoint(matrix, newPoint);
	return newPoint;
}

float PovVector3::sm_identityMatrix[16] =
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

float LDLFacing::glMatrix[16] =
{
	(float)1.0, (float)0.0, (float)0.0, (float)0.0,
	(float)0.0, (float)1.0, (float)0.0, (float)0.0,
	(float)0.0, (float)0.0, (float)1.0, (float)0.0,
	(float)0.0, (float)0.0, (float)0.0, (float)1.0
};

LDLFacing::LDLFacing(void)
	:PovVector3((float)0.0, (float)0.0, (float)0.0)
{
	rotation = (float)1.0;
}

LDLFacing::LDLFacing(PovVector3 &a, float phi)
{
	setFacing(a, phi);
}

LDLFacing LDLFacing::operator+(const LDLFacing& otherFacing)
{
	return mult(otherFacing);
}

float LDLFacing::angleBetween(const LDLFacing &f2)
{
  return (this->x*f2.x) +
         (this->y*f2.y) +
         (this->z*f2.z) +
         (this->rotation*f2.rotation);
}

LDLFacing LDLFacing::inverse(void)
{
	LDLFacing newFacing;

	newFacing.x = -x;
	newFacing.y = -y;
	newFacing.z = -z;
	newFacing.rotation = rotation;

	return newFacing;
}

LDLFacing LDLFacing::negate(void)
{
	LDLFacing newFacing;

	newFacing.x = -x;
	newFacing.y = -y;
	newFacing.z = -z;

	return newFacing;
}

LDLFacing LDLFacing::mult(const LDLFacing& f2)
{
	LDLFacing temp;

	temp.x = this->rotation*f2.x +
		 this->x*f2.rotation +
		 this->y*f2.z -
		 this->z*f2.y;
	temp.y = this->rotation*f2.y +
		 this->y*f2.rotation +
		 this->z*f2.x -
		 this->x*f2.z;
	temp.z = this->rotation*f2.z +
		 this->z*f2.rotation +
		 this->x*f2.y -
		 this->y*f2.x;
	temp.rotation = this->rotation*f2.rotation -
		 this->x*f2.x -
		 this->y*f2.y -
		 this->z*f2.z;

	return temp;
}

LDLFacing LDLFacing::dot(LDLFacing& f2)
{
	LDLFacing answer;
	double temp;
	int i;

	answer.rotation = (this->rotation * f2.rotation) -
		((this->x * f2.x) +
		(this->y * f2.y) +
		(this->z * f2.z));
	answer.x = (this->rotation * f2.x) +
		(f2.rotation * this->x) +
		((this->y * f2.z) -
		(this->z * f2.y));
	answer.y = (this->rotation * f2.y) +
		(f2.rotation * this->y) +
		((this->z * f2.x) -
		(this->x * f2.z));
	answer.z = (this->rotation * f2.z) +
		(f2.rotation * this->z) +
		((this->x * f2.y) -
		(this->y * f2.x));

	temp = sqrt((double)(answer.x*answer.x +
		answer.y*answer.y +
		answer.z*answer.z +
		answer.rotation*answer.rotation));
	for (i=0; i<4; i++)
	{
		answer[i] = (float)(answer[i]/temp);
	}
	return answer;
}

void LDLFacing::setFacing(const PovVector3 &a, float phi)
{
	double phiOver2 = phi / 2.0;

	(*this).x = a.x;
	(*this).y = a.y;
	(*this).z = a.z;
	PovVector3::Normalize();

	(*this).x *= (float)sin(phiOver2);
	(*this).y *= (float)sin(phiOver2);
	(*this).z *= (float)sin(phiOver2);

	this->rotation = (float)cos(phiOver2);
	//printf("LDLFacing set to %f, %f, %f, %f\n", (*this).x, (*this).y,
	// (*this).z, this->rotation);
}

float& LDLFacing::operator[](int i)
{
	if (i == 3)
	{
		return rotation;
	}
	else
	{
		return ((float*)this)[i];
	}
}

LDLFacing& LDLFacing::normalize(void)
{
	int which, i;
	float gr;

	which = 0;
	gr = (*this)[which];
	for (i = 1 ; i < 4 ; i++)
	{
		if (fabs((*this)[i]) > fabs(gr))
		{
			gr = (*this)[i];
			which = i;
		}
	}
	(*this)[which] = (float)0.0;   /* So it doesn't affect next operation */

	(*this)[which] = (float)(sqrt(1.0 - ((*this).x*(*this).x +
		(*this).y*(*this).y +
		(*this).z*(*this).z +
		this->rotation*this->rotation)));

	/* Check to see if we need negative square root */
	if (gr < 0.0)
	{
		(*this)[which] = -(*this)[which];
	}

	return *this;

}

float* LDLFacing::getMatrix(void)
{
//	return glMatrix;

	glMatrix[0] = (float)(1 - 2.0 * ((*this).y * (*this).y + (*this).z * (*this).z));
	glMatrix[1] = (float)(2.0 * ((*this).x * (*this).y - (*this).z * this->rotation));
	glMatrix[2] = (float)(2.0 * ((*this).z * (*this).x + (*this).y * this->rotation));
	glMatrix[3] = (float)0.0;

	glMatrix[4] = (float)(2.0 * ((*this).x * (*this).y + (*this).z * this->rotation));
	glMatrix[5] = (float)(1 - 2.0 * ((*this).z * (*this).z + (*this).x * (*this).x));
	glMatrix[6] = (float)(2.0 * ((*this).y * (*this).z - (*this).x * this->rotation));
	glMatrix[7] = (float)0.0;

	glMatrix[8] = (float)(2.0 * ((*this).z * (*this).x - (*this).y * this->rotation));
	glMatrix[9] = (float)(2.0 * ((*this).y * (*this).z + (*this).x * this->rotation));
	glMatrix[10] = (float)(1 - 2.0 * ((*this).y * (*this).y + (*this).x * (*this).x));
	glMatrix[11] = (float)0.0;

	glMatrix[12] = (float)0.0;
	glMatrix[13] = (float)0.0;
	glMatrix[14] = (float)0.0;
	glMatrix[15] = (float)1.0;

	return glMatrix;
}

void LDLFacing::getInverseMatrix(float *inverseMatrix)
{
	PovVector3::invertMatrix(getMatrix(), inverseMatrix);
}

PovVector3 LDLFacing::getVector() const
{
	return PovVector3(
	 (float)(2.0*((*this).z * (*this).x - (*this).y * this->rotation)),
	 (float)(2.0*((*this).y * (*this).z + (*this).x * this->rotation)),
	 (float)(1-2.0 * ((*this).y * (*this).y + (*this).x * (*this).x))).normalize();
}

void LDLFacing::pointAt(const PovVector3 &v2)
{
	PovVector3 tempVec = v2;
	tempVec.Normalize();
	PovVector3 axis = v2 * this->getVector();
	float radians = (float)acos(tempVec.dot(this->getVector()));
	this->setFacing(axis, radians);
}

void LDLFacing::print(FILE* outFile)
{
	PovVector3::print(outFile);
	fprintf(outFile, " %f", rotation);
}

PovVector3 LDLFacing::difference(const LDLFacing &from)
{
	PovVector3 newVec(from.getVector());
	newVec.mult(getMatrix());
	return newVec;
}

void LDLFacing::swapMatrixRows(float* m, int r1, int r2)
{
	float tmpRow[4];

	memcpy(tmpRow, m + r1*4, 4*sizeof(float));
	memmove(m + r1*4, m + r2*4, 4*sizeof(float));
	memmove(m + r2*4, tmpRow, 4*sizeof(float));
}

float* LDLFacing::invertMatrix(float* inM)
{
	float* inv;
	float m[16];
	int i, j;
	float tmp;

	memcpy(m, inM, sizeof(m));
	inv = glMatrix;
	memcpy(inv, sm_identityMatrix, sizeof(sm_identityMatrix));
	if (fEq(m[0], 0.0))
	{
		for (i = 1; i < 4 && fEq(m[i*4], 0); i++)
			;
		if (i == 4)
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Matrix inversion failed.\n"));
//			exit(1);
		}
		swapMatrixRows(m, 0, i);
		swapMatrixRows(inv, 0, i);
	}
	tmp = m[0];
	if (!fEq(tmp, 1.0))
	{
		for (i = 0; i < 4; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	for (i = 1; i < 4; i++)
	{
		if (!fEq(m[i*4], 0.0))
		{
			tmp = m[i*4];
			for (j = 0; j < 4; j++)
			{
				m[i*4+j] -= tmp * m[j];
				inv[i*4+j] -= tmp * inv[j];
			}
		}
	}
	if (fEq(m[5], 0.0))
	{
		for (i = 2; i < 4 && fEq(m[i*4+1], 0); i++)
			;
		if (i == 4)
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Matrix inversion failed.\n"));
//			exit(1);
		}
		swapMatrixRows(m, 1, i);
		swapMatrixRows(inv, 1, i);
	}
	tmp = m[5];
	if (!fEq(tmp, 1.0))
	{
		for (i = 4; i < 8; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	for (i = 2; i < 4; i++)
	{
		if (!fEq(m[i*4+1], 0.0))
		{
			tmp = m[i*4+1];
			for (j = 0; j < 4; j++)
			{
				m[i*4+j] -= tmp * m[j+4];
				inv[i*4+j] -= tmp * inv[j+4];
			}
		}
	}
	if (fEq(m[10], 0.0))
	{
		if (fEq(m[14], 0.0))
		{
			emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Matrix inversion failed.\n"));
//			exit(1);
		}
		swapMatrixRows(m, 2, 3);
		swapMatrixRows(inv, 2, 3);
	}
	tmp = m[10];
	if (!fEq(tmp, 1.0))
	{
		for (i = 8; i < 12; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	if (!fEq(m[14], 0.0))
	{
		tmp = m[14];
		for (j = 0; j < 4; j++)
		{
			m[12+j] -= tmp * m[j+8];
			inv[12+j] -= tmp * inv[j+8];
		}
	}
	if (fEq(m[15], 0.0))
	{
		emit gui->messageSig(LOG_ERROR,QMessageBox::tr("Matrix inversion failed.\n"));
//		exit(1);
	}
	tmp = m[15];
	if (!fEq(tmp, 1.0))
	{
		for (i = 12; i < 16; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	tmp = m[1];
	for (i = 0; i < 4; i++)
	{
		m[i] -= m[i+4]*tmp;
		inv[i] -= inv[i+4]*tmp;
	}
	tmp = m[2];
	for (i = 0; i < 4; i++)
	{
		m[i] -= m[i+8]*tmp;
		inv[i] -= inv[i+8]*tmp;
	}
	tmp = m[3];
	for (i = 0; i < 4; i++)
	{
		m[i] -= m[i+12]*tmp;
		inv[i] -= inv[i+12]*tmp;
	}
	tmp = m[6];
	for (i = 0; i < 4; i++)
	{
		m[i+4] -= m[i+8]*tmp;
		inv[i+4] -= inv[i+8]*tmp;
	}
	tmp = m[7];
	for (i = 0; i < 4; i++)
	{
		m[i+4] -= m[i+12]*tmp;
		inv[i+4] -= inv[i+12]*tmp;
	}
	tmp = m[11];
	for (i = 0; i < 4; i++)
	{
		m[i+8] -= m[i+12]*tmp;
		inv[i+8] -= inv[i+12]*tmp;
	}
	return inv;
}

LDLCamera::LDLCamera(void)
	:name(NULL)
{
}

LDLCamera::LDLCamera(const LDLCamera &other)
	:name(NULL)
{
	duplicate(other);
}

LDLCamera::~LDLCamera(void)
{
	delete[] name;
}

LDLCamera& LDLCamera::duplicate(const LDLCamera& copyFrom)
{
	position = copyFrom.position;
	facing = copyFrom.facing;
	setName(copyFrom.name);
	return *this;
}

void LDLCamera::setName(char* n)
{
	delete[] name;
	name = copyString(n);
}

void LDLCamera::rotate(const PovVector3 &rotation)
{
	float inverseMatrix[16];

	facing.getInverseMatrix(inverseMatrix);
	if (!fEq(rotation.get(1), 0.0))
	{
		facing = facing +
			LDLFacing(PovVector3(-1,0,0).mult(inverseMatrix).normalize(),
			rotation.get(1));
	}
	if (!fEq(rotation.get(0), 0.0))
	{
		facing = facing +
			LDLFacing(PovVector3(0,1,0).mult(inverseMatrix).normalize(),
			rotation.get(0));
	}
	if (!fEq(rotation.get(2), 0.0))
	{
		facing = facing +
			LDLFacing(PovVector3(0,0,1).mult(inverseMatrix).normalize(),
			rotation.get(2));
	}
}

void LDLCamera::move(const PovVector3 &distance)
{
	float inverseMatrix[16];
	PovVector3 v = distance;
	facing.getInverseMatrix(inverseMatrix);
	position += v.mult(inverseMatrix);
}

// AUTO CAMERA

LDLAutoCamera::LDLAutoCamera(void):
//m_model(NULL),                              // rem this
m_haveGlobeRadius(false),
m_distanceMultiplier(1.0f),
m_width(0.0f),
m_height(0.0f),
m_margin(0.0f),
//m_step(-1),                                // rem this
m_cameraData(NULL)
{
}

LDLAutoCamera::~LDLAutoCamera(void)
{
}

void LDLAutoCamera::dealloc(void)
{
	delete m_cameraData;
	POVObject::dealloc();
}

//void LDLAutoCamera::setModel(LDLModel *value)   // rem this
//{
//	m_model = value;
//}

void LDLAutoCamera::setModelCenter(const PovVector3 &value)
{
	m_modelCenter = value;
}

void LDLAutoCamera::setRotationMatrix(const float *value)
{
	memcpy(m_rotationMatrix, value, sizeof(value[0]) * 16);
}

void LDLAutoCamera::setCamera(const LDLCamera &value)
{
	m_camera = value;
}

void LDLAutoCamera::setCameraGlobe(const char *value)
{
	if (value && sscanf(value, "%*f,%*f,%f", &m_globeRadius) == 1)
	{
		m_haveGlobeRadius = true;
	}
	else
	{
		m_haveGlobeRadius = false;
	}
}

void LDLAutoCamera::setDistanceMultiplier(float value)
{
	m_distanceMultiplier = value;
}

void LDLAutoCamera::setWidth(float value)
{
	m_width = value;
}

void LDLAutoCamera::setHeight(float value)
{
	m_height = value;
}

void LDLAutoCamera::setMargin(float value)
{
	m_margin = value;
}

void LDLAutoCamera::setFov(float value)
{
	m_fov = value;
}

void LDLAutoCamera::zoomToFit(void)
{
	if (m_haveGlobeRadius)
	{
		PovVector3 location;
		location[0] = location[1] = 0.0;
		if (m_globeRadius >= 0)
		{
			location[2] = m_globeRadius;
		}
		else
		{
			location[2] *= 1.0f - m_globeRadius / 100.0f;
		}
		m_camera.setPosition(location);
	}
/*	else if (m_model)
	{
		float d;
		float a[6][6];
		float b[6];
		float x[6];
		PovVector3 tmpVec;
		PovVector3 location;
		PovVector3 cameraDir;
		float tmpMatrix[16];
		float tmpMatrix2[16];
		float transformationMatrix[16];
		LDLMainModel *mainModel = m_model->getMainModel();
		bool origScanControlPoints =
			mainModel->getScanConditionalControlPoints();

		PovVector3::initIdentityMatrix(tmpMatrix);
		tmpMatrix[12] = m_modelCenter[0];
		tmpMatrix[13] = m_modelCenter[1];
		tmpMatrix[14] = m_modelCenter[2];
		PovVector3::multMatrix(tmpMatrix, m_rotationMatrix, tmpMatrix2);
		tmpMatrix[12] = -m_modelCenter[0];
		tmpMatrix[13] = -m_modelCenter[1];
		tmpMatrix[14] = -m_modelCenter[2];
		PovVector3::multMatrix(tmpMatrix2, tmpMatrix, transformationMatrix);
		preCalcCamera();
#ifdef _DEBUG
		m_numPoints = 0;
#endif // _DEBUG
		mainModel->setScanConditionalControlPoints(
			m_scanConditionalControlPoints);
		m_model->scanPoints(this,
			(LDLScanPointCallback)&LDLAutoCamera::scanCameraPoint,
			transformationMatrix, m_step, true);
		mainModel->setScanConditionalControlPoints(origScanControlPoints);
#ifdef _DEBUG
		debugPrintf("num points: %d\n", m_numPoints);
#endif // _DEBUG
		memset(a, 0, sizeof(a));
		memset(b, 0, sizeof(b));
		a[0][0] = m_cameraData->normal[0][0];
		a[0][1] = m_cameraData->normal[0][1];
		a[0][2] = m_cameraData->normal[0][2];
		b[0] = m_cameraData->dMin[0];
		a[1][0] = m_cameraData->normal[1][0];
		a[1][1] = m_cameraData->normal[1][1];
		a[1][2] = m_cameraData->normal[1][2];
		b[1] = m_cameraData->dMin[1];
		a[2][3] = m_cameraData->normal[2][0];
		a[2][4] = m_cameraData->normal[2][1];
		a[2][5] = m_cameraData->normal[2][2];
		b[2] = m_cameraData->dMin[2];
		a[3][3] = m_cameraData->normal[3][0];
		a[3][4] = m_cameraData->normal[3][1];
		a[3][5] = m_cameraData->normal[3][2];
		b[3] = m_cameraData->dMin[3];
		if (m_cameraData->direction[0] == 0.0)
		{
			a[4][1] = -m_cameraData->direction[2];
			a[4][2] = m_cameraData->direction[1];
			a[4][4] = m_cameraData->direction[2];
			a[4][5] = -m_cameraData->direction[1];
			if (m_cameraData->direction[1] == 0.0)
			{
				a[5][0] = -m_cameraData->direction[2];
				a[5][2] = m_cameraData->direction[0];
				a[5][3] = m_cameraData->direction[2];
				a[5][5] = -m_cameraData->direction[0];
			}
			else
			{
				a[5][0] = -m_cameraData->direction[1];
				a[5][1] = m_cameraData->direction[0];
				a[5][3] = m_cameraData->direction[1];
				a[5][4] = -m_cameraData->direction[0];
			}
		}
		else
		{
			a[4][0] = -m_cameraData->direction[2];
			a[4][2] = m_cameraData->direction[0];
			a[4][3] = m_cameraData->direction[2];
			a[4][5] = -m_cameraData->direction[0];
			if (m_cameraData->direction[1] == 0.0 && m_cameraData->direction[2]
				!= 0.0)
			{
				a[5][1] = -m_cameraData->direction[2];
				a[5][2] = m_cameraData->direction[1];
				a[5][4] = m_cameraData->direction[2];
				a[5][5] = -m_cameraData->direction[1];
			}
			else
			{
				a[5][0] = -m_cameraData->direction[1];
				a[5][1] = m_cameraData->direction[0];
				a[5][3] = m_cameraData->direction[1];
				a[5][4] = -m_cameraData->direction[0];
			}
		}
		if (!L3Solve6(x, a, b))
		{
			// Singular matrix; can't work
			// (We shouldn't ever get here, so I'm not going to bother with an
			// error message.  I'd have to first come up with some mechanism
			// for communicating the error with the non-portable part of the
			// app so that it could be displayed to the user.)
			return;
		}
		tmpVec = PovVector3(x[3], x[4], x[5]) - PovVector3(x[0], x[1], x[2]);
		cameraDir = PovVector3(m_cameraData->direction[0], m_cameraData->direction[1],
			m_cameraData->direction[2]);
		d = cameraDir.dot(tmpVec);
		if (d > 0.0)
		{
			location[0] = x[0];
			location[1] = x[1];
			location[2] = x[2] * (m_height + m_margin) / m_height;
		}
		else
		{
			location[0] = x[3];
			location[1] = x[4];
			location[2] = x[5] * (m_width + m_margin) / m_width;
		}
		location[2] *= m_distanceMultiplier;
		m_camera.setPosition(location);
	}  */
}

//void LDLAutoCamera::scanCameraPoint(
//	const PovVector3 &point,
//	const LDLFileLine * /*pFileLine*/)
//{
//	float d;
//	int i;

//	for (i = 0; i < 4; i++)
//	{
//		d = m_cameraData->normal[i].dot(point - m_modelCenter);
//		if (d < m_cameraData->dMin[i])
//		{
//			m_cameraData->dMin[i] = d;
//		}
//	}
//#ifdef _DEBUG
//	m_numPoints++;
//#endif // _DEBUG
//}

void LDLAutoCamera::preCalcCamera(void)
{
	float d;
	int i;

	delete m_cameraData;
	m_cameraData = new CameraData;
	if (m_width > m_height)
	{
		m_cameraData->fov = (float)(2.0 * rad2deg(atan(tan(deg2rad(m_fov / 2.0)) *
			(double)m_width / (double)m_height)));
	}
	else
	{
		m_cameraData->fov = m_fov;
	}
	d = (float)(1.0 / tan(deg2rad(m_cameraData->fov / 2.0)));
	m_cameraData->normal[2] = m_cameraData->direction -
		(m_cameraData->horizontal * d);
	m_cameraData->normal[3] = m_cameraData->direction +
		(m_cameraData->horizontal * d);
	d *= m_width / m_height;
	m_cameraData->normal[0] = m_cameraData->direction -
		(m_cameraData->vertical * d);
	m_cameraData->normal[1] = m_cameraData->direction +
		(m_cameraData->vertical * d);
	for (i = 0; i < 4; i++)
	{
		m_cameraData->normal[i].normalize();
	}
}


/* The following L3Solve6 was extracted by Lars C. Hassing in May 2005
  from jama_lu.h, part of the "JAMA/C++ Linear Algebra Package" (JAva MAtrix)
  found together with the "Template Numerical Toolkit (TNT)"
  at http://math.nist.gov/tnt/download.html
  Their disclaimer:
  "This software was developed at the National Institute of Standards and
   Technology (NIST) by employees of the Federal Government in the course
   of their official duties. Pursuant to title 17 Section 105 of the
   United States Code this software is not subject to copyright protection
   and is in the public domain. NIST assumes no responsibility whatsoever
   for its use by other parties, and makes no guarantees, expressed or
   implied, about its quality, reliability, or any other characteristic."
*/

/* Solve A*x=b, returns 1 if OK, 0 if A is singular */
int LDLAutoCamera::L3Solve6(float x[L3ORDERN],
            const float A[L3ORDERM][L3ORDERN],
            const float b[L3ORDERM])
{
  float          LU_[L3ORDERM][L3ORDERN];
  int            pivsign;
  int            piv[L3ORDERM];/* pivot permutation vector                  */
  int            i;
  int            j;
  int            k;
  int            p;
  float         *LUrowi;
  float          LUcolj[L3ORDERM];
  int            kmax;
  double         s;
  float          t;

  /** LU Decomposition.
  For an m-by-n matrix A with m >= n, the LU decomposition is an m-by-n
  unit lower triangular matrix L, an n-by-n upper triangular matrix U,
  and a permutation vector piv of length m so that A(piv,:) = L*U.
  If m < n, then L is m-by-m and U is m-by-n.

  The LU decompostion with pivoting always exists, even if the matrix is
  singular, so the constructor will never fail.  The primary use of the
  LU decomposition is in the solution of square systems of simultaneous
  linear equations.  This will fail if isNonsingular() returns false.
                                                                            */
  memcpy(LU_, A, sizeof(LU_));

  /* Use a "left-looking", dot-product, Crout/Doolittle algorithm. */
  for (i = 0; i < L3ORDERM; i++)
     piv[i] = i;
  pivsign = 1;

  /* Outer loop. */
  for (j = 0; j < L3ORDERN; j++)
  {
     /* Make a copy of the j-th column to localize references. */
     for (i = 0; i < L3ORDERM; i++)
        LUcolj[i] = LU_[i][j];

     /* Apply previous transformations. */
     for (i = 0; i < L3ORDERM; i++)
     {
        LUrowi = LU_[i];
        /* Most of the time is spent in the following dot product. */
        kmax = i < j ? i : j;  /* min(i, j)                                 */
        s = 0.0;
        for (k = 0; k < kmax; k++)
           s += LUrowi[k] * LUcolj[k];
        LUrowi[j] = LUcolj[i] -= (float)s;
     }

     /* Find pivot and exchange if necessary. */
     p = j;
     for (i = j + 1; i < L3ORDERM; i++)
     {
        if (fabs(LUcolj[i]) > fabs(LUcolj[p]))
           p = i;
     }
     if (p != j)
     {
        for (k = 0; k < L3ORDERN; k++)
        {
           t = LU_[p][k];
           LU_[p][k] = LU_[j][k];
           LU_[j][k] = t;
        }
        k = piv[p];
        piv[p] = piv[j];
        piv[j] = k;
        pivsign = -pivsign;
     }

     /* Compute multipliers. */
     if ((j < L3ORDERM) && (LU_[j][j] != 0.0))
     {
        for (i = j + 1; i < L3ORDERM; i++)
           LU_[i][j] /= LU_[j][j];
     }
  }

  /* LCH: This was LU Decomposition. Now solve: */

  /** Solve A*x = b, where x and b are vectors of length equal
        to the number of rows in A.

  @param  b   a vector (Array1D> of length equal to the first dimension of A.
  @return x   a vector (Array1D> so that L*U*x = b(piv), if B is nonconformant,
              returns 0x0 (null) array.
                                                                            */
  /* Is the matrix nonsingular? I.e. is upper triangular factor U (and hence
     A) nonsingular (isNonsingular())                                       */
  for (j = 0; j < L3ORDERN; j++)
  {
     if (LU_[j][j] == 0)
        return 0;
  }

  for (i = 0; i < L3ORDERN; i++)
     x[i] = b[piv[i]];

  /* Solve L*Y = B(piv) */
  for (k = 0; k < L3ORDERN; k++)
  {
     for (i = k + 1; i < L3ORDERN; i++)
        x[i] -= x[k] * LU_[i][k];
  }

  /* Solve U*X = Y; */
  for (k = L3ORDERN - 1; k >= 0; k--)
  {
     x[k] /= LU_[k][k];
     for (i = 0; i < k; i++)
        x[i] -= x[k] * LU_[i][k];
  }

  return 1;
}                               /* Solve6                                    */
