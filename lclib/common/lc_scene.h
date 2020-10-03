#pragma once

#include "lc_mesh.h"
#include "lc_array.h"

enum class lcRenderMeshState : int
{
	NORMAL,
	SELECTED,
	FOCUSED,
	DISABLED,
	HIGHLIGHT
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

	void SetAllowWireframe(bool AllowWireframe)
	{
		mAllowWireframe = AllowWireframe;
	}

	lcMatrix44 ApplyActiveSubmodelTransform(const lcMatrix44& WorldMatrix) const
	{
		return !mActiveSubmodelInstance ? WorldMatrix : lcMul(WorldMatrix, mActiveSubmodelTransform);
	}

	void Begin(const lcMatrix44& ViewMatrix);
	void End();
	void AddMesh(lcMesh* Mesh, const lcMatrix44& WorldMatrix, int ColorIndex, lcRenderMeshState State);

	void AddInterfaceObject(const lcObject* Object)
	{
		mInterfaceObjects.Add(Object);
	}

	void Draw(lcContext* Context) const;
	void DrawInterfaceObjects(lcContext* Context) const;

protected:
	void DrawOpaqueMeshes(lcContext* Context, bool DrawLit, int PrimitiveTypes) const;
/*** LPub3D Mod - true fade ***/
	void DrawTranslucentMeshes(lcContext* Context, bool DrawLit, int FadeArgs = 0/*NoFade*/) const;
/*** LPub3D Mod end ***/
	void DrawDebugNormals(lcContext* Context, lcMesh* Mesh) const;

	lcMatrix44 mViewMatrix;
	lcMatrix44 mActiveSubmodelTransform;
	lcPiece* mActiveSubmodelInstance;
	bool mDrawInterface;
	bool mAllowWireframe;

	lcArray<lcRenderMesh> mRenderMeshes;
	lcArray<int> mOpaqueMeshes;
	lcArray<lcTranslucentMeshInstance> mTranslucentMeshes;
	lcArray<const lcObject*> mInterfaceObjects;
};
