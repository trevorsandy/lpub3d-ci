#include "lc_global.h"
#include "lc_timelinewidget.h"
#include "lc_model.h"
#include "piece.h"
#include "pieceinf.h"
#include "lc_mainwindow.h"
#include "lc_viewwidget.h"
#include "lc_previewwidget.h"
/*** LPub3D Mod - set Timeline top item ***/
#include "project.h"
/*** LPub3D Mod end ***/

lcTimelineWidget::lcTimelineWidget(QWidget* Parent)
	: QTreeWidget(Parent)
{
	mCurrentStepItem = nullptr;
	mIgnoreUpdates = false;

	setSelectionMode(QAbstractItemView::ExtendedSelection);
/*** LPub3D Mod - suppress item drag as there is only one step loaded at a time***/
	setDragEnabled(false);
/*** LPub3D Mod end ***/
	setDragDropMode(QAbstractItemView::InternalMove);
	setUniformRowHeights(true);
	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);

	invisibleRootItem()->setFlags(invisibleRootItem()->flags() & ~Qt::ItemIsDropEnabled);

	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(CurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
	connect(this, SIGNAL(itemSelectionChanged()), SLOT(ItemSelectionChanged()));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(CustomMenuRequested(QPoint)));
}

lcTimelineWidget::~lcTimelineWidget()
{
}

void lcTimelineWidget::CustomMenuRequested(QPoint Pos)
{
	QMenu* Menu = new QMenu(this);

	lcObject* FocusObject = gMainWindow->GetActiveModel()->GetFocusObject();

	if (FocusObject && FocusObject->IsPiece())
	{
		lcPiece* Piece = (lcPiece*)FocusObject;

		if (Piece->mPieceInfo->IsModel())
		{
			Menu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]);
/*** LPub3D Mod - suppress multi-step context menu items ***/
/***
			Menu->addAction(gMainWindow->mActions[LC_PIECE_VIEW_SELECTED_MODEL]);
			Menu->addAction(gMainWindow->mActions[LC_PIECE_INLINE_SELECTED_MODELS]);
***/
/*** LPub3D Mod end ***/
			Menu->addSeparator();
		}
	}
/*** LPub3D Mod - suppress multi-step context menu items ***/
/***
	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_SET_CURRENT]);
	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_INSERT_BEFORE]);
	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_INSERT_AFTER]);
	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_DELETE]);

	Menu->addSeparator();

	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_MOVE_SELECTION]);
	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_MOVE_SELECTION_BEFORE]);
	Menu->addAction(gMainWindow->mActions[LC_TIMELINE_MOVE_SELECTION_AFTER]);

	Menu->addSeparator();
***/
/*** LPub3D Mod end ***/
	Menu->addAction(gMainWindow->mActions[LC_PIECE_HIDE_SELECTED]);
	Menu->addAction(gMainWindow->mActions[LC_PIECE_HIDE_UNSELECTED]);
	Menu->addAction(gMainWindow->mActions[LC_PIECE_UNHIDE_SELECTED]);
	Menu->addAction(gMainWindow->mActions[LC_PIECE_UNHIDE_ALL]);

	Menu->exec(viewport()->mapToGlobal(Pos));
	delete Menu;
}

