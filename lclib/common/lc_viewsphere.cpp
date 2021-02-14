#include "lc_global.h"
#include "lc_viewsphere.h"
#include "view.h"
#include "lc_previewwidget.h"
#include "camera.h"
#include "lc_context.h"
#include "lc_stringcache.h"
#include "lc_application.h"
#include "image.h"
#include "lc_texture.h"

lcTexture* lcViewSphere::mTexture;
lcVertexBuffer lcViewSphere::mVertexBuffer;
lcIndexBuffer lcViewSphere::mIndexBuffer;
const float lcViewSphere::mRadius = 1.0f;
const float lcViewSphere::mHighlightRadius = 0.35f;
const int lcViewSphere::mSubdivisions = 7;

/*** LPub3D Mod - preview widget for LPub3D ***/
lcViewSphere::lcViewSphere(View* View)
	: mWidget(View), mPreview(nullptr), mView(View), mIsPreview(false), mIsSubstitute(false)
{
/*** LPub3D Mod end ***/
	UpdateSettings();
}

/*** LPub3D Mod - preview widget for LPub3D ***/
lcViewSphere::lcViewSphere(lcPreviewWidget* Preview, bool Substitute)
	: mWidget(Preview), mPreview(Preview), mView(nullptr), mIsPreview(true), mIsSubstitute(Substitute)
{
/*** LPub3D Mod end ***/
	UpdateSettings();
}

void lcViewSphere::UpdateSettings()
{
	const lcPreferences& Preferences = lcGetPreferences();

	switch (mWidget->GetViewType())
	{
	case lcViewType::View:
		mSize = Preferences.mViewSphereSize;
		mEnabled = Preferences.mViewSphereEnabled;
		mLocation = Preferences.mViewSphereLocation;
		break;

	case lcViewType::Preview:
/*** LPub3D Mod - preview widget for LPub3D ***/		
		if (mIsSubstitute && Preferences.mPreviewViewSphereSize != 50)
			mSize = Preferences.mPreviewViewSphereSize * 0.70;
		else
			mSize = Preferences.mPreviewViewSphereSize;
/*** LPub3D Mod end ***/			
		mEnabled = Preferences.mPreviewViewSphereEnabled;
		mLocation = Preferences.mPreviewViewSphereLocation;
		break;

	case lcViewType::Minifig:
	case lcViewType::Count:
		break;
	}
}

lcMatrix44 lcViewSphere::GetViewMatrix() const
{
	lcMatrix44 ViewMatrix = mWidget->GetCamera()->mWorldView;
	ViewMatrix.SetTranslation(lcVector3(0, 0, 0));
	return ViewMatrix;
}

lcMatrix44 lcViewSphere::GetProjectionMatrix() const
{
	return lcMatrix44Ortho(-mRadius * 1.25f, mRadius * 1.25f, -mRadius * 1.25f, mRadius * 1.25f, -mRadius * 1.25f, mRadius * 1.25f);
}

