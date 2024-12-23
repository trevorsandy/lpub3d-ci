#include "lc_global.h"
#include "lc_traintrack.h"
#include "lc_library.h"
#include "pieceinf.h"
#include "piece.h"
#include "lc_application.h"

// todo:
// auto replace cross when going over a straight section
// redo gizmo
// add cross to gizmo
// rotate around connections shortcut
// shortcuts for changing active connection
// move config to json
// add other track types
// macros to encode/decode mTrackToolSection

void lcTrainTrackInit(lcPiecesLibrary* Library)
{
	PieceInfo* Info = Library->FindPiece("74746.dat", nullptr, false, false);

	if (Info)
	{
		lcTrainTrackInfo* TrainTrackInfo = new lcTrainTrackInfo();

		TrainTrackInfo->AddConnection({lcMatrix44Translation(lcVector3(160.0f, 0.0f, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(LC_PI), lcVector3(-160.0f, 0.0f, 0.0f))});

		Info->SetTrainTrackInfo(TrainTrackInfo);
	}

	Info = Library->FindPiece("74747.dat", nullptr, false, false);

	if (Info)
	{
		lcTrainTrackInfo* TrainTrackInfo = new lcTrainTrackInfo();

		const float CurveX = sinf(LC_DTOR * 11.25f) * 800.0f;
		const float CurveY = (cosf(LC_DTOR * 11.25f) * 800.0f) - 800.0f;

		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(-11.25f * LC_DTOR), lcVector3(CurveX, CurveY, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(-168.75f * LC_DTOR), lcVector3(-CurveX, CurveY, 0.0f))});

		Info->SetTrainTrackInfo(TrainTrackInfo);
	}

	const float BranchX = 320.0f + 320.0f + (-(sinf(LC_DTOR * 22.5f) * 800.0f));
	const float BranchY = 320.0f + ((cosf(LC_DTOR * 22.5f) * 800.0f) - 800.0f);

	Info = Library->FindPiece("2861c04.dat", nullptr, false, false);

	if (Info)
	{
		lcTrainTrackInfo* TrainTrackInfo = new lcTrainTrackInfo();

		TrainTrackInfo->AddConnection({lcMatrix44Translation(lcVector3(320.0f, 0.0f, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(22.5f * LC_DTOR), lcVector3(BranchX, BranchY, 0.0f))});	
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(LC_PI), lcVector3(-320.0f, 0.0f, 0.0f))});

		Info->SetTrainTrackInfo(TrainTrackInfo);
	}

	Info = Library->FindPiece("2859c04.dat", nullptr, false, false);

	if (Info)
	{
		lcTrainTrackInfo* TrainTrackInfo = new lcTrainTrackInfo();

		TrainTrackInfo->AddConnection({lcMatrix44Translation(lcVector3(320.0f, 0.0f, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(-22.5f * LC_DTOR), lcVector3(BranchX, -BranchY, 0.0f))});	
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(LC_PI), lcVector3(-320.0f, 0.0f, 0.0f))});

		Info->SetTrainTrackInfo(TrainTrackInfo);
	}

    Info = Library->FindPiece("32087.dat", nullptr, false, false);

	if (Info)
	{
		lcTrainTrackInfo* TrainTrackInfo = new lcTrainTrackInfo();

		TrainTrackInfo->AddConnection({lcMatrix44Translation(lcVector3(160.0f, 0.0f, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(LC_PI / 2.0f), lcVector3(0.0f, 160.0f, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(LC_PI), lcVector3(-160.0f, 0.0f, 0.0f))});
		TrainTrackInfo->AddConnection({lcMatrix44(lcMatrix33RotationZ(-LC_PI / 2.0f), lcVector3(0.0f, -160.0f, 0.0f))});

		Info->SetTrainTrackInfo(TrainTrackInfo);
	}
}

std::pair<PieceInfo*, lcMatrix44> lcTrainTrackInfo::GetPieceInsertTransform(lcPiece* Piece, quint32 ConnectionIndex, lcTrainTrackType TrainTrackType) const
{
	if (ConnectionIndex >= mConnections.size())
		return { nullptr, lcMatrix44Identity() };

	const char* PieceNames[] =
	{
		"74746.dat",
		"74747.dat",
		"74747.dat",
		"2861c04.dat",
		"2859c04.dat"
	};

	PieceInfo* Info = lcGetPiecesLibrary()->FindPiece(PieceNames[static_cast<int>(TrainTrackType)], nullptr, false, false);

	if (!Info)
		return { nullptr, lcMatrix44Identity() };

	lcTrainTrackInfo* TrainTrackInfo = Info->GetTrainTrackInfo();

	if (!TrainTrackInfo || TrainTrackInfo->mConnections.empty())
		return { nullptr, lcMatrix44Identity() };

	lcMatrix44 Transform;

	if (TrainTrackType != lcTrainTrackType::Left)
		Transform = TrainTrackInfo->mConnections[0].Transform;
	else
	{
		Transform = lcMatrix44AffineInverse(TrainTrackInfo->mConnections[0].Transform);
		Transform = lcMul(Transform, lcMatrix44RotationZ(LC_PI));
	}

	Transform = lcMul(Transform, mConnections[ConnectionIndex].Transform);
	Transform = lcMul(Transform, Piece->mModelWorld);

	return { Info, Transform };
}

int lcTrainTrackInfo::GetPieceConnectionIndex(const lcPiece* Piece1, int ConnectionIndex1, const lcPiece* Piece2)
{
	const lcTrainTrackInfo* TrainTrackInfo1 = Piece1->mPieceInfo->GetTrainTrackInfo();
	const lcTrainTrackInfo* TrainTrackInfo2 = Piece2->mPieceInfo->GetTrainTrackInfo();

	const std::vector<lcTrainTrackConnection>& Connections2 = TrainTrackInfo2->GetConnections();
	lcMatrix44 Transform1 = lcMul(TrainTrackInfo1->GetConnections()[ConnectionIndex1].Transform, Piece1->mModelWorld);

	for (int ConnectionIndex2 = 0; ConnectionIndex2 < static_cast<int>(Connections2.size()); ConnectionIndex2++)
	{
		const lcTrainTrackConnection& Connection2 = Connections2[ConnectionIndex2];
		const lcMatrix44 Transform2 = lcMul(Connection2.Transform, Piece2->mModelWorld);

		if (lcLengthSquared(Transform1.GetTranslation() - Transform2.GetTranslation()) > 0.1f)
			continue;

		float Dot = lcDot3(Transform1[0], Transform2[0]);

		if (Dot < -0.99f && Dot > -1.01f)
			return ConnectionIndex2;
	}

	return -1;
}

std::optional<lcMatrix44> lcTrainTrackInfo::GetPieceInsertTransform(lcPiece* CurrentPiece, PieceInfo* Info)
{
	const lcTrainTrackInfo* CurrentTrackInfo = CurrentPiece->mPieceInfo->GetTrainTrackInfo();

	if (!CurrentTrackInfo || CurrentTrackInfo->GetConnections().empty())
		return std::nullopt;

	const quint32 FocusSection = CurrentPiece->GetFocusSection();
	quint32 ConnectionIndex = 0;

	if (FocusSection == LC_PIECE_SECTION_POSITION || FocusSection == LC_PIECE_SECTION_INVALID)
	{
		for (ConnectionIndex = 0; ConnectionIndex < CurrentTrackInfo->GetConnections().size(); ConnectionIndex++)
			if (!CurrentPiece->IsTrainTrackConnected(ConnectionIndex))
				break;
	}
	else
	{
		if (FocusSection < LC_PIECE_SECTION_TRAIN_TRACK_CONNECTION_FIRST)
			return std::nullopt;

		ConnectionIndex = FocusSection - LC_PIECE_SECTION_TRAIN_TRACK_CONNECTION_FIRST;
	}

	return GetConnectionTransform(CurrentPiece, ConnectionIndex, Info, 0);
}

std::optional<lcMatrix44> lcTrainTrackInfo::GetConnectionTransform(lcPiece* CurrentPiece, quint32 CurrentConnectionIndex, PieceInfo* Info, quint32 NewConnectionIndex)
{
	if (!CurrentPiece || !Info)
		return std::nullopt;

	const lcTrainTrackInfo* CurrentTrackInfo = CurrentPiece->mPieceInfo->GetTrainTrackInfo();

	if (!CurrentTrackInfo || CurrentTrackInfo->GetConnections().empty())
		return std::nullopt;

	if (CurrentConnectionIndex >= CurrentTrackInfo->GetConnections().size())
		return std::nullopt;

	lcTrainTrackInfo* NewTrackInfo = Info->GetTrainTrackInfo();

	if (!NewTrackInfo || NewConnectionIndex >= NewTrackInfo->mConnections.size())
		return std::nullopt;

	lcMatrix44 Transform;

//	if (TrainTrackType != lcTrainTrackType::Left)
//		Transform = NewTrackInfo->mConnections[NewConnectionIndex].Transform;
//	else
//	{
		Transform = lcMatrix44AffineInverse(NewTrackInfo->mConnections[NewConnectionIndex].Transform);
		Transform = lcMul(Transform, lcMatrix44RotationZ(LC_PI));
//	}

	Transform = lcMul(Transform, CurrentTrackInfo->GetConnections()[CurrentConnectionIndex].Transform);
	Transform = lcMul(Transform, CurrentPiece->mModelWorld);

	return Transform;
}