void lcTimelineWidget::Update(bool Clear, bool UpdateItems)
{
	if (mIgnoreUpdates)
		return;

	lcModel* Model = gMainWindow->GetActiveModel();

	bool Blocked = blockSignals(true);

	if (!Model)
	{
		mCurrentStepItem = nullptr;
		mItems.clear();
		clear();
		blockSignals(Blocked);
		return;
	}

	if (Clear)
	{
		mCurrentStepItem = nullptr;
		mItems.clear();
		clear();
	}

	lcStep LastStep = Model->GetLastStep();
/*** LPub3D Mod - Disable always showing an extra step ***/
/***
	if (Model->HasPieces())
		LastStep++;
***/
/*** LPub3D Mod end ***/
	LastStep  = lcMax(LastStep, Model->GetCurrentStep());

	for (int TopLevelItemIdx = LastStep; TopLevelItemIdx < topLevelItemCount(); )
	{
		QTreeWidgetItem* StepItem = topLevelItem(TopLevelItemIdx);

		while (StepItem->childCount())
		{
			QTreeWidgetItem* PieceItem = StepItem->child(0);
			lcPiece* Piece = (lcPiece*)PieceItem->data(0, Qt::UserRole).value<uintptr_t>();
			mItems.remove(Piece);
			delete PieceItem;
		}

		if (mCurrentStepItem == StepItem)
			mCurrentStepItem = nullptr;

		delete StepItem;
	}

	for (unsigned int TopLevelItemIdx = topLevelItemCount(); TopLevelItemIdx < LastStep; TopLevelItemIdx++)
	{
/*** LPub3D Mod - set Timeline top item to loaded model name when loading single step ***/
		QString TimelineTopItem = tr("Step %1").arg(TopLevelItemIdx + 1);
		if (LastStep == 1)
			TimelineTopItem = lcGetActiveProject()->GetTimelineTopItemString();
		QTreeWidgetItem* StepItem = new QTreeWidgetItem(this, QStringList(TimelineTopItem));
		StepItem->setData(0, Qt::UserRole, QVariant::fromValue(int(TopLevelItemIdx) + 1));
/*** LPub3D Mod end ***/
		StepItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
		addTopLevelItem(StepItem);
		StepItem->setExpanded(true);
	}

	const std::vector<std::unique_ptr<lcPiece>>& Pieces = Model->GetPieces();
	QTreeWidgetItem* StepItem = nullptr;
	int PieceItemIndex = 0;
	lcStep Step = 0;

	for (const std::unique_ptr<lcPiece>& Piece : Pieces)
	{
		while (Step != Piece->GetStepShow())
		{
			if (StepItem)
			{
				while (PieceItemIndex < StepItem->childCount())
				{
					QTreeWidgetItem* PieceItem = StepItem->child(PieceItemIndex);
					lcPiece* RemovePiece = (lcPiece*)PieceItem->data(0, Qt::UserRole).value<uintptr_t>();
					auto RemoveIt = std::find_if(Pieces.begin(), Pieces.end(), [RemovePiece](const std::unique_ptr<lcPiece>& CheckPiece){ return CheckPiece.get() == RemovePiece; });

					if (RemoveIt == Pieces.end())
					{
						mItems.remove(RemovePiece);
						delete PieceItem;
					}
					else
					{
						PieceItem->parent()->removeChild(PieceItem);
						topLevelItem(RemovePiece->GetStepShow() - 1)->addChild(PieceItem);
					}
				}
			}

			Step++;
			StepItem = topLevelItem(Step - 1);
			PieceItemIndex = 0;
		}

		QTreeWidgetItem* PieceItem = mItems.value(Piece.get());
		bool UpdateItem = UpdateItems;

		if (StepItem)
		{
			if (!PieceItem)
			{
				PieceItem = new QTreeWidgetItem();
				PieceItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
				PieceItem->setData(0, Qt::UserRole, QVariant::fromValue<uintptr_t>((uintptr_t)Piece.get()));
				StepItem->insertChild(PieceItemIndex, PieceItem);
				mItems[Piece.get()] = PieceItem;

				UpdateItem = true;
			}
			else
			{
				if (PieceItemIndex >= StepItem->childCount() || PieceItem != StepItem->child(PieceItemIndex))
				{
					QTreeWidgetItem* PieceParent = PieceItem->parent();

					if (PieceParent)
						PieceParent->removeChild(PieceItem);

					StepItem->insertChild(PieceItemIndex, PieceItem);
				}
			}
		}

		if (UpdateItem)
		{
			PieceItem->setText(0, Piece->mPieceInfo->m_strDescription);

			int ColorIndex = Piece->GetColorIndex();
/*** LPub3D Mod - Timeline part icons ***/
			int Size = rowHeight(indexFromItem(PieceItem));

			QFileInfo p = QFileInfo(Piece->GetID());
			bool fPiece = (p.completeBaseName().right(4) == QString(LPUB3D_COLOUR_FADE_SUFFIX));
			bool hPiece = (p.completeBaseName().right(9) == QString(LPUB3D_COLOUR_HIGHLIGHT_SUFFIX));
			QString pieceName = p.completeBaseName().left(p.completeBaseName().size() - (fPiece ? 5 : hPiece ? 10 : 0)).append("." + p.suffix());

			bool IsModel = gMainWindow->IsLPub3DSubModel(pieceName);
			int IconIndex = IsModel ? SUBMODEL_ICON_INDEX_BASE + ColorIndex : ColorIndex;

			if (lcGetPreferences().mViewPieceIcons && gMainWindow->mSubmodelIconsLoaded) {

				bool UseFColor = gApplication->UseLPubFadeColour();
				bool Use0Code = IsModel && (hPiece || (fPiece && !gApplication->UseLPubFadeColour()) || (!hPiece && !fPiece));

				QString colorCode = fPiece && UseFColor ? gApplication->LPubFadeColour() : QString("%1").arg(Piece->GetColorCode());
				QString colorPrefix = IsModel ? fPiece ? LPUB3D_COLOUR_FADE_PREFIX : hPiece ? LPUB3D_COLOUR_HIGHLIGHT_PREFIX : QString() : fPiece && UseFColor ? LPUB3D_COLOUR_FADE_PREFIX : QString();
				QString ImageKey = QString("%1_%2").arg(p.completeBaseName().toLower(), QString("%1%2").arg(colorPrefix, Use0Code ? QString("0") : colorCode));

				if (GetPieceIcon(Size, ImageKey)) {

					PieceItem->setIcon(0, mPieceIcons[ImageKey]);

				} else {

					GetIcon(Size,ColorIndex,IsModel);
					PieceItem->setIcon(0, mIcons[IconIndex]);
//#ifdef QT_DEBUG_MODE
//					qDebug() << qPrintable(QString("ALERT - Could Not Insert %1 Icon - UID [%2]")
//												   .arg(IsModel ? "Submodel" : "Piece")
//												   .arg(ImageKey));
//#endif
				}

			} else {

				GetIcon(Size,ColorIndex,IsModel);
				PieceItem->setIcon(0, mIcons[IconIndex]);

			}
/*** LPub3D Mod end ***/

			QColor Color = palette().text().color();

			if (Piece->mPieceInfo->IsPlaceholder())
				Color = QColor(208, 0, 0);
/*** LPub3D Mod - Set color only if hidden otherwise use default ***/
			else if (Piece->IsHidden()) 
			{
				Color = PieceItem->foreground(0).color();
				Color.setAlpha(128);
			}
			PieceItem->setForeground(0, Color);
/*** LPub3D Mod end ***/
		}

		PieceItem->setSelected(Piece->IsSelected());
		PieceItemIndex++;
	}

	if (Step == 0)
	{
		Step = 1;
		StepItem = topLevelItem(0);
	}

	while (Step <= LastStep && StepItem)
	{
		while (PieceItemIndex < StepItem->childCount())
		{
			QTreeWidgetItem* PieceItem = StepItem->child(PieceItemIndex);
			lcPiece* RemovePiece = (lcPiece*)PieceItem->data(0, Qt::UserRole).value<uintptr_t>();

			mItems.remove(RemovePiece);
			delete PieceItem;
		}

		Step++;
		StepItem = topLevelItem(Step - 1);
		PieceItemIndex = 0;
	}

	UpdateCurrentStepItem();

	blockSignals(Blocked);
}