void lcViewSphere::CreateResources(lcContext* Context)
{
	const int ImageSize = 128;
	mTexture = new lcTexture();

	const QString ViewNames[6] =
	{
		QCoreApplication::translate("ViewName", "Left"), QCoreApplication::translate("ViewName", "Right"), QCoreApplication::translate("ViewName", "Back"),
		QCoreApplication::translate("ViewName", "Front"), QCoreApplication::translate("ViewName", "Top"), QCoreApplication::translate("ViewName", "Bottom")
	};

	const QTransform ViewTransforms[6] =
	{
		QTransform(0, 1, 1, 0, 0, 0), QTransform(0, -1, -1, 0, ImageSize, ImageSize), QTransform(-1, 0, 0, 1, ImageSize, 0),
		QTransform(1, 0, 0, -1, 0, ImageSize), QTransform(1, 0, 0, -1, 0, ImageSize), QTransform(-1, 0, 0, 1, ImageSize, 0)
	};

	QImage PainterImage(ImageSize, ImageSize, QImage::Format_ARGB32);
	QPainter Painter;
	QFont Font("Helvetica", 20);
	std::vector<Image> Images;

	for (int ViewIdx = 0; ViewIdx < 6; ViewIdx++)
	{
		Image TextureImage;
		TextureImage.Allocate(ImageSize, ImageSize, LC_PIXEL_FORMAT_A8);

		Painter.begin(&PainterImage);
		Painter.fillRect(0, 0, PainterImage.width(), PainterImage.height(), QColor(0, 0, 0));
		Painter.setBrush(QColor(255, 255, 255));
		Painter.setPen(QColor(255, 255, 255));
		Painter.setFont(Font);
		Painter.setTransform(ViewTransforms[ViewIdx]);
		Painter.drawText(0, 0, PainterImage.width(), PainterImage.height(), Qt::AlignCenter, ViewNames[ViewIdx]);
		Painter.end();

		for (int y = 0; y < ImageSize; y++)
		{
			unsigned char* Dest = TextureImage.mData + (ImageSize - y - 1) * TextureImage.mWidth;

			for (int x = 0; x < ImageSize; x++)
				*Dest++ = qRed(PainterImage.pixel(x, y));
		}

		Images.emplace_back(std::move(TextureImage));
	}

	mTexture->SetImage(std::move(Images), LC_TEXTURE_CUBEMAP | LC_TEXTURE_LINEAR);

	lcVector3 Verts[(mSubdivisions + 1) * (mSubdivisions + 1) * 6];
	GLushort Indices[mSubdivisions * mSubdivisions * 6 * 6];

	lcMatrix44 Transforms[6] =
	{
		lcMatrix44(lcVector4(0.0f,  1.0f, 0.0f, 0.0f), lcVector4(0.0f,  0.0f, 1.0f, 0.0f), lcVector4(1.0f, 0.0f, 0.0f, 0.0f), lcVector4(1.0f,  0.0f,  0.0f, 1.0f)),
		lcMatrix44(lcVector4(0.0f, -1.0f, 0.0f, 0.0f), lcVector4(0.0f,  0.0f, 1.0f, 0.0f), lcVector4(1.0f, 0.0f, 0.0f, 0.0f), lcVector4(-1.0f,  0.0f,  0.0f, 1.0f)),
		lcMatrix44(lcVector4(-1.0f,  0.0f, 0.0f, 0.0f), lcVector4(0.0f,  0.0f, 1.0f, 0.0f), lcVector4(0.0f, 1.0f, 0.0f, 0.0f), lcVector4(0.0f,  1.0f,  0.0f, 1.0f)),
		lcMatrix44(lcVector4(1.0f,  0.0f, 0.0f, 0.0f), lcVector4(0.0f,  0.0f, 1.0f, 0.0f), lcVector4(0.0f, 1.0f, 0.0f, 0.0f), lcVector4(0.0f, -1.0f,  0.0f, 1.0f)),
		lcMatrix44(lcVector4(1.0f,  0.0f, 0.0f, 0.0f), lcVector4(0.0f,  1.0f, 0.0f, 0.0f), lcVector4(0.0f, 0.0f, 1.0f, 0.0f), lcVector4(0.0f,  0.0f,  1.0f, 1.0f)),
		lcMatrix44(lcVector4(1.0f,  0.0f, 0.0f, 0.0f), lcVector4(0.0f, -1.0f, 0.0f, 0.0f), lcVector4(0.0f, 0.0f, 1.0f, 0.0f), lcVector4(0.0f,  0.0f, -1.0f, 1.0f)),
	};

	const float Step = 2.0f / mSubdivisions;
	lcVector3* CurVert = Verts;

	for (int FaceIdx = 0; FaceIdx < 6; FaceIdx++)
	{
		for (int y = 0; y <= mSubdivisions; y++)
		{
			for (int x = 0; x <= mSubdivisions; x++)
			{
				lcVector3 Vert = lcMul31(lcVector3(Step * x - 1.0f, Step * y - 1.0f, 0.0f), Transforms[FaceIdx]);
				lcVector3 Vert2 = Vert * Vert;

				*CurVert++ = lcVector3(Vert.x * sqrt(1.0 - 0.5 * (Vert2.y + Vert2.z) + Vert2.y * Vert2.z / 3.0),
									   Vert.y * sqrt(1.0 - 0.5 * (Vert2.z + Vert2.x) + Vert2.z * Vert2.x / 3.0),
									   Vert.z * sqrt(1.0 - 0.5 * (Vert2.x + Vert2.y) + Vert2.x * Vert2.y / 3.0)
				) * mRadius;
			}
		}
	}

	GLushort* CurIndex = Indices;

	for (int FaceIdx = 0; FaceIdx < 6; FaceIdx++)
	{
		const int FaceBase = FaceIdx * (mSubdivisions + 1) * (mSubdivisions + 1);

		for (int y = 0; y < mSubdivisions; y++)
		{
			int RowBase = FaceBase + (mSubdivisions + 1) * y;

			for (int x = 0; x < mSubdivisions; x++)
			{
				*CurIndex++ = RowBase + x;
				*CurIndex++ = RowBase + x + 1;
				*CurIndex++ = RowBase + x + (mSubdivisions + 1);

				*CurIndex++ = RowBase + x + 1;
				*CurIndex++ = RowBase + x + 1 + (mSubdivisions + 1);
				*CurIndex++ = RowBase + x + (mSubdivisions + 1);
			}
		}
	}

	mVertexBuffer = Context->CreateVertexBuffer(sizeof(Verts), Verts);
	mIndexBuffer = Context->CreateIndexBuffer(sizeof(Indices), Indices);
}

