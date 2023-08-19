#include "lc_global.h"
#include "lc_math.h"
#include "lc_colors.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "light.h"
#include "lc_application.h"
#include "lc_context.h"

#define LC_LIGHT_POSITION_EDGE 7.5f
#define LC_LIGHT_TARGET_EDGE 5.0f
#define LC_LIGHT_SPHERE_RADIUS 5.0f
/*** LPub3D Mod - enable lights ***/
#define LC_LIGHT_SUN_RADIUS 8.5f
#define LC_LIGHT_SPOT_BASE_EDGE 12.5f

// New omni light.
lcLight::lcLight(float px, float py, float pz, bool lpubMeta)
	: lcObject(lcObjectType::Light)
{
	mAngleSet      = false;
	mSpotBlendSet  = false;
	mSpotCutoffSet = false;
	mHeightSet     = false;
/*** LPub3D Mod - LPUB meta command ***/
	mLPubMeta      = lpubMeta;
/*** LPub3D Mod end ***/
	Initialize(lcVector3(px, py, pz), lcVector3(0.0f, 0.0f, 0.0f), LC_POINTLIGHT);
	UpdatePosition(1);
}
/*** LPub3D Mod end ***/

// New directional light.
/*** LPub3D Mod - enable lights ***/
lcLight::lcLight(float px, float py, float pz, float tx, float ty, float tz, int LightType)
	: lcObject(lcObjectType::Light)
{
	mLPubMeta = true;
	Initialize(lcVector3(px, py, pz), lcVector3(tx, ty, tz), LightType);
/*** LPub3D Mod end ***/
	UpdatePosition(1);
}

/*** LPub3D Mod - enable lights ***/
void lcLight::SetLightState(int LightType)
{
	mState = 0;

	switch (LightType)
	{
	case LC_AREALIGHT:
	case LC_SUNLIGHT:
	case LC_SPOTLIGHT:
		mState |= LC_LIGHT_DIRECTIONAL;
		break;
	default:
		break;
	}
}

void lcLight::Initialize(const lcVector3& Position, const lcVector3& TargetPosition, int LightType)
{
	SetLightState(LightType);

	mPOVRayLight = false;
	mShadowless = false;
	mEnableCutoff = false;
	mPosition = Position;
	mTargetPosition = TargetPosition;
	mAmbientColor = lcVector4(0.0f, 0.0f, 0.0f, 1.0f);
	mDiffuseColor = lcVector4(0.8f, 0.8f, 0.8f, 1.0f);
	mSpecularColor = lcVector4(1.0f, 1.0f, 1.0f, 1.0f);
	mAttenuation = lcVector3(1.0f, 0.0f, 0.0f);
	mLightColor = lcVector3(1.0f, 1.0f, 1.0f); /*RGB - White*/
	mLightType = LightType ? LightType : int(LC_POINTLIGHT);
	mLightFactor[0] = LightType ? LightType == LC_SUNLIGHT ? 11.4f : 0.25f : 0.0f;
	mLightFactor[1] = LightType == LC_AREALIGHT ? 0.25f : LightType == LC_SPOTLIGHT ? 0.150f : 0.0f;
	mLightDiffuse = 1.0f;
	mLightSpecular = 1.0f;
	mSpotExponent = 10.0f;
	mPOVRayExponent = 1.0f;
	mSpotSize = 75.0f;
	mSpotCutoff = LightType ? LightType != LC_SUNLIGHT ? 40.0f : 0.0f : 30.0f;
	mSpotFalloff = 45.0f;
	mSpotTightness = 0;
	mAreaGrid = lcVector2(10.0f, 10.0f);
	mAreaSize = lcVector2(200.0f, 200.0f);
	mLightShape = LC_LIGHT_SHAPE_SQUARE;

	mPositionKeys.ChangeKey(mPosition, 1, true);
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, true);
	mAmbientColorKeys.ChangeKey(mAmbientColor, 1, true);
	mDiffuseColorKeys.ChangeKey(mDiffuseColor, 1, true);
	mSpecularColorKeys.ChangeKey(mSpecularColor, 1, true);
	mAttenuationKeys.ChangeKey(mAttenuation, 1, true);
	mLightShapeKeys.ChangeKey(mLightShape, 1, true);
	mLightColorKeys.ChangeKey(mLightColor, 1, true);
	mLightTypeKeys.ChangeKey(mLightType, 1, true);
	mLightFactorKeys.ChangeKey(mLightFactor, 1, true);
	mLightDiffuseKeys.ChangeKey(mLightDiffuse, 1, true);
	mLightSpecularKeys.ChangeKey(mLightSpecular, 1, true);
	mSpotCutoffKeys.ChangeKey(mSpotCutoff, 1, true);
	mSpotFalloffKeys.ChangeKey(mSpotFalloff, 1, true);
	mSpotExponentKeys.ChangeKey(mSpotExponent, 1, true);
	mSpotSizeKeys.ChangeKey(mSpotSize, 1, true);
	mSpotTightnessKeys.ChangeKey(mSpotTightness, 1, true);
	mAreaGridKeys.ChangeKey(mAreaGrid, 1, true);
/*** LPub3D Mod end ***/
}

lcLight::~lcLight()
{
}

