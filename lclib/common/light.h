#pragma once

#include "object.h"
#include "lc_math.h"

#define LC_LIGHT_HIDDEN			   0x0001
#define LC_LIGHT_DISABLED		   0x0002

enum lcLightSection : quint32
{
	LC_LIGHT_SECTION_INVALID = ~0U,
	LC_LIGHT_SECTION_POSITION = 0,
	LC_LIGHT_SECTION_TARGET
};

enum class lcLightType
{
	Point,
	Spot,
	Directional,
	Area,
	Count
};

enum class lcLightAreaShape
{
	Rectangle,
	Square,
	Disk,
	Ellipse,
	Count
};

/*** LPub3D Mod - LPUB meta properties ***/
enum class lcLightFormat
{
	BlenderLight,
	POVRayLight,
	Count
};
/*** LPub3D Mod end ***/

class lcLight : public lcObject
{
public:
	lcLight(const lcVector3& Position, lcLightType LightType);
	virtual ~lcLight() = default;

	lcLight(const lcLight&) = delete;
	lcLight(lcLight&&) = delete;
	lcLight& operator=(const lcLight&) = delete;
	lcLight& operator=(lcLight&&) = delete;

	static QString GetLightTypeString(lcLightType LightType);
	static QStringList GetLightTypeStrings();
	static QString GetAreaShapeString(lcLightAreaShape LightAreaShape);
	static QStringList GetAreaShapeStrings();
/*** LPub3D Mod - LPUB meta properties ***/
	static QString GetLightFormatString(lcLightFormat LightFormat);
	static QStringList GetLightFormatStrings();
/*** LPub3D Mod end ***/

	bool IsPointLight() const
	{
		return mLightType == lcLightType::Point;
	}

	bool IsSpotLight() const
	{
		return mLightType == lcLightType::Spot;
	}

	bool IsDirectionalLight() const
	{
		return mLightType == lcLightType::Directional;
	}

	bool IsAreaLight() const
	{
		return mLightType == lcLightType::Area;
	}

	lcLightType GetLightType() const
	{
		return mLightType;
	}

	bool SetLightType(lcLightType LightType);

	bool IsSelected() const override
	{
		return mSelected;
	}

	bool IsSelected(quint32 Section) const override
	{
		Q_UNUSED(Section);

		return mSelected;
	}

	void SetSelected(bool Selected) override
	{
		mSelected = Selected;

		if (!Selected)
			mFocusedSection = LC_LIGHT_SECTION_INVALID;
	}

	void SetSelected(quint32 Section, bool Selected) override
	{
		Q_UNUSED(Section);

		mSelected = Selected;

		if (!Selected)
			mFocusedSection = LC_LIGHT_SECTION_INVALID;
	}

	bool IsFocused() const override
	{
		return mFocusedSection != LC_LIGHT_SECTION_INVALID;
	}

	bool IsFocused(quint32 Section) const override
	{
		return mFocusedSection == Section;
	}

	void SetFocused(quint32 Section, bool Focused) override
	{
		if (Focused)
		{
			mFocusedSection = Section;
			mSelected = true;
		}
		else
			mFocusedSection = LC_LIGHT_SECTION_INVALID;
	}

	quint32 GetFocusSection() const override
	{
		return mFocusedSection;
	}

	quint32 GetAllowedTransforms() const override
	{
		if (IsPointLight())
			return LC_OBJECT_TRANSFORM_MOVE_XYZ;

		const quint32 Section = GetFocusSection();

		if (Section == LC_LIGHT_SECTION_POSITION || Section == LC_LIGHT_SECTION_INVALID)
			return LC_OBJECT_TRANSFORM_MOVE_XYZ | LC_OBJECT_TRANSFORM_ROTATE_XYZ;

		if (Section == LC_LIGHT_SECTION_TARGET)
			return LC_OBJECT_TRANSFORM_MOVE_XYZ;

		return 0;
	}

	lcMatrix33 GetRelativeRotation() const
	{
		const quint32 Section = GetFocusSection();

		if (Section == LC_LIGHT_SECTION_POSITION)
			return lcMatrix33(mWorldMatrix);
		else
			return lcMatrix33Identity();
	}

	lcVector3 GetSectionPosition(quint32 Section) const override
	{
		if (Section == LC_LIGHT_SECTION_POSITION)
			return mWorldMatrix.GetTranslation();

		if (Section == LC_LIGHT_SECTION_TARGET)
			return lcMul31(lcVector3(0.0f, 0.0f, -mTargetDistance), mWorldMatrix);

		return lcVector3(0.0f, 0.0f, 0.0f);
	}

	void SetPosition(const lcVector3& Position, lcStep Step, bool AddKey)
	{
		mPosition.ChangeKey(Position, Step, AddKey);
	}

	void SetRotation(const lcMatrix33& Rotation, lcStep Step, bool AddKey)
	{
		mRotation.ChangeKey(Rotation, Step, AddKey);
	}

/*** LPub3D Mod - LPUB meta properties ***/
	lcMatrix33 GetRotation() const
	{
		return mRotation;
	}
/*** LPub3D end ***/