void lcViewSphere::DestroyResources(lcContext* Context)
{
	delete mTexture;
	mTexture = nullptr;
	Context->DestroyVertexBuffer(mVertexBuffer);
	Context->DestroyIndexBuffer(mIndexBuffer);
}

void lcViewSphere::Draw()
{
	UpdateSettings();

	if (!mSize || !mEnabled)
		return;

	lcContext* Context = mWidget->mContext;
	const int Width = mWidget->GetWidth();
	const int Height = mWidget->GetHeight();
	const int ViewportSize = mSize;
	const int Left = (mLocation == lcViewSphereLocation::BottomLeft || mLocation == lcViewSphereLocation::TopLeft) ? 0 : Width - ViewportSize;
	const int Bottom = (mLocation == lcViewSphereLocation::BottomLeft || mLocation == lcViewSphereLocation::BottomRight) ? 0 : Height - ViewportSize;
	Context->SetViewport(Left, Bottom, ViewportSize, ViewportSize);

	glDepthFunc(GL_ALWAYS);
	glEnable(GL_CULL_FACE);

	Context->SetVertexBuffer(mVertexBuffer);
	Context->SetVertexFormatPosition(3);
	Context->SetIndexBuffer(mIndexBuffer);

	Context->SetMaterial(lcMaterialType::UnlitColor);
	Context->SetColor(lcVector4(0.0f, 0.0f, 0.0f, 1.0f));

	float Scale = 1.005f + 2.0f / (float)ViewportSize;
	Context->SetWorldMatrix(lcMatrix44Scale(lcVector3(Scale, Scale, Scale)));
	Context->SetViewMatrix(GetViewMatrix());
	Context->SetProjectionMatrix(GetProjectionMatrix());

	Context->DrawIndexedPrimitives(GL_TRIANGLES, mSubdivisions * mSubdivisions * 6 * 6, GL_UNSIGNED_SHORT, 0);

	Context->SetMaterial(lcMaterialType::UnlitViewSphere);
	Context->BindTextureCubeMap(mTexture->mTexture);

	Context->SetWorldMatrix(lcMatrix44Identity());
	Context->SetViewMatrix(GetViewMatrix());
	Context->SetProjectionMatrix(GetProjectionMatrix());

	lcVector4 HighlightPosition(0.0f, 0.0f, 0.0f, 0.0f);

	if (mIntersectionFlags.any())
	{
		for (int AxisIdx = 0; AxisIdx < 3; AxisIdx++)
		{
			if (mIntersectionFlags.test(2 * AxisIdx))
				HighlightPosition[AxisIdx] = 1.0f;
			else if (mIntersectionFlags.test(2 * AxisIdx + 1))
				HighlightPosition[AxisIdx] = -1.0f;
		}

		HighlightPosition = lcVector4(lcNormalize(lcVector3(HighlightPosition)), mHighlightRadius);
	}

	const lcPreferences& Preferences = lcGetPreferences();
	const lcVector4 TextColor = lcVector4FromColor(Preferences.mViewSphereTextColor);
	const lcVector4 BackgroundColor = lcVector4FromColor(Preferences.mViewSphereColor);
	const lcVector4 HighlightColor = lcVector4FromColor(Preferences.mViewSphereHighlightColor);

	Context->SetHighlightParams(HighlightPosition, TextColor, BackgroundColor, HighlightColor);
	Context->DrawIndexedPrimitives(GL_TRIANGLES, mSubdivisions * mSubdivisions * 6 * 6, GL_UNSIGNED_SHORT, 0);

	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	Context->SetViewport(0, 0, Width, Height);
}

bool lcViewSphere::OnLeftButtonDown()
{
	if (!mSize || !mEnabled)
		return false;

	mIntersectionFlags = GetIntersectionFlags(mIntersection);

	if (!mIntersectionFlags.any())
		return false;

	mMouseDownX = mWidget->GetMouseX();
	mMouseDownY = mWidget->GetMouseY();
	mMouseDown = true;

	return true;
}

bool lcViewSphere::OnLeftButtonUp()
{
	if (!mSize || !mEnabled)
		return false;

	if (!mMouseDown)
		return false;

	mMouseDown = false;

	if (!mIntersectionFlags.any())
		return false;

	lcVector3 Position(0.0f, 0.0f, 0.0f);

	for (int AxisIdx = 0; AxisIdx < 3; AxisIdx++)
	{
		if (mIntersectionFlags.test(AxisIdx * 2))
			Position[AxisIdx] = 1250.0f;
		else if (mIntersectionFlags.test(AxisIdx * 2 + 1))
			Position[AxisIdx] = -1250.0f;
	}

	mWidget->SetViewpoint(Position);

	return true;
}