void lcTimelineWidget::UpdateCurrentStepItem()
{
	lcModel* Model = gMainWindow->GetActiveModel();
	lcStep CurrentStep = Model->GetCurrentStep();
	QTreeWidgetItem* CurrentStepItem = topLevelItem(CurrentStep - 1);

	if (CurrentStepItem != mCurrentStepItem)
	{
		if (mCurrentStepItem)
		{
			QFont Font = mCurrentStepItem->font(0);
			Font.setBold(false);
			mCurrentStepItem->setFont(0, Font);
		}

		if (CurrentStepItem)
		{
			QFont Font = CurrentStepItem->font(0);
			Font.setBold(true);
			CurrentStepItem->setFont(0, Font);
			setCurrentItem(CurrentStepItem);
		}

		mCurrentStepItem = CurrentStepItem;
	}
}

/*** LPub3D Mod - Timeline part icons ***/
void lcTimelineWidget::GetIcon(int Size, int ColorIndex, bool IsModel) {

	int IconIndex = IsModel ? SUBMODEL_ICON_INDEX_BASE + ColorIndex : ColorIndex;

	if (!mIcons.contains(IconIndex))
	{


		QImage Image(Size, Size, QImage::Format_ARGB32);
		Image.fill(0);
		const lcVector4& Color = gColorList[ColorIndex].Value;
		QPainter Painter(&Image);
		Painter.setPen(Qt::darkGray);
		Painter.setBrush(QColor::fromRgbF(Color[0], Color[1], Color[2]));
		IsModel ?
		Painter.drawRect(0, 0, Size - 1, Size - 1) :
		Painter.drawEllipse(0, 0, Size - 1, Size - 1);

		mIcons[IconIndex] = QIcon(QPixmap::fromImage(Image));
	}
}