	lcVector3 GetRotationCenter() const
	{
		const quint32 Section = GetFocusSection();

		if (Section == LC_LIGHT_SECTION_POSITION || Section == LC_LIGHT_SECTION_INVALID)
		{
			return mWorldMatrix.GetTranslation();
		}
		else
		{
			return lcMul31(lcVector3(0.0f, 0.0f, -mTargetDistance), mWorldMatrix);
		}
	}

	lcVector3 GetPosition() const
	{
		return mWorldMatrix.GetTranslation();
	}

	lcVector3 GetDirection() const
	{
		return -lcVector3(mWorldMatrix[2]);
	}

	const lcMatrix44& GetWorldMatrix() const
	{
		return mWorldMatrix;
	}

	void SaveLDraw(QTextStream& Stream) const;
	bool ParseLDrawLine(QTextStream& Stream);

public:
	void RayTest(lcObjectRayTest& ObjectRayTest) const override;
	void BoxTest(lcObjectBoxTest& ObjectBoxTest) const override;
	void DrawInterface(lcContext* Context, const lcScene& Scene) const override;
	QVariant GetPropertyValue(lcObjectPropertyId PropertyId) const override;
	bool SetPropertyValue(lcObjectPropertyId PropertyId, lcStep Step, bool AddKey, QVariant Value) override;
	bool HasKeyFrame(lcObjectPropertyId PropertyId, lcStep Time) const override;
	bool SetKeyFrame(lcObjectPropertyId PropertyId, lcStep Time, bool KeyFrame) override;
	void RemoveKeyFrames() override;

	void InsertTime(lcStep Start, lcStep Time);
	void RemoveTime(lcStep Start, lcStep Time);

	bool IsVisible() const
	{
		return (mState & LC_LIGHT_HIDDEN) == 0;
	}

	bool SetColor(const lcVector3& Color, lcStep Step, bool AddKey);

	lcVector3 GetColor() const
	{
		return mColor;
	}

	bool SetPOVRayFadeDistance(float Distance, lcStep Step, bool AddKey);

	float GetPOVRayFadeDistance() const
	{
		return mPOVRayFadeDistance;
	}

	bool SetPOVRayFadePower(float Power, lcStep Step, bool AddKey);

	float GetPOVRayFadePower() const
	{
		return mPOVRayFadePower;
	}

	bool SetSpotConeAngle(float Angle, lcStep Step, bool AddKey);

	float GetSpotConeAngle() const
	{
		return mSpotConeAngle;
	}

	bool SetSpotPenumbraAngle(float Angle, lcStep Step, bool AddKey);

	float GetSpotPenumbraAngle() const
	{
		return mSpotPenumbraAngle;
	}

	bool SetSpotPOVRayTightness(float Angle, lcStep Step, bool AddKey);

	float GetSpotPOVRayTightness() const
	{
		return mSpotPOVRayTightness;
	}

	bool SetAreaShape(lcLightAreaShape LightAreaShape);

	lcLightAreaShape GetAreaShape() const
	{
		return mAreaShape;
	}

	bool SetAreaPOVRayGridX(int AreaGrid, lcStep Step, bool AddKey);

	int GetAreaPOVRayGridX() const
	{
		return mAreaPOVRayGridX;
	}

	bool SetAreaPOVRayGridY(int AreaGrid, lcStep Step, bool AddKey);

	int GetAreaPOVRayGridY() const
	{
		return mAreaPOVRayGridY;
	}

	bool SetPointBlenderRadius(float Radius, lcStep Step, bool AddKey);

	float GetPointBlenderRadius() const
	{
		return mPointBlenderRadius;
	}

	bool SetSpotBlenderRadius(float Radius, lcStep Step, bool AddKey);

	float GetSpotBlenderRadius() const
	{
		return mSpotBlenderRadius;
	}

	bool SetDirectionalBlenderAngle(float Angle, lcStep Step, bool AddKey);

	float GetDirectionalBlenderAngle() const
	{
		return mDirectionalBlenderAngle;
	}

	bool SetAreaSizeX(float Size, lcStep Step, bool AddKey);

	float GetAreaSizeX() const
	{
		return mAreaSizeX;
	}

	bool SetAreaSizeY(float Size, lcStep Step, bool AddKey);

	float GetAreaSizeY() const
	{
		return mAreaSizeY;
	}

	bool SetBlenderPower(float Power, lcStep Step, bool AddKey);

	float GetBlenderPower() const
	{
		return mBlenderPower;
	}

	bool SetPOVRayPower(float Power, lcStep Step, bool AddKey);

	float GetPOVRayPower() const
	{
		return mPOVRayPower;
	}

	bool SetCastShadow(bool CastShadow);

	bool GetCastShadow() const
	{
		return mCastShadow;
	}

	bool SetName(const QString& Name);

	QString GetName() const override
	{
		return mName;
	}

/*** LPub3D Mod - LPUB meta properties ***/
	bool SetBlenderCutoffDistance(int BlenderCutoffDistance, lcStep Step, bool AddKey);

	int GetBlenderCutoffDistance() const
	{
		return mBlenderCutoffDistance;
	}