void lcLight::SaveLDraw(QTextStream& Stream) const
{
/*** LPub3D Mod - enable lights ***/
	lcVector3 Vector;
	QByteArray Meta(mLPubMeta ? "0 !LPUB" : "0 !LEOCAD");
	const QLatin1String LineEnding("\r\n");

	if (mPOVRayLight)
		Stream << QLatin1String(Meta + " LIGHT POV_RAY") << LineEnding;

	if (mShadowless)
		Stream << QLatin1String(Meta + " LIGHT SHADOWLESS") << LineEnding;

	if (mPositionKeys.GetSize() > 1)
		mPositionKeys.SaveKeysLDraw(Stream, "LIGHT POSITION_KEY ");
	else
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	{
		Vector = mLPubMeta ? lcVector3LeoCADToLDraw(mPosition) : mPosition;
		Stream << QLatin1String(Meta + " LIGHT POSITION ") << Vector[0] << ' ' << Vector[1] << ' ' << Vector[2] << LineEnding;
	}

	if (mLightType != LC_POINTLIGHT && !(mLightType == LC_AREALIGHT && mPOVRayLight))
	{
		if (mTargetPositionKeys.GetSize() > 1)
			mTargetPositionKeys.SaveKeysLDraw(Stream, "LIGHT TARGET_POSITION_KEY ");
		else
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		{
			Vector = mLPubMeta ? lcVector3LeoCADToLDraw(mTargetPosition) : mTargetPosition;
			Stream << QLatin1String(Meta + " LIGHT TARGET_POSITION ") << Vector[0] << ' ' << Vector[1] << ' ' << Vector[2] << LineEnding;
		}
	}

	if (mLightColorKeys.GetSize() > 1)
		mLightColorKeys.SaveKeysLDraw(Stream, "LIGHT COLOR_RGB_KEY ");
	else
		Stream << QLatin1String(Meta + " LIGHT COLOR_RGB ") << mLightColor[0] << ' ' << mLightColor[1] << ' ' << mLightColor[2] << LineEnding;

	if (!mPOVRayLight)
	{
		if (mLightDiffuseKeys.GetSize() > 1)
			mLightDiffuseKeys.SaveKeysLDraw(Stream, "LIGHT DIFFUSE_KEY ");
		else
			Stream << QLatin1String(Meta + " LIGHT DIFFUSE ") << mLightDiffuse << LineEnding;

		if (mLightSpecularKeys.GetSize() > 1)
			mLightSpecularKeys.SaveKeysLDraw(Stream, "LIGHT SPECULAR_KEY ");
		else
			Stream << QLatin1String(Meta + " LIGHT SPECULAR ") << mLightSpecular << LineEnding;
	}

	if (mLightType == LC_SUNLIGHT)
	{
		if (mSpotExponentKeys.GetSize() > 1)
			mSpotExponentKeys.SaveKeysLDraw(Stream, "LIGHT STRENGTH_KEY ");
		else
			Stream << QLatin1String(Meta + " LIGHT STRENGTH ") << (mPOVRayLight ? mPOVRayExponent : mSpotExponent) << LineEnding;

		if (!mPOVRayLight)
		{
			if (mLightFactorKeys.GetSize() > 1)
				mLightFactorKeys.SaveKeysLDraw(Stream, "LIGHT ANGLE_KEY ");
			else
				Stream << QLatin1String(Meta + " LIGHT ANGLE ") << mLightFactor[0] << LineEnding;
		}
	}
	else
	{
		if (mSpotExponentKeys.GetSize() > 1)
			mSpotExponentKeys.SaveKeysLDraw(Stream, "LIGHT POWER_KEY ");
		else
			Stream << QLatin1String(Meta + " LIGHT POWER ") << (mPOVRayLight ? mPOVRayExponent : mSpotExponent) << LineEnding;

		if (mEnableCutoff && !mPOVRayLight)
		{
			if (mSpotCutoffKeys.GetSize() > 1)
				mSpotCutoffKeys.SaveKeysLDraw(Stream, "LIGHT CUTOFF_DISTANCE_KEY ");
			else
				Stream << QLatin1String(Meta + " LIGHT CUTOFF_DISTANCE ") << mSpotCutoff << LineEnding;
		}

		switch (mLightType)
		{
		case LC_POINTLIGHT:
			if (!mPOVRayLight)
			{
				if (mLightFactorKeys.GetSize() > 1)
					mLightFactorKeys.SaveKeysLDraw(Stream, "LIGHT RADIUS_KEY ");
				else
					Stream << QLatin1String(Meta + " LIGHT RADIUS ") << mLightFactor[0] << LineEnding;
			}
			break;
		case LC_SPOTLIGHT:
			if (mPOVRayLight)
			{
				if (mLightFactorKeys.GetSize() > 1)
					mLightFactorKeys.SaveKeysLDraw(Stream, "LIGHT RADIUS_KEY ");
				else
					Stream << QLatin1String(Meta + " LIGHT RADIUS ") << (mSpotSize - mSpotFalloff) << LineEnding;
				if (mSpotFalloffKeys.GetSize() > 1)
					mSpotFalloffKeys.SaveKeysLDraw(Stream, "LIGHT SPOT_FALLOFF_KEY ");
				else
					Stream << QLatin1String(Meta + " LIGHT SPOT_FALLOFF ") << mSpotFalloff << LineEnding;
				if (mSpotTightnessKeys.GetSize() > 1)
					mSpotTightnessKeys.SaveKeysLDraw(Stream, "SPOT_TIGHTNESS_KEY ");
				else
					Stream << QLatin1String(Meta + " LIGHT SPOT_TIGHTNESS ") << mSpotTightness << LineEnding;
			}
			else
			{
				if (mSpotSizeKeys.GetSize() > 1)
					mSpotSizeKeys.SaveKeysLDraw(Stream, "LIGHT SPOT_SIZE_KEY ");
				else
					Stream << QLatin1String(Meta + " LIGHT SPOT_SIZE ") << mSpotSize << LineEnding;

				if (mLightFactorKeys.GetSize() > 1)
					mLightFactorKeys.SaveKeysLDraw(Stream, "LIGHT RADIUS_AND_SPOT_BLEND_KEY ");
				else
				{
					Stream << QLatin1String(Meta + " LIGHT RADIUS ") << mLightFactor[0] << LineEnding;
					Stream << QLatin1String(Meta + " LIGHT SPOT_BLEND ") << mLightFactor[1] << LineEnding;
				}
			}
			break;
		case LC_AREALIGHT:
			if (mPOVRayLight)
			{
				if (mAreaGridKeys.GetSize() > 1)
					mAreaGridKeys.SaveKeysLDraw(Stream, "LIGHT AREA_GRID_KEY ");
				else
					Stream << QLatin1String(Meta + " LIGHT AREA_ROWS ") << mAreaGrid[0] << QLatin1String(" AREA_COLUMNS ") << mAreaGrid[1] << LineEnding;
			}
			if (mLightFactorKeys.GetSize() > 1)
				mLightFactorKeys.SaveKeysLDraw(Stream, "LIGHT SIZE_KEY ");
			else
			{
				if (mPOVRayLight)
				{
					Stream << QLatin1String(Meta + " LIGHT WIDTH ") << mAreaSize[0] << QLatin1String(" HEIGHT ") << mAreaSize[1] << LineEnding;
				}
				else
				{
					if (mLightShape == LC_LIGHT_SHAPE_RECTANGLE || mLightShape == LC_LIGHT_SHAPE_ELLIPSE || mLightFactor[1] > 0)
						Stream << QLatin1String(Meta + " LIGHT WIDTH ") << mLightFactor[0] << QLatin1String(" HEIGHT ") << mLightFactor[1] << LineEnding;
					else
						Stream << QLatin1String(Meta + " LIGHT SIZE ") << mLightFactor[0] << LineEnding;
				}
			}
			if (mLightShapeKeys.GetSize() > 1)
				mLightShapeKeys.SaveKeysLDraw(Stream, "LIGHT SHAPE_KEY ");
			else
			{
				Stream << QLatin1String(Meta + " LIGHT SHAPE ");

				QString Shape = QLatin1String("Undefined ");
				switch(mLightShape)
				{
				case LC_LIGHT_SHAPE_SQUARE:
					Shape = QLatin1String("Square ");
					break;
				case LC_LIGHT_SHAPE_DISK:
					Shape = mPOVRayLight ? QLatin1String("Circle ") : QLatin1String("Disk ");
					break;
				case LC_LIGHT_SHAPE_RECTANGLE:
					Shape = QLatin1String("Rectangle ");
					break;
				case LC_LIGHT_SHAPE_ELLIPSE:
					Shape = QLatin1String("Ellipse ");
					break;
				default:
					break;
				}
				Stream << QLatin1String(Shape.toLatin1()) << LineEnding;
			}

			break;
		}
	}

	if (mLightTypeKeys.GetSize() > 1)
		mLightTypeKeys.SaveKeysLDraw(Stream, "LIGHT TYPE_KEY ");
	else
	{
		Stream << QLatin1String(Meta + " LIGHT TYPE ");

		QString Type = QLatin1String("Undefined ");
		switch(mLightType)
		{
		case LC_POINTLIGHT:
			Type = QLatin1String("Point ");
			break;
		case LC_SUNLIGHT:
			Type = QLatin1String("Sun ");
			break;
		case LC_AREALIGHT:
			Type = QLatin1String("Area ");
			break;
		case LC_SPOTLIGHT:
			Type = QLatin1String("Spot ");
			break;
		}
		Stream << QLatin1String(Type.toLatin1()) << QLatin1String("NAME ") << mName << LineEnding;
	}
/*** LPub3D Mod end ***/
}

