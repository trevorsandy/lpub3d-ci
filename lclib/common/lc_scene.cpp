#include "lc_global.h"
#include "lc_scene.h"
#include "lc_context.h"
#include "pieceinf.h"
#include "lc_texture.h"
#include "lc_library.h"
#include "lc_application.h"
#include "object.h"

/*** LPub3D Mod - true fade ***/
enum lcFadeArgs {
	LC_NO_FADE,
	LC_DISABLE_COLOR_WRITES,
	LC_ENABLE_COLOR_WRITES,
	LC_DISABLE_BFC
};
/*** LPub3D Mod end ***/

lcScene::lcScene()
	: mRenderMeshes(0, 1024), mOpaqueMeshes(0, 1024), mTranslucentMeshes(0, 1024), mInterfaceObjects(0, 1024)
{
	mActiveSubmodelInstance = nullptr;
	mDrawInterface = false;
	mShadingMode = lcShadingMode::DefaultLights;
	mAllowLOD = true;
	mMeshLODDistance = 250.0f;
	mHasFadedParts = false;
/*** LPub3D Mod - true fade ***/
	mHasLPubFadedParts = false;
/*** LPub3D Mod end ***/

	mPreTranslucentCallback = nullptr;
}

void lcScene::Begin(const lcMatrix44& ViewMatrix)
{
	mViewMatrix = ViewMatrix;
	mActiveSubmodelInstance = nullptr;
	mPreTranslucentCallback = nullptr;
	mRenderMeshes.RemoveAll();
	mOpaqueMeshes.RemoveAll();
	mTranslucentMeshes.RemoveAll();
	mInterfaceObjects.RemoveAll();

	const lcPreferences& Preferences = lcGetPreferences();
	mHighlightColor = lcVector4FromColor(Preferences.mHighlightNewPartsColor);
	mFadeColor = lcVector4FromColor(Preferences.mFadeStepsColor);
	mHasFadedParts = false;
/*** LPub3D Mod - true fade ***/
	mHasLPubFadedParts = false;
/*** LPub3D Mod end ***/
	mTranslucentFade = mFadeColor.w != 1.0f;
}

void lcScene::End()
{
	const auto OpaqueMeshCompare = [this](int Index1, int Index2)
	{
		const lcMesh* Mesh1 = mRenderMeshes[Index1].Mesh;
		const lcMesh* Mesh2 = mRenderMeshes[Index2].Mesh;

		const int Texture1 = Mesh1->mFlags & lcMeshFlag::HasTexture;
		const int Texture2 = Mesh2->mFlags & lcMeshFlag::HasTexture;

		if (Texture1 == Texture2)
			return Mesh1 < Mesh2;

		return Texture1 ? false : true;
	};

	std::sort(mOpaqueMeshes.begin(), mOpaqueMeshes.end(), OpaqueMeshCompare);

	auto TranslucentMeshCompare = [](const lcTranslucentMeshInstance& Mesh1, const lcTranslucentMeshInstance& Mesh2)
	{
		return Mesh1.Distance > Mesh2.Distance;
	};

	std::sort(mTranslucentMeshes.begin(), mTranslucentMeshes.end(), TranslucentMeshCompare);
}