bool lcTimelineWidget::GetPieceIcon(int Size, QString ImageKey) {

	if (!mPieceIcons.contains(ImageKey))
	{
		QFileInfo iconFile(gMainWindow->GetPliIconsPath(ImageKey));

		if (!iconFile.exists())
			return false;

		QImage RawImage(iconFile.absoluteFilePath());
		RawImage = RawImage.convertToFormat(QImage::Format_ARGB32);

		QImage Image = RawImage.scaled(Size, Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		QPainter Painter(&Image);
		Painter.setRenderHints(QPainter::Antialiasing,true);

		mPieceIcons[ImageKey] = QIcon(QPixmap::fromImage(Image));
	}
	return true;
}
/*** LPub3D Mod end ***/

void lcTimelineWidget::UpdateSelection()
{
	if (mIgnoreUpdates)
		return;

	QItemSelection ItemSelection;

	for (int TopLevelItemIdx = 0; TopLevelItemIdx < topLevelItemCount(); TopLevelItemIdx++)
	{
		QTreeWidgetItem* StepItem = topLevelItem(TopLevelItemIdx);

		for (int PieceItemIdx = 0; PieceItemIdx < StepItem->childCount(); PieceItemIdx++)
		{
			QTreeWidgetItem* PieceItem = StepItem->child(PieceItemIdx);
			lcPiece* Piece = (lcPiece*)PieceItem->data(0, Qt::UserRole).value<uintptr_t>();

			if (Piece && Piece->IsSelected())
			{
				QModelIndex Index = indexFromItem(PieceItem);
				ItemSelection.select(Index, Index);
			}
		}
	}

	bool Blocked = blockSignals(true);

	selectionModel()->select(ItemSelection, QItemSelectionModel::ClearAndSelect);

	blockSignals(Blocked);
}

void lcTimelineWidget::InsertStepBefore()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;

	gMainWindow->GetActiveModel()->InsertStep(Step + 1);
}

void lcTimelineWidget::InsertStepAfter()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;

	gMainWindow->GetActiveModel()->InsertStep(Step + 2);
}

void lcTimelineWidget::RemoveStep()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;

	gMainWindow->GetActiveModel()->RemoveStep(Step + 1);
}

void lcTimelineWidget::MoveSelection()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;
	Step++;

	QList<QTreeWidgetItem*> SelectedItems = selectedItems();

	for (QTreeWidgetItem* PieceItem : SelectedItems)
	{
		QTreeWidgetItem* Parent = PieceItem->parent();

		if (!Parent)
			continue;

		int ChildIndex = Parent->indexOfChild(PieceItem);
		CurrentItem->addChild(Parent->takeChild(ChildIndex));
	}

	UpdateModel();

	lcModel* Model = gMainWindow->GetActiveModel();

	if (Step > static_cast<int>(Model->GetCurrentStep()))
		Model->SetCurrentStep(Step);
}

