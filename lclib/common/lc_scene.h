#pragma once

#include "lc_mesh.h"
#include "lc_array.h"

enum class lcRenderMeshState : int
{
	Default,
	Selected,
	Focused,
	Faded,
	Highlighted
};

struct lcRenderMesh
{
	lcMatrix44 WorldMatrix;
	lcMesh* Mesh;
	int ColorIndex;
	int LodIndex;
	lcRenderMeshState State;
};

struct lcTranslucentMeshInstance
{
	const lcMeshSection* Section;
	int RenderMeshIndex;
	float Distance;
};

class lcScene
{
public:
	lcScene();

	void SetActiveSubmodelInstance(lcPiece* ActiveSubmodelInstance, const lcMatrix44& ActiveSubmodelTransform)
	{
		mActiveSubmodelInstance = ActiveSubmodelInstance;
		mActiveSubmodelTransform = ActiveSubmodelTransform;
	}

	lcPiece* GetActiveSubmodelInstance() const
	{
		return mActiveSubmodelInstance;
	}

	const lcMatrix44& GetViewMatrix() const
	{
		return mViewMatrix;
	}

	void SetDrawInterface(bool DrawInterface)
	{
		mDrawInterface = DrawInterface;
	}

	bool GetDrawInterface() const
	{
		return mDrawInterface;
	}

	void SetShadingMode(lcShadingMode ShadingMode)
	{
		mShadingMode = ShadingMode;
	}

	void SetAllowLOD(bool AllowLOD)
	{
		mAllowLOD = AllowLOD;
	}

	void SetLODDistance(float Distance)
	{
		mMeshLODDistance = Distance;
	}

	void SetPreTranslucentCallback(std::function<void()> Callback)
	{
		mPreTranslucentCallback = Callback;
	}

	lcMatrix44 ApplyActiveSubmodelTransform(const lcMatrix44& WorldMatrix) const
	{
		return !mActiveSubmodelInstance ? WorldMatrix : lcMul(WorldMatrix, mActiveSubmodelTransform);
	}

	void Begin(const lcMatrix44& ViewMatrix);
	void End();
/*** LPub3D Mod - true fade ***/	
	void AddMesh(lcMesh* Mesh, const lcMatrix44& WorldMatrix, int ColorIndex, lcRenderMeshState State, bool LPubFade = false);
/*** LPub3D Mod end ***/

	void AddInterfaceObject(const lcObject* Object)
	{
		mInterfaceObjects.Add(Object);
	}

	void Draw(lcContext* Context) const;
	void DrawInterfaceObjects(lcContext* Context) const;

protected:
	void DrawOpaqueMeshes(lcContext* Context, bool DrawLit, int PrimitiveTypes, bool DrawFaded, bool DrawNonFaded) const;
/*** LPub3D Mod - true fade ***/
	void DrawTranslucentMeshes(lcContext* Context, bool DrawLit, bool DrawFadePrepass, bool DrawFaded, bool DrawNonFaded, int LPubFade = 0/*NoFade*/) const;
/*** LPub3D Mod end ***/
	void DrawDebugNormals(lcContext* Context, const lcMesh* Mesh) const;

	lcMatrix44 mViewMatrix;
	lcMatrix44 mActiveSubmodelTransform;
	lcPiece* mActiveSubmodelInstance;
	lcShadingMode mShadingMode;
	bool mDrawInterface;
	bool mAllowLOD;
	float mMeshLODDistance;

	lcVector4 mFadeColor;
	lcVector4 mHighlightColor;
	bool mHasFadedParts;
/*** LPub3D Mod - true fade ***/		
	bool mHasLPubFadedParts;
/*** LPub3D Mod end ***/	
	bool mTranslucentFade;

	std::function<void()> mPreTranslucentCallback;
	lcArray<lcRenderMesh> mRenderMeshes;
	lcArray<int> mOpaqueMeshes;
	lcArray<lcTranslucentMeshInstance> mTranslucentMeshes;
	lcArray<const lcObject*> mInterfaceObjects;
};