void lcLight::CreateName(const lcArray<lcLight*>& Lights)
{
	if (!mName.isEmpty())
	{
		bool Found = false;

		for (const lcLight* Light : Lights)
		{
			if (Light->GetName() == mName)
			{
				Found = true;
				break;
			}
		}

		if (!Found)
			return;
	}

	int MaxLightNumber = 0;
/*** LPub3D Mod - enable lights ***/
	const QLatin1String Prefix(mLightType == LC_POINTLIGHT ? "Pointlight "
							 : mLightType == LC_AREALIGHT  ? "Arealight "
							 : mLightType == LC_SUNLIGHT   ? "Sunlight "
							 : "Spotlight ");
/*** LPub3D Mod end ***/

	for (const lcLight* Light : Lights)
	{
		QString LightName = Light->GetName();

		if (LightName.startsWith(Prefix))
		{
			bool Ok = false;
			int LightNumber = LightName.mid(Prefix.size()).toInt(&Ok);

			if (Ok && LightNumber > MaxLightNumber)
				MaxLightNumber = LightNumber;
		}
	}

	mName = Prefix + QString::number(MaxLightNumber + 1);
}

/*** LPub3D Mod - enable lights ***/
bool lcLight::ParseLDrawLine(QTextStream& Stream)
{
	while (!Stream.atEnd())
	{
		QString Token;
		Stream >> Token;
		if (Token == QLatin1String("COLOR_RGB"))
		{
			Stream >> mLightColor[0] >> mLightColor[1] >> mLightColor[2];
			mLightColorKeys.ChangeKey(mLightColor, 1, true);
		}
		else if (Token == QLatin1String("POWER") || Token == QLatin1String("STRENGTH"))
		{
			if (mPOVRayLight)
			{
				Stream >> mPOVRayExponent;
				mSpotExponentKeys.ChangeKey(mPOVRayExponent, 1, true);
			}
			else
			{
				Stream >> mSpotExponent;
				mSpotExponentKeys.ChangeKey(mSpotExponent, 1, true);
			}
		}
		else if (Token == QLatin1String("RADIUS") || Token == QLatin1String("SIZE") || Token == QLatin1String("WIDTH") || (mHeightSet = Token == QLatin1String("HEIGHT")) || (mSpotBlendSet = Token == QLatin1String("SPOT_BLEND")) || (mAngleSet = Token == QLatin1String("ANGLE")))
		{
			if (mPOVRayLight)
			{
				if (Token == QLatin1String("WIDTH"))
					Stream >> mAreaSize[0];
				else if (Token == QLatin1String("HEIGHT"))
					Stream >> mAreaSize[1];
				mLightFactorKeys.ChangeKey(mAreaSize, 1, true);
			}
			else
			{
				if(Token == QLatin1String("HEIGHT") || Token == QLatin1String("SPOT_BLEND"))
					Stream >> mLightFactor[1];
				else
					Stream >> mLightFactor[0];
				mLightFactorKeys.ChangeKey(mLightFactor, 1, true);
			}
		}
		else if (Token == QLatin1String("AREA_ROWS"))
		{
			mPOVRayLight = true;
			Stream >> mAreaGrid[0];
			mAreaGridKeys.ChangeKey(mAreaGrid, 1, true);
		}
		else if (Token == QLatin1String("AREA_COLUMNS"))
		{
			mPOVRayLight = true;
			Stream >> mAreaGrid[1];
			mAreaGridKeys.ChangeKey(mAreaGrid, 1, true);
		}
		else if (Token == QLatin1String("SPOT_FALLOFF"))
		{
			mPOVRayLight = true;
			Stream >> mSpotFalloff;
			mSpotFalloffKeys.ChangeKey(mSpotFalloff, 1, true);
		}
		else if (Token == QLatin1String("SPOT_TIGHTNESS"))
		{
			mPOVRayLight = true;
			Stream >> mSpotTightness;
			mSpotTightnessKeys.ChangeKey(mSpotTightness, 1, true);
		}
		else if (Token == QLatin1String("SPOT_SIZE"))
		{
			Stream >> mSpotSize;
			mSpotSizeKeys.ChangeKey(mSpotSize, 1, true);
		}
		else if (Token == QLatin1String("SHAPE"))
		{
			QString Shape;
			Stream >> Shape;
			Shape = Shape.replace("\"", "").toLower();
			if (Shape == QLatin1String("square"))
				mLightShape = LC_LIGHT_SHAPE_SQUARE;
			else if (Shape == QLatin1String("disk") || Shape == QLatin1String("circle"))
				mLightShape = LC_LIGHT_SHAPE_DISK;
			else if (Shape == QLatin1String("rectangle"))
				mLightShape = LC_LIGHT_SHAPE_RECTANGLE;
			else if (Shape == QLatin1String("ellipse"))
				mLightShape = LC_LIGHT_SHAPE_ELLIPSE;
			mLightShapeKeys.ChangeKey(mLightShape, 1, true);
		}
		else if (Token == QLatin1String("DIFFUSE"))
		{
			Stream >>mLightDiffuse;
			mLightDiffuseKeys.ChangeKey(mLightDiffuse, 1, true);
		}
		else if (Token == QLatin1String("SPECULAR"))
		{
			Stream >>mLightSpecular;
			mLightSpecularKeys.ChangeKey(mLightSpecular, 1, true);
		}
		else if ((mSpotCutoffSet = Token == QLatin1String("CUTOFF_DISTANCE")))
		{
			mEnableCutoff = true;
			Stream >> mSpotCutoff;
			mSpotCutoffKeys.ChangeKey(mSpotCutoff, 1, true);
		}
		else if (Token == QLatin1String("TYPE"))
		{
			QString Type;
			Stream >> Type;
			Type = Type.replace("\"", "").toLower();
			if (Type == QLatin1String("point"))
				mLightType = LC_POINTLIGHT;
			else if (Type == QLatin1String("sun"))
				mLightType = LC_SUNLIGHT;
			else if (Type == QLatin1String("spot"))
				mLightType = LC_SPOTLIGHT;
			else if (Type == QLatin1String("area"))
				mLightType = LC_AREALIGHT;
			SetLightState(mLightType);
			mLightTypeKeys.ChangeKey(mLightType, 1, true);
		}
		else if (Token == QLatin1String("POSITION"))
		{
			if (mLPubMeta)
			{
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction (Reset) ***/
				Stream >> mPosition[0] >> mPosition[2] >> mPosition[1];
				mPosition[2] = -mPosition[2];
			}
			else
				Stream >> mPosition[0] >> mPosition[1] >> mPosition[2];
			mPositionKeys.ChangeKey(mPosition, 1, true);
		}
		else if (Token == QLatin1String("TARGET_POSITION"))
		{
			if (mLPubMeta)
			{
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction (Reset) ***/
				Stream >> mTargetPosition[0] >> mTargetPosition[2] >> mTargetPosition[1];
				mTargetPosition[2] = -mTargetPosition[2];
			}
			else
				Stream >> mTargetPosition[0] >> mTargetPosition[1] >> mTargetPosition[2];
			mTargetPositionKeys.ChangeKey(mTargetPosition, 1, true);
		}
		else if (Token == QLatin1String("POV_RAY"))
		{
			mPOVRayLight = true;
		}
		else if (Token == QLatin1String("SHADOWLESS"))
		{
			mShadowless = true;
		}
		else if (Token == QLatin1String("COLOR_RGB_KEY"))
			mLightColorKeys.LoadKeysLDraw(Stream);
		else if ((Token == QLatin1String("POWER_KEY")) || (Token == QLatin1String("STRENGTH_KEY")))
			mSpotExponentKeys.LoadKeysLDraw(Stream);
		else if ((Token == QLatin1String("ANGLE_KEY")) || (Token == QLatin1String("RADIUS_KEY")) || (Token == QLatin1String("SIZE_KEY")) || (Token == QLatin1String("RADIUS_AND_SPOT_BLEND_KEY")))
			mLightFactorKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("SPOT_SIZE_KEY"))
			mSpotSizeKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("SPOT_FALLOFF_KEY"))
			mSpotFalloffKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("SPOT_TIGHTNESS_KEY"))
			mSpotTightnessKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("AREA_GRID_KEY"))
			mAreaGridKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("SHAPE_KEY"))
			mLightShapeKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("DIFFUSE_KEY"))
			mLightDiffuseKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("SPECULAR_KEY"))
			mLightSpecularKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("CUTOFF_DISTANCE_KEY"))
			mSpotCutoffKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("TYPE_KEY"))
			mLightTypeKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("POSITION_KEY"))
			mPositionKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("TARGET_POSITION_KEY"))
			mTargetPositionKeys.LoadKeysLDraw(Stream);
		else if (Token == QLatin1String("NAME"))
		{
			mName = Stream.readAll().trimmed();
			mName.replace("\"", "");

			// Set default settings per light type
			if (mLightType == LC_SPOTLIGHT)
			{
				if (!mSpotBlendSet)
				{
					mLightFactor[1] = 0.15f;
					mLightFactorKeys.ChangeKey(mLightFactor, 1, true);
				}
			}
			if (mLightType == LC_AREALIGHT && (mLightShape == LC_LIGHT_SHAPE_RECTANGLE || mLightShape == LC_LIGHT_SHAPE_ELLIPSE)) {
				if (!mHeightSet)
				{
					mLightFactor[1] = 0.25f;
					mLightFactorKeys.ChangeKey(mLightFactor, 1, true);
				}
			}
			if (mLightType == LC_SUNLIGHT)
			{
				if (!mAngleSet)
				{
					mLightFactor[0] = 11.4f;
					mLightFactorKeys.ChangeKey(mLightFactor, 1, true);
				}
				if (!mSpotCutoffSet)
				{
					mSpotCutoff     = 0.0f;
					mSpotCutoffKeys.ChangeKey(mSpotCutoff, 1, true);
				}
			}
			return true;
		}
	}

	return false;
}
/*** LPub3D Mod end ***/