/*** LPub3D Mod - true fade ***/
void lcScene::AddMesh(lcMesh* Mesh, const lcMatrix44& WorldMatrix, int ColorIndex, lcRenderMeshState State, bool LPubFade)
/*** LPub3D Mod end ***/
{
	lcRenderMesh& RenderMesh = mRenderMeshes.Add();

	RenderMesh.WorldMatrix = WorldMatrix;
	RenderMesh.Mesh = Mesh;
	RenderMesh.ColorIndex = ColorIndex;
	RenderMesh.State = State;
	const float Distance = fabsf(lcMul31(WorldMatrix[3], mViewMatrix).z) - mMeshLODDistance;
	RenderMesh.LodIndex = mAllowLOD ? RenderMesh.Mesh->GetLodIndex(Distance) : LC_MESH_LOD_HIGH;

	const bool ForceTranslucent = (mTranslucentFade && State == lcRenderMeshState::Faded);
	const bool Translucent = lcIsColorTranslucent(size_t(ColorIndex)) || ForceTranslucent;
	const lcMeshFlags Flags = Mesh->mFlags;
	mHasFadedParts |= State == lcRenderMeshState::Faded;
/*** LPub3D Mod - true fade ***/
	mHasLPubFadedParts |= LPubFade;
/*** LPub3D Mod end ***/

	if ((Flags & (lcMeshFlag::HasSolid | lcMeshFlag::HasLines)) || ((Flags & lcMeshFlag::HasDefault) && !Translucent))
		mOpaqueMeshes.Add(mRenderMeshes.GetSize() - 1);

	if ((Flags & lcMeshFlag::HasTranslucent) || ((Flags & lcMeshFlag::HasDefault) && Translucent))
	{
		const lcMeshLod& Lod = Mesh->mLods[RenderMesh.LodIndex];

		for (int SectionIdx = 0; SectionIdx < Lod.NumSections; SectionIdx++)
		{
			const lcMeshSection* Section = &Lod.Sections[SectionIdx];

			if ((Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES)) == 0)
				continue;

			int SectionColorIndex = Section->ColorIndex;

			if (SectionColorIndex == gDefaultColor)
				SectionColorIndex = RenderMesh.ColorIndex;

			if (!lcIsColorTranslucent(size_t(SectionColorIndex)) && !ForceTranslucent)
				continue;

			const lcVector3 Center = (Section->BoundingBox.Min + Section->BoundingBox.Max) / 2;
			const float InstanceDistance = fabsf(lcMul31(lcMul31(Center, WorldMatrix), mViewMatrix).z);

			lcTranslucentMeshInstance& Instance = mTranslucentMeshes.Add();
			Instance.Section = Section;
			Instance.Distance = InstanceDistance;
			Instance.RenderMeshIndex = mRenderMeshes.GetSize() - 1;
		}
	}
}

void lcScene::DrawDebugNormals(lcContext* Context, const lcMesh* Mesh) const
{
	const lcVertex* const VertexBuffer = (lcVertex*)Mesh->mVertexData;
	lcVector3* const Vertices = (lcVector3*)malloc(Mesh->mNumVertices * 2 * sizeof(lcVector3));

	for (int VertexIdx = 0; VertexIdx < Mesh->mNumVertices; VertexIdx++)
	{
		Vertices[VertexIdx * 2] = VertexBuffer[VertexIdx].Position;
		Vertices[VertexIdx * 2 + 1] = VertexBuffer[VertexIdx].Position + lcUnpackNormal(VertexBuffer[VertexIdx].Normal);
	}

	Context->SetVertexBufferPointer(Vertices);
	Context->SetVertexFormatPosition(3);
	Context->DrawPrimitives(GL_LINES, 0, Mesh->mNumVertices * 2);
	free(Vertices);
}