void lcTimelineWidget::MoveSelectionBefore()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;

	Step++;

	QList<QTreeWidgetItem*> SelectedItems = selectedItems();

	gMainWindow->GetActiveModel()->InsertStep(Step);

	CurrentItem = topLevelItem(Step - 1);

	for (QTreeWidgetItem* PieceItem : SelectedItems)
	{
		QTreeWidgetItem* Parent = PieceItem->parent();

		if (!Parent)
			continue;

		int ChildIndex = Parent->indexOfChild(PieceItem);
		CurrentItem->addChild(Parent->takeChild(ChildIndex));
	}

	UpdateModel();

	lcModel* Model = gMainWindow->GetActiveModel();

	if (Step > static_cast<int>(Model->GetCurrentStep()))
		Model->SetCurrentStep(Step);
}

void lcTimelineWidget::MoveSelectionAfter()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;

	Step += 2;

	QList<QTreeWidgetItem*> SelectedItems = selectedItems();

	gMainWindow->GetActiveModel()->InsertStep(Step);

	for (int TopLevelItemIdx = topLevelItemCount(); TopLevelItemIdx < Step; TopLevelItemIdx++)
	{
		QTreeWidgetItem* StepItem = new QTreeWidgetItem(this, QStringList(tr("Step %1").arg(TopLevelItemIdx + 1)));
		StepItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
		addTopLevelItem(StepItem);
		StepItem->setExpanded(true);
	}

	CurrentItem = topLevelItem(Step - 1);

	for (QTreeWidgetItem* PieceItem : SelectedItems)
	{
		QTreeWidgetItem* Parent = PieceItem->parent();

		if (!Parent)
			continue;

		int ChildIndex = Parent->indexOfChild(PieceItem);
		CurrentItem->addChild(Parent->takeChild(ChildIndex));
	}

	UpdateModel();

	lcModel* Model = gMainWindow->GetActiveModel();

	if (Step > static_cast<int>(Model->GetCurrentStep()))
		Model->SetCurrentStep(Step);
}

void lcTimelineWidget::SetCurrentStep()
{
	QTreeWidgetItem* CurrentItem = currentItem();

	if (!CurrentItem)
		return;

	if (CurrentItem->parent())
		CurrentItem = CurrentItem->parent();

	int Step = indexOfTopLevelItem(CurrentItem);

	if (Step == -1)
		return;

	gMainWindow->GetActiveModel()->SetCurrentStep(Step + 1);
}

void lcTimelineWidget::CurrentItemChanged(QTreeWidgetItem* Current, QTreeWidgetItem* Previous)
{
	Q_UNUSED(Previous);

	if (Current && !Current->parent())
		SetCurrentStep();
}

void lcTimelineWidget::ItemSelectionChanged()
{
	std::vector<lcObject*> Selection;
	lcStep LastStep = 1;
	QList<QTreeWidgetItem*> SelectedItems = selectedItems();

	for (QTreeWidgetItem* PieceItem : SelectedItems)
	{
		lcPiece* Piece = (lcPiece*)PieceItem->data(0, Qt::UserRole).value<uintptr_t>();
		if (Piece)
		{
			LastStep = lcMax(LastStep, Piece->GetStepShow());
			Selection.emplace_back(Piece);
		}
	}

	lcPiece* CurrentPiece = nullptr;
	QTreeWidgetItem* CurrentItem = currentItem();

	if (CurrentItem && CurrentItem->isSelected())
		CurrentPiece = (lcPiece*)CurrentItem->data(0, Qt::UserRole).value<uintptr_t>();

	bool Blocked = blockSignals(true);
	mIgnoreUpdates = true;
	lcModel* Model = gMainWindow->GetActiveModel();

	if (LastStep > Model->GetCurrentStep())
	{
		Model->SetCurrentStep(LastStep);
		UpdateCurrentStepItem();
	}

	Model->SetSelectionAndFocus(Selection, CurrentPiece, LC_PIECE_SECTION_POSITION, false);
	mIgnoreUpdates = false;
	blockSignals(Blocked);
}