void lcLight::CompareBoundingBox(lcVector3& Min, lcVector3& Max)
{
	const lcVector3 Points[2] =
	{
		mPosition, mTargetPosition
	};

	for (int i = 0; i < (IsPointLight() ? 1 : 2); i++)
	{
		const lcVector3& Point = Points[i];

		// TODO: this should check the entire mesh

		Min = lcMin(Point, Min);
		Max = lcMax(Point, Max);
	}
}

/*** LPub3D Mod - enable lights ***/
void lcLight::UpdateLight(lcStep Step, lcLightProperties Props, int Property)
{
	switch(Property)
	{
	case LC_LIGHT_SHAPE:
		mLightShape = Props.mLightShape;
		mLightShapeKeys.ChangeKey(mLightShape, Step, false);
		break;
	case LC_LIGHT_COLOR:
		mLightColor = Props.mLightColor;
		mLightColorKeys.ChangeKey(mLightColor, Step, false);
		break;
	case LC_LIGHT_FACTOR:
		if (Props.mPOVRayLight && mLightType == LC_AREALIGHT)
		{
			mAreaSize = Props.mLightFactor;
			mLightFactorKeys.ChangeKey(mAreaSize, 1, true);
		}
		else
		{
			mLightFactor = Props.mLightFactor;
			mLightFactorKeys.ChangeKey(mLightFactor, 1, true);
		}
		break;
	case LC_LIGHT_DIFFUSE:
		mLightDiffuse = Props.mLightDiffuse;
		mLightDiffuseKeys.ChangeKey(mLightDiffuse, Step, false);
		break;
	case LC_LIGHT_SPECULAR:
		mLightSpecular = Props.mLightSpecular;
		mLightSpecularKeys.ChangeKey(mLightSpecular, Step, false);
		break;
	case LC_LIGHT_SHADOWLESS:
		mShadowless = Props.mShadowless;
		break;
	case LC_LIGHT_EXPONENT:
		if (Props.mPOVRayLight)
		{
			mPOVRayExponent = Props.mSpotExponent;
			mSpotExponentKeys.ChangeKey(mPOVRayExponent, Step, false);
		}
		else
		{
			mSpotExponent = Props.mSpotExponent;
			mSpotExponentKeys.ChangeKey(mSpotExponent, Step, false);
		}
		break;
	case LC_LIGHT_AREA_GRID:
		mAreaGrid = Props.mAreaGrid;
		mAreaGridKeys.ChangeKey(mAreaGrid, Step, false);
		break;
	case LC_LIGHT_SPOT_SIZE:
		mSpotSize = Props.mSpotSize;
		mSpotSizeKeys.ChangeKey(mSpotSize, Step, false);
		break;
	case LC_LIGHT_SPOT_FALLOFF:
		mSpotFalloff = Props.mSpotFalloff;
		mSpotFalloffKeys.ChangeKey(mSpotFalloff, Step, false);
		break;
	case LC_LIGHT_SPOT_TIGHTNESS:
		mSpotTightness = Props.mSpotTightness;
		mSpotTightnessKeys.ChangeKey(mSpotTightness, Step, false);
		break;
	case LC_LIGHT_CUTOFF:
		mSpotCutoff = Props.mSpotCutoff;
		mSpotCutoffKeys.ChangeKey(mSpotCutoff, Step, false);
		break;
	case LC_LIGHT_USE_CUTOFF:
		mEnableCutoff = Props.mEnableCutoff;
		break;
	case LC_LIGHT_POVRAY:
		mPOVRayLight = Props.mPOVRayLight;
		break;
	}
	UpdatePosition(Step);
}
/*** LPub3D Mod end ***/