bool lcViewSphere::OnMouseMove()
{
	if (!mSize || !mEnabled)
		return false;

	if (IsDragging())
	{
		mIntersectionFlags.reset();
		mIsPreview ? mPreview->StartOrbitTracking() : mView->StartOrbitTracking();
		return true;
	}

	if (mWidget->IsTracking())
		return false;

	std::bitset<6> IntersectionFlags = GetIntersectionFlags(mIntersection);

	if (IntersectionFlags != mIntersectionFlags)
	{
		mIntersectionFlags = IntersectionFlags;
		mWidget->Redraw();
	}

	return mIntersectionFlags.any();
}

bool lcViewSphere::IsDragging() const
{
	int InputStateX = mWidget->GetMouseX();
	int InputStateY = mWidget->GetMouseY();
	return mMouseDown && (qAbs(mMouseDownX - InputStateX) > 3 || qAbs(mMouseDownY - InputStateY) > 3);
}

std::bitset<6> lcViewSphere::GetIntersectionFlags(lcVector3& Intersection) const
{
	const int Width = mWidget->GetWidth();
	const int Height = mWidget->GetHeight();
	const int ViewportSize = mSize;
	const int Left = (mLocation == lcViewSphereLocation::BottomLeft || mLocation == lcViewSphereLocation::TopLeft) ? 0 : Width - ViewportSize;
	const int Bottom = (mLocation == lcViewSphereLocation::BottomLeft || mLocation == lcViewSphereLocation::BottomRight) ? 0 : Height - ViewportSize;
	const int x = mWidget->GetMouseX() - Left;
	const int y = mWidget->GetMouseY() - Bottom;
	std::bitset<6> IntersectionFlags;

	if (x < 0 || x > Width || y < 0 || y > Height)
		return IntersectionFlags;

	lcVector3 StartEnd[2] = { lcVector3(x, y, 0), lcVector3(x, y, 1) };
	const int Viewport[4] = { 0, 0, ViewportSize, ViewportSize };

	lcUnprojectPoints(StartEnd, 2, GetViewMatrix(), GetProjectionMatrix(), Viewport);

	float Distance;
	if (lcSphereRayMinIntersectDistance(lcVector3(0.0f, 0.0f, 0.0f), mRadius, StartEnd[0], StartEnd[1], &Distance))
	{
		Intersection = (StartEnd[0] + (StartEnd[1] - StartEnd[0]) * Distance) / mRadius;

		auto CheckIntersection = [&]()
		{
			for (int Axis1Idx = 0; Axis1Idx < 6; Axis1Idx++)
			{
				lcVector3 Point1(0.0f, 0.0f, 0.0f);

				Point1[Axis1Idx / 2] = Axis1Idx % 2 ? -1.0f : 1.0f;

				if (lcLengthSquared(Point1 - Intersection) < mHighlightRadius * mHighlightRadius)
				{
					IntersectionFlags.set(Axis1Idx);
					return;
				}

				for (int Axis2Idx = 0; Axis2Idx < 6; Axis2Idx++)
				{
					if (Axis1Idx / 2 == Axis2Idx / 2)
						continue;

					lcVector3 Point2(0.0f, 0.0f, 0.0f);
					Point2[Axis1Idx / 2] = Axis1Idx % 2 ? -0.70710678118f : 0.70710678118f;
					Point2[Axis2Idx / 2] = Axis2Idx % 2 ? -0.70710678118f : 0.70710678118f;

					if (lcLengthSquared(Point2 - Intersection) < mHighlightRadius * mHighlightRadius)
					{
						IntersectionFlags.set(Axis1Idx);
						IntersectionFlags.set(Axis2Idx);
						return;
					}

					for (int Axis3Idx = 0; Axis3Idx < 6; Axis3Idx++)
					{
						if (Axis1Idx / 2 == Axis3Idx / 2 || Axis2Idx / 2 == Axis3Idx / 2)
							continue;

						lcVector3 Point3(0.0f, 0.0f, 0.0f);
						Point3[Axis1Idx / 2] = Axis1Idx % 2 ? -0.57735026919f : 0.57735026919f;
						Point3[Axis2Idx / 2] = Axis2Idx % 2 ? -0.57735026919f : 0.57735026919f;
						Point3[Axis3Idx / 2] = Axis3Idx % 2 ? -0.57735026919f : 0.57735026919f;

						if (lcLengthSquared(Point3 - Intersection) < mHighlightRadius * mHighlightRadius)
						{
							IntersectionFlags.set(Axis1Idx);
							IntersectionFlags.set(Axis2Idx);
							IntersectionFlags.set(Axis3Idx);
							return;
						}
					}
				}
			}
		};

		CheckIntersection();
	}

	return IntersectionFlags;
}