void lcTimelineWidget::dropEvent(QDropEvent* Event)
{
	QTreeWidgetItem* DropItem = itemAt(Event->pos());
	lcModel* Model = gMainWindow->GetActiveModel();

	QList<QTreeWidgetItem*> SelectedItems = selectedItems();
	clearSelection();

	if (DropItem)
	{
		QTreeWidgetItem* ParentItem = DropItem->parent();
		lcStep Step = indexOfTopLevelItem(ParentItem ? ParentItem : DropItem) + 1;

		if (Step > Model->GetCurrentStep())
			Model->SetCurrentStep(Step);
	}

	auto SortItems = [this](QTreeWidgetItem* Item1, QTreeWidgetItem* Item2)
	{
		QTreeWidgetItem* StepItem1 = Item1->parent();
		QTreeWidgetItem* StepItem2 = Item2->parent();

		if (StepItem1 == StepItem2)
			return StepItem1->indexOfChild(Item1) < StepItem1->indexOfChild(Item2);

		return indexOfTopLevelItem(StepItem1) < indexOfTopLevelItem(StepItem2);
	};

	std::sort(SelectedItems.begin(), SelectedItems.end(), SortItems);

	for (QTreeWidgetItem* SelectedItem : SelectedItems)
		SelectedItem->setSelected(true);

	QTreeWidget::dropEvent(Event);

	UpdateModel();
	Update(false, false);
}

void lcTimelineWidget::mousePressEvent(QMouseEvent* Event)
{
	if (Event->button() == Qt::RightButton)
	{
		QItemSelection Selection = selectionModel()->selection();

		bool Blocked = blockSignals(true);
		QTreeWidget::mousePressEvent(Event);
		blockSignals(Blocked);

		selectionModel()->select(Selection, QItemSelectionModel::ClearAndSelect);
	}
	else
		QTreeWidget::mousePressEvent(Event);
}

void lcTimelineWidget::mouseDoubleClickEvent(QMouseEvent* MouseEvent)
{
	if (MouseEvent->button() == Qt::LeftButton)
	{
		QTreeWidgetItem* CurrentItem = currentItem();
		PreviewSelection(CurrentItem);
	}

	QTreeWidget::mouseDoubleClickEvent(MouseEvent);
}

void lcTimelineWidget::PreviewSelection(QTreeWidgetItem* CurrentItem)
{
	if (!CurrentItem)
		return;

	lcPiece* Piece = (lcPiece*)CurrentItem->data(0, Qt::UserRole).value<uintptr_t>();

	if (!Piece)
		return;

	PieceInfo* Info = Piece->mPieceInfo;

	if (!Info)
		return;

	gMainWindow->PreviewPiece(Info->mFileName, Piece->GetColorCode(), false);
}

void lcTimelineWidget::UpdateModel()
{
	std::vector<std::pair<lcPiece*, lcStep>> PieceSteps;

	for (int TopLevelItemIdx = 0; TopLevelItemIdx < topLevelItemCount(); TopLevelItemIdx++)
	{
		QTreeWidgetItem* StepItem = topLevelItem(TopLevelItemIdx);

		for (int PieceItemIdx = 0; PieceItemIdx < StepItem->childCount(); PieceItemIdx++)
		{
			QTreeWidgetItem* PieceItem = StepItem->child(PieceItemIdx);
			lcPiece* Piece = (lcPiece*)PieceItem->data(0, Qt::UserRole).value<uintptr_t>();

			PieceSteps.emplace_back(std::pair<lcPiece*, lcStep>(Piece, TopLevelItemIdx + 1));
		}
	}

	mIgnoreUpdates = true;
	gMainWindow->GetActiveModel()->SetPieceSteps(PieceSteps);
	mIgnoreUpdates = false;
}