void lcLight::RayTest(lcObjectRayTest& ObjectRayTest) const
{
	if (IsPointLight())
	{
		float Distance;

		if (lcSphereRayMinIntersectDistance(mPosition, LC_LIGHT_SPHERE_RADIUS, ObjectRayTest.Start, ObjectRayTest.End, &Distance))
		{
			ObjectRayTest.ObjectSection.Object = const_cast<lcLight*>(this);
			ObjectRayTest.ObjectSection.Section = LC_LIGHT_SECTION_POSITION;
			ObjectRayTest.Distance = Distance;
		}

		return;
	}

	lcVector3 Min = lcVector3(-LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE);
	lcVector3 Max = lcVector3(LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE);

	lcVector3 Start = lcMul31(ObjectRayTest.Start, mWorldLight);
	lcVector3 End = lcMul31(ObjectRayTest.End, mWorldLight);

	float Distance;
	lcVector3 Plane;

	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr, &Plane) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcLight*>(this);
		ObjectRayTest.ObjectSection.Section = LC_LIGHT_SECTION_POSITION;
		ObjectRayTest.Distance = Distance;
		ObjectRayTest.PieceInfoRayTest.Plane = Plane;
	}

	Min = lcVector3(-LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE);
	Max = lcVector3(LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE);

	lcMatrix44 WorldTarget = mWorldLight;
	WorldTarget.SetTranslation(lcMul30(-mTargetPosition, WorldTarget));

	Start = lcMul31(ObjectRayTest.Start, WorldTarget);
	End = lcMul31(ObjectRayTest.End, WorldTarget);

	if (lcBoundingBoxRayIntersectDistance(Min, Max, Start, End, &Distance, nullptr, &Plane) && (Distance < ObjectRayTest.Distance))
	{
		ObjectRayTest.ObjectSection.Object = const_cast<lcLight*>(this);
		ObjectRayTest.ObjectSection.Section = LC_LIGHT_SECTION_TARGET;
		ObjectRayTest.Distance = Distance;
		ObjectRayTest.PieceInfoRayTest.Plane = Plane;
	}
}

void lcLight::BoxTest(lcObjectBoxTest& ObjectBoxTest) const
{
	if (IsPointLight())
	{
		for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
			if (lcDot3(mPosition, ObjectBoxTest.Planes[PlaneIdx]) + ObjectBoxTest.Planes[PlaneIdx][3] > LC_LIGHT_SPHERE_RADIUS)
				return;

		ObjectBoxTest.Objects.Add(const_cast<lcLight*>(this));
		return;
	}

	lcVector3 Min(-LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE, -LC_LIGHT_POSITION_EDGE);
	lcVector3 Max(LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE, LC_LIGHT_POSITION_EDGE);

	lcVector4 LocalPlanes[6];

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		const lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], mWorldLight);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(mWorldLight[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcLight*>(this));
		return;
	}

	Min = lcVector3(-LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE);
	Max = lcVector3(LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE, LC_LIGHT_TARGET_EDGE);

	lcMatrix44 WorldTarget = mWorldLight;
	WorldTarget.SetTranslation(lcMul30(-mTargetPosition, WorldTarget));

	for (int PlaneIdx = 0; PlaneIdx < 6; PlaneIdx++)
	{
		const lcVector3 Normal = lcMul30(ObjectBoxTest.Planes[PlaneIdx], WorldTarget);
		LocalPlanes[PlaneIdx] = lcVector4(Normal, ObjectBoxTest.Planes[PlaneIdx][3] - lcDot3(WorldTarget[3], Normal));
	}

	if (lcBoundingBoxIntersectsVolume(Min, Max, LocalPlanes))
	{
		ObjectBoxTest.Objects.Add(const_cast<lcLight*>(this));
		return;
	}
}

void lcLight::MoveSelected(lcStep Step, bool AddKey, const lcVector3& Distance)
{
	if (IsSelected(LC_LIGHT_SECTION_POSITION))
	{
		mPosition += Distance;
		mPositionKeys.ChangeKey(mPosition, Step, AddKey);
	}

	if (IsSelected(LC_LIGHT_SECTION_TARGET))
	{
		mTargetPosition += Distance;
		mTargetPositionKeys.ChangeKey(mTargetPosition, Step, AddKey);
	}
}

void lcLight::InsertTime(lcStep Start, lcStep Time)
{
	mPositionKeys.InsertTime(Start, Time);
	mTargetPositionKeys.InsertTime(Start, Time);
	mAmbientColorKeys.InsertTime(Start, Time);
	mDiffuseColorKeys.InsertTime(Start, Time);
	mSpecularColorKeys.InsertTime(Start, Time);
	mAttenuationKeys.InsertTime(Start, Time);
/*** LPub3D Mod - enable lights ***/
	mLightShapeKeys.InsertTime(Start, Time);
	mLightColorKeys.InsertTime(Start, Time);
	mLightTypeKeys.InsertTime(Start, Time);
	mLightFactorKeys.InsertTime(Start, Time);
	mLightDiffuseKeys.InsertTime(Start, Time);
	mLightSpecularKeys.InsertTime(Start, Time);
	mAreaGridKeys.InsertTime(Start, Time);
	mSpotFalloffKeys.InsertTime(Start, Time);
	mSpotTightnessKeys.InsertTime(Start, Time);
	mSpotSizeKeys.InsertTime(Start, Time);
/*** LPub3D Mod end ***/
	mSpotCutoffKeys.InsertTime(Start, Time);
	mSpotExponentKeys.InsertTime(Start, Time);
}

void lcLight::RemoveTime(lcStep Start, lcStep Time)
{
	mPositionKeys.RemoveTime(Start, Time);
	mTargetPositionKeys.RemoveTime(Start, Time);
	mAmbientColorKeys.RemoveTime(Start, Time);
	mDiffuseColorKeys.RemoveTime(Start, Time);
	mSpecularColorKeys.RemoveTime(Start, Time);
	mAttenuationKeys.RemoveTime(Start, Time);
/*** LPub3D Mod - enable lights ***/
	mLightShapeKeys.RemoveTime(Start, Time);
	mLightColorKeys.RemoveTime(Start, Time);
	mLightTypeKeys.RemoveTime(Start, Time);
	mLightFactorKeys.RemoveTime(Start, Time);
	mLightDiffuseKeys.RemoveTime(Start, Time);
	mLightSpecularKeys.RemoveTime(Start, Time);
	mAreaGridKeys.RemoveTime(Start, Time);
	mSpotFalloffKeys.RemoveTime(Start, Time);
	mSpotTightnessKeys.RemoveTime(Start, Time);
	mSpotSizeKeys.RemoveTime(Start, Time);
/*** LPub3D Mod end ***/
	mSpotCutoffKeys.RemoveTime(Start, Time);
	mSpotExponentKeys.RemoveTime(Start, Time);
}