void lcScene::DrawOpaqueMeshes(lcContext* Context, bool DrawLit, int PrimitiveTypes, bool DrawFaded, bool DrawNonFaded) const
{
	if (mOpaqueMeshes.IsEmpty())
		return;

	lcMaterialType FlatMaterial, TexturedMaterial;

	if (DrawLit)
	{
		FlatMaterial = lcMaterialType::FakeLitColor;
		TexturedMaterial = lcMaterialType::FakeLitTextureDecal;
	}
	else
	{
		FlatMaterial = lcMaterialType::UnlitColor;
		TexturedMaterial = lcMaterialType::UnlitTextureDecal;
	}

	Context->SetPolygonOffset(lcPolygonOffset::Opaque);

	for (const int MeshIndex : mOpaqueMeshes)
	{
		const lcRenderMesh& RenderMesh = mRenderMeshes[MeshIndex];
		const lcMesh* Mesh = RenderMesh.Mesh;
		const int LodIndex = RenderMesh.LodIndex;

		if (!DrawFaded && RenderMesh.State == lcRenderMeshState::Faded)
			continue;

		if (!DrawNonFaded && RenderMesh.State != lcRenderMeshState::Faded)
			continue;

		Context->BindMesh(Mesh);
		Context->SetWorldMatrix(RenderMesh.WorldMatrix);

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[LodIndex].NumSections; SectionIdx++)
		{
			const lcMeshSection* const Section = &Mesh->mLods[LodIndex].Sections[SectionIdx];

			if ((Section->PrimitiveType & PrimitiveTypes) == 0)
				continue;

			int ColorIndex = Section->ColorIndex;

			if (Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES))
			{
				if (ColorIndex == gDefaultColor)
					ColorIndex = RenderMesh.ColorIndex;

				if (lcIsColorTranslucent(size_t(ColorIndex)))   /*** LPub3D Mod - Suppress int -> size_t warning ***/
					continue;

				switch (RenderMesh.State)
				{
				case lcRenderMeshState::Default:
				case lcRenderMeshState::Highlighted:
					Context->SetColorIndex(ColorIndex);
					break;

				case lcRenderMeshState::Selected:
 /*** LPub3D Mod - Selected Parts ***/
					if (gApplication->mPreferences.mBuildModificationEnabled)
						Context->SetColorIndexTinted(ColorIndex, LC_COLOR_BM_SELECTED, 0.5f);
					else
						Context->SetColorIndexTinted(ColorIndex, LC_COLOR_SELECTED, 0.5f);
/*** LPub3D Mod end ***/

					break;

				case lcRenderMeshState::Focused:
					Context->SetColorIndexTinted(ColorIndex, LC_COLOR_FOCUSED, 0.5f);
					break;

				case lcRenderMeshState::Faded:
					if (mTranslucentFade)
						continue;
					Context->SetColorIndexTinted(ColorIndex, mFadeColor);
					break;
				}
			}
			else if (Section->PrimitiveType & (LC_MESH_LINES | LC_MESH_CONDITIONAL_LINES))
			{
				switch (RenderMesh.State)
				{
				case lcRenderMeshState::Default:
					if (mShadingMode != lcShadingMode::Wireframe)
					{
						if (ColorIndex != gEdgeColor)
							Context->SetColorIndex(ColorIndex);
						else
							Context->SetEdgeColorIndex(RenderMesh.ColorIndex);
					}
					else
					{
						if (ColorIndex == gEdgeColor)
							ColorIndex = RenderMesh.ColorIndex;

						Context->SetColorIndex(ColorIndex);
					}
					break;

				case lcRenderMeshState::Selected:
/*** LPub3D Mod - Selected Parts ***/
				   if (gApplication->mPreferences.mBuildModificationEnabled)
					   Context->SetInterfaceColor(LC_COLOR_BM_SELECTED);
				   else
					   Context->SetInterfaceColor(LC_COLOR_SELECTED);
/*** LPub3D Mod end ***/
					break;

				case lcRenderMeshState::Focused:
					Context->SetInterfaceColor(LC_COLOR_FOCUSED);
					break;

				case lcRenderMeshState::Highlighted:
					Context->SetColor(mHighlightColor);
					break;

				case lcRenderMeshState::Faded:
					Context->SetEdgeColorIndexTinted(ColorIndex, mFadeColor);
					break;
				}

				if (Section->PrimitiveType == LC_MESH_CONDITIONAL_LINES)
				{
					int VertexBufferOffset = Mesh->mVertexCacheOffset != -1 ? Mesh->mVertexCacheOffset : 0;
					VertexBufferOffset += Mesh->mNumVertices * sizeof(lcVertex) + Mesh->mNumTexturedVertices * sizeof(lcVertexTextured);
					const int IndexBufferOffset = Mesh->mIndexCacheOffset != -1 ? Mesh->mIndexCacheOffset : 0;

					Context->SetMaterial(lcMaterialType::UnlitColorConditional);
					Context->SetVertexFormatConditional(VertexBufferOffset);

					Context->DrawIndexedPrimitives(GL_LINES, Section->NumIndices, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset);

					continue;
				}
			}

			lcTexture* const Texture = Section->Texture;
			int VertexBufferOffset = Mesh->mVertexCacheOffset != -1 ? Mesh->mVertexCacheOffset : 0;
			const int IndexBufferOffset = Mesh->mIndexCacheOffset != -1 ? Mesh->mIndexCacheOffset : 0;

			if (!Texture)
			{
				Context->SetMaterial(FlatMaterial);
				Context->SetVertexFormat(VertexBufferOffset, 3, 1, 0, 0, DrawLit);
			}
			else
			{
				if (Texture->NeedsUpload())
					Texture->Upload(Context);
				Context->SetMaterial(TexturedMaterial);
				VertexBufferOffset += Mesh->mNumVertices * sizeof(lcVertex);
				Context->SetVertexFormat(VertexBufferOffset, 3, 1, 2, 0, DrawLit);
				Context->BindTexture2D(Texture->mTexture);
			}

			const GLenum DrawPrimitiveType = Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES) ? GL_TRIANGLES : GL_LINES;
			Context->DrawIndexedPrimitives(DrawPrimitiveType, Section->NumIndices, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset);
		}