	bool SetBlenderDiffuse(int BlenderDiffuse, lcStep Step, bool AddKey);

	int GetBlenderDiffuse() const
	{
		return mBlenderDiffuse;
	}

	bool SetBlenderSpecular(int BlenderSpecular, lcStep Step, bool AddKey);

	int GetBlenderSpecular() const
	{
		return mBlenderSpecular;
	}

	bool SetLightFormat(lcLightFormat LightFormat);

	lcLightFormat GetLightFormat() const
	{
		return mLightFormat;
	}

	bool GetEnableBlenderCutoff() const
	{
		return mEnableBlenderCutoff;
	}
/*** LPub3D Mod end ***/

/*** LPub3D Mod - LeoCAD meta command ***/
	void SetLCMeta(bool LCMeta)
	{
		mLCMeta = LCMeta;
	}
/*** LPub3D Mod end ***/

	void CompareBoundingBox(lcVector3& Min, lcVector3& Max);
	void UpdatePosition(lcStep Step) override;
	void MoveSelected(lcStep Step, bool AddKey, const lcVector3& Distance, bool FirstMove);
	void Rotate(lcStep Step, bool AddKey, const lcMatrix33& RotationMatrix, const lcVector3& Center, const lcMatrix33& RotationFrame);
	void CreateName(const std::vector<std::unique_ptr<lcLight>>& Lights);

protected:
	void DrawPointLight(lcContext* Context) const;
	void DrawSpotLight(lcContext* Context) const;
	void DrawDirectionalLight(lcContext* Context) const;
	void DrawAreaLight(lcContext* Context) const;

	void SetupLightMatrix(lcContext* Context) const;
	void DrawSphere(lcContext* Context, const lcVector3& Center, float Radius) const;
	void DrawCylinder(lcContext* Context, float Radius, float Height) const;
	void DrawTarget(lcContext* Context) const;
	void DrawCone(lcContext* Context, float TargetDistance) const;

	QString mName;
	lcLightType mLightType = lcLightType::Point;
	bool mCastShadow = true;
	lcObjectProperty<lcVector3> mPosition = lcObjectProperty<lcVector3>(lcVector3(0.0f, 0.0f, 0.0f));
	lcObjectProperty<lcMatrix33> mRotation = lcObjectProperty<lcMatrix33>(lcMatrix33Identity());
	lcObjectProperty<lcVector3> mColor = lcObjectProperty<lcVector3>(lcVector3(1.0f, 1.0f, 1.0f));
	lcObjectProperty<float> mPointBlenderRadius = lcObjectProperty<float>(0.0f);
	lcObjectProperty<float> mSpotBlenderRadius = lcObjectProperty<float>(0.0f);
	lcObjectProperty<float> mDirectionalBlenderAngle = lcObjectProperty<float>(0.526f * LC_DTOR);
	lcObjectProperty<float> mAreaSizeX = lcObjectProperty<float>(250.0f);
	lcObjectProperty<float> mAreaSizeY = lcObjectProperty<float>(250.0f);
	lcObjectProperty<float> mBlenderPower = lcObjectProperty<float>(10.0f);
	lcObjectProperty<float> mPOVRayPower = lcObjectProperty<float>(1.0f);
	lcObjectProperty<float> mPOVRayFadeDistance = lcObjectProperty<float>(0.0f);
	lcObjectProperty<float> mPOVRayFadePower = lcObjectProperty<float>(0.0f);
	lcObjectProperty<float> mSpotConeAngle = lcObjectProperty<float>(80.0f);
	lcObjectProperty<float> mSpotPenumbraAngle = lcObjectProperty<float>(0.0f);
	lcObjectProperty<float> mSpotPOVRayTightness = lcObjectProperty<float>(0.0f);
	lcObjectProperty<int> mAreaPOVRayGridX = lcObjectProperty<int>(2);
	lcObjectProperty<int> mAreaPOVRayGridY = lcObjectProperty<int>(2);
/*** LPub3D Mod - LPUB meta properties ***/
	lcObjectProperty<int> mBlenderCutoffDistance = lcObjectProperty<int>(40);
	lcObjectProperty<int> mBlenderDiffuse = lcObjectProperty<int>(1);
	lcObjectProperty<int> mBlenderSpecular = lcObjectProperty<int>(1);

	lcLightFormat mLightFormat = lcLightFormat::BlenderLight;
/*** LPub3D Mod end ***/
	lcLightAreaShape mAreaShape = lcLightAreaShape::Rectangle;

	quint32 mState = 0;
	bool mSelected = false;
	quint32 mFocusedSection = LC_LIGHT_SECTION_INVALID;
	lcVector3 mTargetMovePosition = lcVector3(0.0f, 0.0f, 0.0f);
	lcMatrix44 mWorldMatrix;

	static constexpr float mTargetDistance = 50.0f;
/*** LPub3D Mod - LPUB meta properties ***/
	bool mEnableBlenderCutoff = false;
/*** LPub3D Mod end ***/
/*** LPub3D Mod - LeoCAD meta command ***/
	bool mLCMeta = false;
/*** LPub3D Mod end ***/
};