void lcLight::UpdatePosition(lcStep Step)
{
	mPosition = mPositionKeys.CalculateKey(Step);
	mTargetPosition = mTargetPositionKeys.CalculateKey(Step);
	mAmbientColor = mAmbientColorKeys.CalculateKey(Step);
	mDiffuseColor = mDiffuseColorKeys.CalculateKey(Step);
	mSpecularColor = mSpecularColorKeys.CalculateKey(Step);
	mAttenuation = mAttenuationKeys.CalculateKey(Step);
/*** LPub3D Mod - enable lights ***/
	mLightShape = mLightShapeKeys.CalculateKey(Step);
	mLightColor = mLightColorKeys.CalculateKey(Step);
	mLightType = mLightTypeKeys.CalculateKey(Step);
	mLightFactor = mLightFactorKeys.CalculateKey(Step);
	mLightDiffuse = mLightDiffuseKeys.CalculateKey(Step);
	mLightSpecular = mLightSpecularKeys.CalculateKey(Step);
	mAreaGrid = mAreaGridKeys.CalculateKey(Step);
	mSpotSize = mSpotSizeKeys.CalculateKey(Step);
	mSpotFalloff = mSpotFalloffKeys.CalculateKey(Step);
	mSpotTightness = mSpotTightnessKeys.CalculateKey(Step);
/*** LPub3D Mod end ***/
	mSpotCutoff = mSpotCutoffKeys.CalculateKey(Step);
	mSpotExponent = mSpotExponentKeys.CalculateKey(Step);

	if (IsPointLight())
	{
		mWorldLight = lcMatrix44Identity();
		mWorldLight.SetTranslation(-mPosition);
	}
	else
	{
		lcVector3 FrontVector(mTargetPosition - mPosition);
		lcVector3 UpVector(1, 1, 1);

		if (fabs(FrontVector[0]) < fabs(FrontVector[1]))
		{
			if (fabs(FrontVector[0]) < fabs(FrontVector[2]))
				UpVector[0] = -(UpVector[1] * FrontVector[1] + UpVector[2] * FrontVector[2]);
			else
				UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
		}
		else
		{
			if (fabs(FrontVector[1]) < fabs(FrontVector[2]))
				UpVector[1] = -(UpVector[0] * FrontVector[0] + UpVector[2] * FrontVector[2]);
			else
				UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
		}

		mWorldLight = lcMatrix44LookAt(mPosition, mTargetPosition, UpVector);
	}
}

void lcLight::DrawInterface(lcContext* Context, const lcScene& Scene) const
{
	Q_UNUSED(Scene);
	Context->SetMaterial(lcMaterialType::UnlitColor);

	if (IsPointLight())
		DrawPointLight(Context);
/*** LPub3D Mod - enable lights ***/
	else
		DrawDirectionalLight(Context);
/*** LPub3D Mod end ***/
}