#ifdef LC_DEBUG_NORMALS
		DrawDebugNormals(Context, Mesh);
#endif
	}

	Context->BindTexture2D(0);
	Context->SetPolygonOffset(lcPolygonOffset::None);
}

/*** LPub3D Mod - true fade ***/
void lcScene::DrawTranslucentMeshes(lcContext* Context, bool DrawLit, bool DrawFadePrepass, bool DrawFaded, bool DrawNonFaded, int LPubFade) const
/*** LPub3D Mod end ***/
{
	if (mTranslucentMeshes.IsEmpty())
		return;

	lcMaterialType FlatMaterial, TexturedMaterial;

	if (DrawLit)
	{
		FlatMaterial = lcMaterialType::FakeLitColor;
		TexturedMaterial = lcMaterialType::FakeLitTextureDecal;
	}
	else
	{
		FlatMaterial = lcMaterialType::UnlitColor;
		TexturedMaterial = lcMaterialType::UnlitTextureDecal;
	}

/*** LPub3D Mod - true fade ***/
	if (LPubFade)
	{
		// Enable BFC
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// Disable color writes
		if (LPubFade == LC_DISABLE_COLOR_WRITES){
			glDisable(GL_BLEND);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		}

		// Enable color writes
		else {
			glEnable(GL_BLEND);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}

		Context->SetDepthWrite(LPubFade == LC_DISABLE_COLOR_WRITES);
	}
	else
/*** LPub3D Mod end ***/
	if (!DrawFadePrepass)
	{
		glEnable(GL_BLEND);
		Context->SetDepthWrite(false);
	}
	else
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	Context->SetPolygonOffset(lcPolygonOffset::Translucent);

	for (const lcTranslucentMeshInstance& MeshInstance : mTranslucentMeshes)
	{
		const lcRenderMesh& RenderMesh = mRenderMeshes[MeshInstance.RenderMeshIndex];
		const lcMesh* Mesh = RenderMesh.Mesh;

		if (!DrawFaded && RenderMesh.State == lcRenderMeshState::Faded)
			continue;

		if (!DrawNonFaded && RenderMesh.State != lcRenderMeshState::Faded)
			continue;

		Context->BindMesh(Mesh);
		Context->SetWorldMatrix(RenderMesh.WorldMatrix);

		const lcMeshSection* Section = MeshInstance.Section;

		int ColorIndex = Section->ColorIndex;

		if (ColorIndex == gDefaultColor)
			ColorIndex = RenderMesh.ColorIndex;

/*** LPub3D Mod - true fade ***/
		if (!LPubFade && DrawFadePrepass && lcIsColorTranslucent(ColorIndex))
			continue;
/*** LPub3D Mod end ***/

		switch (RenderMesh.State)
		{
		case lcRenderMeshState::Default:
		case lcRenderMeshState::Highlighted:
			Context->SetColorIndex(ColorIndex);
			break;

		case lcRenderMeshState::Selected:
			Context->SetColorIndexTinted(ColorIndex, LC_COLOR_SELECTED, 0.5f);
			break;

		case lcRenderMeshState::Focused:
			Context->SetColorIndexTinted(ColorIndex, LC_COLOR_FOCUSED, 0.5f);
			break;

		case lcRenderMeshState::Faded:
			Context->SetColorIndexTinted(ColorIndex, mFadeColor);
			break;
		}

		lcTexture* const Texture = Section->Texture;
		int VertexBufferOffset = Mesh->mVertexCacheOffset != -1 ? Mesh->mVertexCacheOffset : 0;
		const int IndexBufferOffset = Mesh->mIndexCacheOffset != -1 ? Mesh->mIndexCacheOffset : 0;

		if (!Texture)
		{
			Context->SetMaterial(FlatMaterial);
			Context->SetVertexFormat(VertexBufferOffset, 3, 1, 0, 0, DrawLit);
		}
		else
		{
			if (Texture->NeedsUpload())
				Texture->Upload(Context);
			Context->SetMaterial(TexturedMaterial);
			VertexBufferOffset += Mesh->mNumVertices * sizeof(lcVertex);
			Context->SetVertexFormat(VertexBufferOffset, 3, 1, 2, 0, DrawLit);
			Context->BindTexture2D(Texture->mTexture);
		}

		const GLenum DrawPrimitiveType = Section->PrimitiveType & (LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES) ? GL_TRIANGLES : GL_LINES;
		Context->DrawIndexedPrimitives(DrawPrimitiveType, Section->NumIndices, Mesh->mIndexType, IndexBufferOffset + Section->IndexOffset);

#ifdef LC_DEBUG_NORMALS
		DrawDebugNormals(Context, Mesh);
#endif
	}

	Context->BindTexture2D(0);
	Context->SetPolygonOffset(lcPolygonOffset::None);

/*** LPub3D Mod - true fade ***/
	if (LPubFade)
	{
		Context->SetDepthWrite(true);
		glDisable(GL_BLEND);

		// Wrap up, Disable BFC
		glDisable(GL_CULL_FACE);
	}
	else
/*** LPub3D Mod end ***/
	if (!DrawFadePrepass)
	{
		Context->SetDepthWrite(true);
		glDisable(GL_BLEND);
	}
	else
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void lcScene::Draw(lcContext* Context) const
{
	// TODO: find a better place for these updates
	lcGetPiecesLibrary()->UpdateBuffers(Context);

	Context->SetViewMatrix(mViewMatrix);

	const lcPreferences& Preferences = lcGetPreferences();
	const bool DrawLines = Preferences.mDrawEdgeLines && Preferences.mLineWidth > 0.0f;
	const bool DrawConditional = Preferences.mDrawConditionalLines && Preferences.mLineWidth > 0.0f;
/*** LPub3D Mod - true fade ***/
	const bool LPubTrueFade = gApplication->LPubFadeSteps() && // to turn off during HTML Steps export
							  gApplication->mPreferences.mLPubTrueFade &&
							  mHasLPubFadedParts &&
							  !mTranslucentMeshes.IsEmpty();
/*** LPub3D Mod end ***/

//	lcShadingMode ShadingMode = Preferences.mShadingMode;
//	if (ShadingMode == lcShadingMode::Wireframe && !mAllowWireframe)
//		ShadingMode = lcShadingMode::Flat;

	if (mShadingMode == lcShadingMode::Wireframe)
	{
		int PrimitiveTypes = LC_MESH_LINES;

		if (DrawConditional)
			PrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;

		DrawOpaqueMeshes(Context, false, PrimitiveTypes, true, true);

		if (mPreTranslucentCallback)
			mPreTranslucentCallback();
	}
	else if (mShadingMode == lcShadingMode::Flat)
	{
		int LinePrimitiveTypes = 0;

		if (DrawLines)
		{
			LinePrimitiveTypes |= LC_MESH_LINES;

			if (DrawConditional)
				LinePrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;
		}

		const int SolidPrimitiveTypes = LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES;


/*** LPub3D Mod - true fade ***/
		if (LPubTrueFade) // Fade - Flat
		{
			// 1. Draw opaque mesh triangles - without mesh lines
			DrawOpaqueMeshes(Context, false, SolidPrimitiveTypes, false, true);

			// 2. Disable color writes to only update the Z buffer
			DrawTranslucentMeshes(Context, true, false, false, true, LC_DISABLE_COLOR_WRITES);

			if (mPreTranslucentCallback)
				mPreTranslucentCallback();

			// 3. Enable color writes to draw translucent mesh triangles normally
			DrawTranslucentMeshes(Context, true, false, true, true, LC_ENABLE_COLOR_WRITES);

			// 4. Draw mesh lines
			if (DrawLines)
				// 5. Draw opaque unlit mesh lines
				DrawOpaqueMeshes(Context, false, SolidPrimitiveTypes | LinePrimitiveTypes, true, true);
		}                 // Fade - Flat
		else
/*** LPub3D Mod end ***/
		if (mTranslucentFade && mHasFadedParts)
		{
			DrawOpaqueMeshes(Context, false, SolidPrimitiveTypes | LinePrimitiveTypes, false, true);

			if (mPreTranslucentCallback)
				mPreTranslucentCallback();

			DrawTranslucentMeshes(Context, false, true, true, false);

			if (DrawLines)
				DrawOpaqueMeshes(Context, false, LinePrimitiveTypes, true, false);

			DrawTranslucentMeshes(Context, false, false, true, true);
		}
		else
		{
			DrawOpaqueMeshes(Context, false, SolidPrimitiveTypes | LinePrimitiveTypes, true, true);

			if (mPreTranslucentCallback)
				mPreTranslucentCallback();

			DrawTranslucentMeshes(Context, false, false, true, true);
		}
	}
	else
	{
		int LinePrimitiveTypes = LC_MESH_LINES;

		if (DrawConditional)
			LinePrimitiveTypes |= LC_MESH_CONDITIONAL_LINES;

/*** LPub3D Mod - true fade ***/
		const int SolidPrimitiveTypes = LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES;

		if (LPubTrueFade)           // Fade - Default
		{

			// 1. Draw opaque mesh triangles - without mesh lines
			DrawOpaqueMeshes(Context, true, SolidPrimitiveTypes , false, true);

			// 2. Disable color writes to only update the Z buffer
			DrawTranslucentMeshes(Context, true, false, false, true, LC_DISABLE_COLOR_WRITES);

			if (mPreTranslucentCallback)
				 mPreTranslucentCallback();

			// 3. Enable color writes to draw translucent mesh triangles normally
			DrawTranslucentMeshes(Context, true, false, true, true, LC_ENABLE_COLOR_WRITES);

			// 4. Draw mesh lines
			if (DrawLines)
			{
				// 5. Draw translucent unlit mesh lines
				DrawTranslucentMeshes(Context, false, false, true, true, LC_DISABLE_BFC);

				// 6. Draw opaque unlit mesh lines
				DrawOpaqueMeshes(Context, false, SolidPrimitiveTypes | LinePrimitiveTypes, true, true);
			}
		}                // Fade - Default
		else
/*** LPub3D Mod end ***/
		if (mTranslucentFade && mHasFadedParts)
		{
			DrawOpaqueMeshes(Context, true, LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES, false, true);

			if (DrawLines)
				DrawOpaqueMeshes(Context, false, LinePrimitiveTypes, false, true);

			if (mPreTranslucentCallback)
				mPreTranslucentCallback();

			DrawTranslucentMeshes(Context, false, true, true, false);

			if (DrawLines)
				DrawOpaqueMeshes(Context, false, LinePrimitiveTypes, true, false);

			DrawTranslucentMeshes(Context, true, false, true, true);
		}
		else
		{
			if (DrawLines)
				DrawOpaqueMeshes(Context, false, LinePrimitiveTypes, true, true);

			DrawOpaqueMeshes(Context, true, LC_MESH_TRIANGLES | LC_MESH_TEXTURED_TRIANGLES, true, true);

			if (mPreTranslucentCallback)
				mPreTranslucentCallback();

			DrawTranslucentMeshes(Context, true, false, true, true);
		}
	}
}

void lcScene::DrawInterfaceObjects(lcContext* Context) const
{
	for (const lcObject* Object : mInterfaceObjects)
		Object->DrawInterface(Context, *this);
}