/*** LPub3D Mod - enable lights ***/
void lcLight::DrawDirectionalLight(lcContext* Context) const
{
	lcVector3 FrontVector(mTargetPosition - mPosition);
	lcVector3 UpVector(1, 1, 1);

	if (fabs(FrontVector[0]) < fabs(FrontVector[1]))
	{
		if (fabs(FrontVector[0]) < fabs(FrontVector[2]))
			UpVector[0] = -(UpVector[1] * FrontVector[1] + UpVector[2] * FrontVector[2]);
		else
			UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
	}
	else
	{
		if (fabs(FrontVector[1]) < fabs(FrontVector[2]))
			UpVector[1] = -(UpVector[0] * FrontVector[0] + UpVector[2] * FrontVector[2]);
		else
			UpVector[2] = -(UpVector[0] * FrontVector[0] + UpVector[1] * FrontVector[1]);
	}

	lcMatrix44 LightMatrix = lcMatrix44LookAt(mPosition, mTargetPosition, UpVector);
	LightMatrix = lcMatrix44AffineInverse(LightMatrix);
	LightMatrix.SetTranslation(lcVector3(0, 0, 0));

	const lcMatrix44 LightViewMatrix = lcMul(LightMatrix, lcMatrix44Translation(mPosition));
	Context->SetWorldMatrix(LightViewMatrix);

	float Length = FrontVector.Length();

	float Verts[(20 + 8 + 2 + 16) * 3];
	float* CurVert = Verts;

	if (mLightType != LC_SUNLIGHT)
	{
		if (mLightType == LC_SPOTLIGHT)
		{
			for (int EdgeIdx = 0; EdgeIdx < 8; EdgeIdx++)
			{
				float c = cosf((float)EdgeIdx / 4 * LC_PI) * LC_LIGHT_POSITION_EDGE;
				float s = sinf((float)EdgeIdx / 4 * LC_PI) * LC_LIGHT_POSITION_EDGE;

				*CurVert++ = c;
				*CurVert++ = s;
				*CurVert++ = LC_LIGHT_POSITION_EDGE;
				*CurVert++ = c;
				*CurVert++ = s;
				*CurVert++ = -LC_LIGHT_POSITION_EDGE;
			}

			*CurVert++ = -LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ = -LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ = -LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ =  LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ = -LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ =  LC_LIGHT_SPOT_BASE_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		}
		else if (mLightType == LC_AREALIGHT)
		{
			const float LC_LIGHT_AREA_EDGE = 5.0f;
			const float LC_LIGHT_AREA_H_EDGE = 8.5f;
			const float LC_LIGHT_AREA_W_EDGE = 17.0f;

			*CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_POSITION_EDGE;
			*CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_POSITION_EDGE;
			*CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_AREA_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;

			*CurVert++ = -LC_LIGHT_AREA_H_EDGE; *CurVert++ = -LC_LIGHT_AREA_W_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_AREA_H_EDGE; *CurVert++ = -LC_LIGHT_AREA_W_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ =  LC_LIGHT_AREA_H_EDGE; *CurVert++ =  LC_LIGHT_AREA_W_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
			*CurVert++ = -LC_LIGHT_AREA_H_EDGE; *CurVert++ =  LC_LIGHT_AREA_W_EDGE; *CurVert++ = -LC_LIGHT_POSITION_EDGE;
		}

		*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;
		*CurVert++ =  LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE; *CurVert++ = -LC_LIGHT_TARGET_EDGE - Length;

		*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = 0.0f;
		*CurVert++ = 0.0f; *CurVert++ = 0.0f; *CurVert++ = -Length;

		Context->SetVertexBufferPointer(Verts);
		Context->SetVertexFormatPosition(3);
	}

	int BaseIndices = 0;

	if (mLightType == LC_SPOTLIGHT)
	{
		BaseIndices = 56;
		const GLushort Indices[56 + 24 + 2 + 40] =
			{
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
				0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14, 0,
				1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15, 15, 1,
				16, 17, 17, 18, 18, 19, 19, 16,
				20, 21, 21, 22, 22, 23, 23, 20,
				24, 25, 25, 26, 26, 27, 27, 24,
				20, 24, 21, 25, 22, 26, 23, 27,
				28, 29,
				30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38,
				38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 30,
				28, 30, 28, 34, 28, 38, 28, 42
			};

		Context->SetIndexBufferPointer(Indices);
	}
	else if (mLightType == LC_AREALIGHT)
	{
		BaseIndices = 32;
		const GLushort Indices[32 + 24 + 2] =
			{
				0, 1, 1, 2, 2, 3, 3, 0,
				4, 5, 5, 6, 6, 7, 7, 4,
				0, 4, 1, 5, 2, 6, 3, 7,
				8, 9, 9, 10, 10, 11, 11, 8,
				12, 13, 13, 14, 14, 15, 15, 12,
				16, 17, 17, 18, 18, 19, 19, 16,
				12, 16, 13, 17, 14, 18, 15, 19,
				20, 21
			};

		Context->SetIndexBufferPointer(Indices);
	}
	else if (mLightType == LC_SUNLIGHT)
	{
		constexpr float Radius = LC_LIGHT_SUN_RADIUS;
		constexpr int Slices = 9;              // longitude
		constexpr int Stacks = 9;              // latitude
		constexpr int NumSphereVertices = 918; // Slices * 2 * 3 * 3 + (Stacks - 2) * Slices * 4 * 3
		constexpr int NumSphereIndices = 306;  // Slices * 2 + (Stacks - 1) * Slices * 2 * 2

		BaseIndices = NumSphereIndices;

		float Vertices[NumSphereVertices + 24 + 6];
		float *Vert = Vertices;

		quint16 Indices[NumSphereIndices + 24 + 2];
		quint16 *Indx = Indices;

		auto AddVertex = [&](float x, float y, float z)
		{
			*Vert++ = x;
			*Vert++ = y;
			*Vert++ = z;
		};

		auto AddLineIndex = [&](quint16 v1, quint16 v2)
		{
			*Indx++ = v1;
			*Indx++ = v2;
		};

		std::vector<lcVector3> WrkVertices;

		float Slice = LC_2PI / Slices;
		float Stack = LC_PI / Stacks;
		float SliceAngle, StackAngle;

		for(int i = 0; i <= Stacks; ++i)
		{
			StackAngle = LC_PI / 2 - i * Stack;   // starting from pi/2 to -pi/2
			float xy = Radius * cosf(StackAngle);
			float z = Radius * sinf(StackAngle);

			for(int j = 0; j <= Slices; ++j)      // add (Slices+1) vertices per stack
			{
				SliceAngle = j * Slice;

				lcVector3 Vertex;
				Vertex.x = xy * cosf(SliceAngle);
				Vertex.y = xy * sinf(SliceAngle);
				Vertex.z = z;

				WrkVertices.push_back(Vertex);
			}
		}

		int Index = 0;

		lcVector3 v1, v2, v3, v4;

		for(int i = 0; i < Stacks; ++i)
		{
			int vi1 = i * (Slices + 1);
			int vi2 = (i + 1) * (Slices + 1);

			for(int j = 0; j < Slices; ++j, ++vi1, ++vi2)
			{
				// 4 vertices per slice
				// v1--v3
				// |    |
				// v2--v4
				v1 = WrkVertices[vi1];
				v2 = WrkVertices[vi2];
				v3 = WrkVertices[vi1 + 1];
				v4 = WrkVertices[vi2 + 1];

				// if first stack or last stack, store 1 triangle per slice else, store 2 triangles (1 quad) per slice
				if(i == 0)
				{
					// first stack triangle v1-v2-v4
					AddVertex(v1.x, v1.y, v1.z);
					AddVertex(v2.x, v2.y, v2.z);
					AddVertex(v4.x, v4.y, v4.z);

					// only vertical lines for first stack)
					AddLineIndex(Index, Index+1);

					Index += 3;
				}
				else if(i == (Stacks-1))
				{
					// last stack inverted triangle v1-v2-v3
					AddVertex(v1.x, v1.y, v1.z);
					AddVertex(v2.x, v2.y, v2.z);
					AddVertex(v3.x, v3.y, v3.z);

					// both vertical and horizontal lines for last stack
					AddLineIndex(Index, Index+1);
					AddLineIndex(Index, Index+2);

					Index += 3;
				}
				else
				{
					// 2 triangles (quad vertices v1-v2-v3-v4) for other stacks
					AddVertex(v1.x, v1.y, v1.z);
					AddVertex(v2.x, v2.y, v2.z);
					AddVertex(v3.x, v3.y, v3.z);
					AddVertex(v4.x, v4.y, v4.z);

					// both vertical and horizontal lines for other stacks
					AddLineIndex(Index, Index+1);
					AddLineIndex(Index, Index+2);

					Index += 4;
				}
			}
		}

		AddVertex( LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE - Length);
		AddVertex(-LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE - Length);
		AddVertex(-LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE - Length);
		AddVertex( LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE - Length);
		AddVertex( LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE - Length);
		AddVertex(-LC_LIGHT_TARGET_EDGE,  LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE - Length);
		AddVertex(-LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE - Length);
		AddVertex( LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE, -LC_LIGHT_TARGET_EDGE - Length);
		AddVertex(0.0f, 0.0f, 0.0f);
		AddVertex(0.0f, 0.0f, -Length);

		const int Idx[10] = { Index++, Index++, Index++, Index++, Index++, Index++, Index++, Index++, Index++, Index++ };
		AddLineIndex(Idx[0], Idx[1]); AddLineIndex(Idx[1], Idx[2]); AddLineIndex(Idx[2], Idx[3]); AddLineIndex(Idx[3], Idx[0]);
		AddLineIndex(Idx[4], Idx[5]); AddLineIndex(Idx[5], Idx[6]); AddLineIndex(Idx[6], Idx[7]); AddLineIndex(Idx[7], Idx[4]);
		AddLineIndex(Idx[0], Idx[4]); AddLineIndex(Idx[1], Idx[5]); AddLineIndex(Idx[2], Idx[6]); AddLineIndex(Idx[3], Idx[7]);
		AddLineIndex(Idx[8], Idx[9]);

		Context->SetVertexBufferPointer(Vertices);
		Context->SetVertexFormatPosition(3);
		Context->SetIndexBufferPointer(Indices);
	}

	const lcPreferences& Preferences = lcGetPreferences();
	const float LineWidth = Preferences.mLineWidth;
	const lcVector4 SelectedColor = lcVector4FromColor(Preferences.mObjectSelectedColor);
	const lcVector4 FocusedColor = lcVector4FromColor(Preferences.mObjectFocusedColor);
	const lcVector4 LightColor = lcVector4FromColor(Preferences.mLightColor);

	if (!IsSelected())
	{
		Context->SetLineWidth(LineWidth);
		Context->SetColor(LightColor);

		Context->DrawIndexedPrimitives(GL_LINES, BaseIndices + 24 + 2, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		if (IsSelected(LC_LIGHT_SECTION_POSITION))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_LIGHT_SECTION_POSITION))
				Context->SetColor(FocusedColor);
			else
				Context->SetColor(SelectedColor);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetColor(LightColor);
		}

		Context->DrawIndexedPrimitives(GL_LINES, BaseIndices, GL_UNSIGNED_SHORT, 0);

		if (IsSelected(LC_LIGHT_SECTION_TARGET))
		{
			Context->SetLineWidth(2.0f * LineWidth);
			if (IsFocused(LC_LIGHT_SECTION_TARGET))
				Context->SetColor(FocusedColor);
			else
				Context->SetColor(SelectedColor);
		}
		else
		{
			Context->SetLineWidth(LineWidth);
			Context->SetColor(LightColor);
		}

		Context->DrawIndexedPrimitives(GL_LINES, 24, GL_UNSIGNED_SHORT, BaseIndices * 2);

		Context->SetLineWidth(LineWidth);
		Context->SetColor(LightColor);

		int SpotCone = 0;

		if (mLightType == LC_SPOTLIGHT)
		{
			SpotCone = 40;

			float Radius = tanf(LC_DTOR * mSpotCutoff) * Length;

			for (int EdgeIdx = 0; EdgeIdx < 16; EdgeIdx++)
			{
				*CurVert++ = cosf((float)EdgeIdx / 16 * LC_2PI) * Radius;
				*CurVert++ = sinf((float)EdgeIdx / 16 * LC_2PI) * Radius;
				*CurVert++ = -Length;
			}
		}

		Context->DrawIndexedPrimitives(GL_LINES, 2 + SpotCone, GL_UNSIGNED_SHORT, (BaseIndices + 24) * 2);
	}
}
/*** LPub3D Mod end ***/

void lcLight::DrawPointLight(lcContext* Context) const
{
	constexpr int Slices = 6;
	constexpr int NumIndices = 3 * Slices + 6 * Slices * (Slices - 2) + 3 * Slices;
	constexpr int NumVertices = (Slices - 1) * Slices + 2;
	constexpr float Radius = LC_LIGHT_SPHERE_RADIUS;
	lcVector3 Vertices[NumVertices];
	quint16 Indices[NumIndices];

	lcVector3* Vertex = Vertices;
	quint16* Index = Indices;

	*Vertex++ = lcVector3(0, 0, Radius);

	for (int i = 1; i < Slices; i++ )
	{
		const float r0 = Radius * sinf(i * (LC_PI / Slices));
		const float z0 = Radius * cosf(i * (LC_PI / Slices));

		for (int j = 0; j < Slices; j++)
		{
			const float x0 = r0 * sinf(j * (LC_2PI / Slices));
			const float y0 = r0 * cosf(j * (LC_2PI / Slices));

			*Vertex++ = lcVector3(x0, y0, z0);
		}
	}

	*Vertex++ = lcVector3(0, 0, -Radius);

	for (quint16 i = 0; i < Slices - 1; i++ )
	{
		*Index++ = 0;
		*Index++ = 1 + i;
		*Index++ = 1 + i + 1;
	}

	*Index++ = 0;
	*Index++ = 1;
	*Index++ = 1 + Slices - 1;

	for (quint16 i = 0; i < Slices - 2; i++ )
	{
		quint16 Row1 = 1 + i * Slices;
		quint16 Row2 = 1 + (i + 1) * Slices;

		for (quint16 j = 0; j < Slices - 1; j++ )
		{
			*Index++ = Row1 + j;
			*Index++ = Row2 + j + 1;
			*Index++ = Row2 + j;

			*Index++ = Row1 + j;
			*Index++ = Row1 + j + 1;
			*Index++ = Row2 + j + 1;
		}

		*Index++ = Row1 + Slices - 1;
		*Index++ = Row2 + 0;
		*Index++ = Row2 + Slices - 1;

		*Index++ = Row1 + Slices - 1;
		*Index++ = Row2 + 0;
		*Index++ = Row1 + 0;
	}

	for (quint16 i = 0; i < Slices - 1; i++ )
	{
		*Index++ = (Slices - 1) * Slices + 1;
		*Index++ = (Slices - 1) * (Slices - 1) + i;
		*Index++ = (Slices - 1) * (Slices - 1) + i + 1;
	}

	*Index++ = (Slices - 1) * Slices + 1;
	*Index++ = (Slices - 1) * (Slices - 1) + (Slices - 2) + 1;
	*Index++ = (Slices - 1) * (Slices - 1);

	Context->SetWorldMatrix(lcMatrix44Translation(mPosition));

	const lcPreferences& Preferences = lcGetPreferences();

	if (IsFocused(LC_LIGHT_SECTION_POSITION))
	{
		const lcVector4 FocusedColor = lcVector4FromColor(Preferences.mObjectFocusedColor);
		Context->SetColor(FocusedColor);
	}
	else if (IsSelected(LC_LIGHT_SECTION_POSITION))
	{
		const lcVector4 SelectedColor = lcVector4FromColor(Preferences.mObjectSelectedColor);
		Context->SetColor(SelectedColor);
	}
	else
	{
		const lcVector4 LightColor = lcVector4FromColor(Preferences.mLightColor);
		Context->SetColor(LightColor);
	}

	Context->SetVertexBufferPointer(Vertices);
	Context->SetVertexFormatPosition(3);
	Context->SetIndexBufferPointer(Indices);
	Context->DrawIndexedPrimitives(GL_TRIANGLES, NumIndices, GL_UNSIGNED_SHORT, 0);
}

void lcLight::RemoveKeyFrames()
{
	mPositionKeys.RemoveAll();
	mPositionKeys.ChangeKey(mPosition, 1, true);

	mTargetPositionKeys.RemoveAll();
	mTargetPositionKeys.ChangeKey(mTargetPosition, 1, true);

	mAmbientColorKeys.RemoveAll();
	mAmbientColorKeys.ChangeKey(mAmbientColor, 1, true);

	mDiffuseColorKeys.RemoveAll();
	mDiffuseColorKeys.ChangeKey(mDiffuseColor, 1, true);

	mSpecularColorKeys.RemoveAll();
	mSpecularColorKeys.ChangeKey(mSpecularColor, 1, true);

	mAttenuationKeys.RemoveAll();
	mAttenuationKeys.ChangeKey(mAttenuation, 1, true);
/*** LPub3D Mod - enable lights ***/
	mLightShapeKeys.RemoveAll();
	mLightShapeKeys.ChangeKey(mLightShape, 1, false);

	mLightColorKeys.RemoveAll();
	mLightColorKeys.ChangeKey(mLightColor, 1, true);

	mLightFactorKeys.RemoveAll();
	mLightFactorKeys.ChangeKey(mLightFactor, 1, true);

	mLightTypeKeys.RemoveAll();
	mLightTypeKeys.ChangeKey(mLightType, 1, true);

	mLightDiffuseKeys.RemoveAll();
	mLightDiffuseKeys.ChangeKey(mLightDiffuse, 1, true);

	mLightSpecularKeys.RemoveAll();
	mLightSpecularKeys.ChangeKey(mLightSpecular, 1, true);

	mAreaGridKeys.RemoveAll();
	mAreaGridKeys.ChangeKey(mAreaGrid, 1, true);

	mSpotFalloffKeys.RemoveAll();
	mSpotFalloffKeys.ChangeKey(mSpotFalloff, 1, true);

	mSpotTightnessKeys.RemoveAll();
	mSpotTightnessKeys.ChangeKey(mSpotTightness, 1, true);

	mSpotSizeKeys.RemoveAll();
	mSpotSizeKeys.ChangeKey(mSpotSize, 1, false);
/*** LPub3D Mod end ***/
	mSpotCutoffKeys.RemoveAll();
	mSpotCutoffKeys.ChangeKey(mSpotCutoff, 1, true);

	mSpotExponentKeys.RemoveAll();
	mSpotExponentKeys.ChangeKey(mSpotExponent, 1, true);
}

bool lcLight::Setup(int LightIndex)
{
	Q_UNUSED(LightIndex);

	return true;
}
