#include "lc_global.h"
#include "lc_colorlist.h"
#include "lc_application.h"
#include "lc_library.h"
#include "lc_colors.h"

lcColorList::lcColorList(QWidget* Parent, bool AllowNoColor)
	: QWidget(Parent), mAllowNoColor(AllowNoColor)
{
	setFocusPolicy(Qt::StrongFocus);

	UpdateCells();

	connect(lcGetPiecesLibrary(), &lcPiecesLibrary::ColorsLoaded, this, &lcColorList::ColorsLoaded);
}

void lcColorList::UpdateCells()
{
	mCells.clear();
	mGroups.clear();
/*** LPub3D Mod - Exclude colour group if empty ***/
	mColorGroups = 0;

	for (int GroupIdx = 0; GroupIdx < LC_NUM_COLORGROUPS; GroupIdx++)
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];
		if ((int)Group->Colors.size())
			mColorGroups++;
	}
/*** LPub3D Mod end ***/

	for (int GroupIdx = 0; GroupIdx < mColorGroups; GroupIdx++)
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];
		lcColorListGroup ListGroup;

		for (int ColorIndex : Group->Colors)
		{
			mCells.emplace_back(lcColorListCell{ QRect(), ColorIndex });
			ListGroup.Name = Group->Name;
			ListGroup.Cells.emplace_back(mCells.size());
		}

		mGroups.emplace_back(std::move(ListGroup));
	}

	if (mAllowNoColor)
	{
		mCells.emplace_back(lcColorListCell{ QRect(), lcGetColorIndex(LC_COLOR_NOCOLOR) });
		mGroups[LC_COLORGROUP_SPECIAL].Cells.emplace_back(mCells.size());
	}

	mColumns = 14;
	mRows = 0;

	for (const lcColorListGroup& Group : mGroups)
		mRows += ((int)Group.Cells.size() + mColumns - 1) / mColumns;

	QFontMetrics Metrics(font());
	int TextHeight = 0;

	for (lcColorListGroup& Group : mGroups)
	{
		Group.Rect = Metrics.boundingRect(rect(), Qt::TextSingleLine | Qt::AlignCenter, Group.Name);

		TextHeight += Group.Rect.height();
	}

	mPreferredHeight = TextHeight + 10 * mRows;

	setMinimumHeight(TextHeight + 5 * mRows);
}

void lcColorList::UpdateRects()
{
	QFontMetrics Metrics(font());
	int TextHeight = 0;

	for (lcColorListGroup& Group : mGroups)
	{
		Group.Rect = Metrics.boundingRect(rect(), Qt::TextSingleLine | Qt::AlignCenter, Group.Name);

		TextHeight += Group.Rect.height();
	}

	mPreferredHeight = TextHeight + 10 * mRows;

	float CellWidth = (float)(width() + 1) / (float)mColumns;
	float CellHeight = (float)(height() - TextHeight) / (float)mRows;

	while (CellWidth / CellHeight > 1.5f)
	{
		mColumns++;
		mRows = 0;

		for (const lcColorListGroup& Group : mGroups)
			mRows += ((int)Group.Cells.size() + mColumns - 1) / mColumns;

		CellWidth = (float)(width() + 1) / (float)mColumns;
		CellHeight = (float)(height() - TextHeight) / (float)mRows;

/*** LPub3D Mod - Exclude colour group if empty ***/
		if (mRows <= mColorGroups)
/*** LPub3D Mod end ***/
			break;
	}

	while (CellHeight / CellWidth > 1.5f)
	{
		mColumns--;
		mRows = 0;

		for (const lcColorListGroup& Group : mGroups)
			mRows += ((int)Group.Cells.size() + mColumns - 1) / mColumns;

		CellWidth = (float)(width() + 1) / (float)mColumns;
		CellHeight = (float)(height() - TextHeight) / (float)mRows;

		if (mColumns <= 5)
			break;
	}

	int CurCell = 0;
	float GroupY = 0.0f;
	int TotalRows = 1;

	for (lcColorListGroup& Group : mGroups)
	{
		int CurColumn = 0;
		int NumRows = 0;

		Group.Rect = QRect(0, (int)GroupY, width(), Group.Rect.height());
		GroupY += Group.Rect.height();

		for (size_t ColorIdx = 0; ColorIdx < Group.Cells.size(); ColorIdx++)
		{
			const int Left = CellWidth * CurColumn - 1;
			const int Right = (CurColumn + 1) * CellWidth - 1;
			const int Top = GroupY + CellHeight * NumRows;
			const int Bottom = (TotalRows != mRows) ? GroupY + CellHeight * (NumRows + 1) : height() - 1;

			mCells[CurCell].Rect = QRect(Left, Top, Right - Left, Bottom - Top);

			CurColumn++;
			if (CurColumn == mColumns)
			{
				CurColumn = 0;
				NumRows++;
				TotalRows++;
			}

			CurCell++;
		}

		if (CurColumn != 0)
		{
			NumRows++;
			TotalRows++;
		}

		GroupY += NumRows * CellHeight;
	}
}

void lcColorList::ColorsLoaded()
{
	UpdateCells();
	UpdateRects();

	SetCurrentColor(lcGetColorIndex(mColorCode));

	update();
}

QSize lcColorList::sizeHint() const
{
	return QSize(200, mPreferredHeight);
}

void lcColorList::SetCurrentColor(int ColorIndex)
{
	for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
	{
		if (mCells[CellIndex].ColorIndex == ColorIndex)
		{
			SelectCell(CellIndex);
			break;
		}
	}
}

bool lcColorList::event(QEvent* Event)
{
	if (Event->type() == QEvent::ToolTip)
	{
		QHelpEvent* HelpEvent = static_cast<QHelpEvent*>(Event);

		for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
		{
			if (!mCells[CellIndex].Rect.contains(HelpEvent->pos()))
				continue;

			QString Text = lcGetColorToolTip(mCells[CellIndex].ColorIndex);
			QToolTip::showText(HelpEvent->globalPos(), Text);

			return true;
		}

		QToolTip::hideText();
		Event->ignore();

		return true;
	}
	else if (Event->type() == QEvent::ShortcutOverride)
	{
		QKeyEvent* KeyEvent = static_cast<QKeyEvent*>(Event);

		if (KeyEvent->modifiers() == Qt::NoModifier || KeyEvent->modifiers() == Qt::KeypadModifier)
		{
			switch (KeyEvent->key())
			{
			case Qt::Key_Left:
			case Qt::Key_Right:
			case Qt::Key_Up:
			case Qt::Key_Down:
				KeyEvent->accept();
				break;

			default:
				break;
			}
		}
	}

	return QWidget::event(Event);
}

void lcColorList::mousePressEvent(QMouseEvent* MouseEvent)
{
	for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
	{
		if (!mCells[CellIndex].Rect.contains(MouseEvent->pos()))
			continue;

		SelectCell(CellIndex);
		emit ColorSelected(mCells[CellIndex].ColorIndex);

		break;
	}

	mDragStartPosition = MouseEvent->pos();
}

void lcColorList::mouseMoveEvent(QMouseEvent* MouseEvent)
{
	if (!(MouseEvent->buttons() & Qt::LeftButton))
		return;

	if ((MouseEvent->pos() - mDragStartPosition).manhattanLength() < QApplication::startDragDistance())
		return;

	QMimeData* MimeData = new QMimeData;
	MimeData->setData("application/vnd.leocad-color", QString::number(mCells[mCurrentCell].ColorIndex).toLatin1());

	QDrag* Drag = new QDrag(this);
	Drag->setMimeData(MimeData);

	Drag->exec(Qt::CopyAction);
}

void lcColorList::keyPressEvent(QKeyEvent *event)
{
	size_t NewCell = mCurrentCell;

	if (event->key() == Qt::Key_Left)
	{
		if (mCurrentCell > 0)
			NewCell = mCurrentCell - 1;
	}
	else if (event->key() == Qt::Key_Right)
	{
		if (mCurrentCell < mCells.size() - 1)
			NewCell = mCurrentCell + 1;
	}
	else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
	{
		if (mCurrentCell >= mCells.size())
			mCurrentCell = 0;

		size_t CurGroup = 0;
		size_t NumCells = 0;

		for (CurGroup = 0; CurGroup < mGroups.size(); CurGroup++)
		{
			int NumColors = (int)mGroups[CurGroup].Cells.size();

			if (mCurrentCell < NumCells + NumColors)
				break;

			NumCells += NumColors;
		}

		size_t Row = (mCurrentCell - NumCells) / mColumns;
		size_t Column = (mCurrentCell - NumCells) % mColumns;

		if (event->key() == Qt::Key_Up)
		{
			if (Row > 0)
				NewCell = mCurrentCell - mColumns;
			else if (CurGroup > 0)
			{
				size_t NumColors = mGroups[CurGroup - 1].Cells.size();
				size_t NumColumns = NumColors % mColumns;

				if (NumColumns < Column + 1)
					NewCell = mCurrentCell - NumColumns - mColumns;
				else
					NewCell = mCurrentCell - NumColumns;
			}
		}
		else if (event->key() == Qt::Key_Down)
		{
			int NumColors = (int)mGroups[CurGroup].Cells.size();

			if (mCurrentCell + mColumns < NumCells + NumColors)
				NewCell = mCurrentCell + mColumns;
			else
			{
				size_t NumColumns = NumColors % mColumns;

				if (NumColumns > Column)
				{
					if (mCurrentCell + NumColumns < mCells.size())
						NewCell = mCurrentCell + NumColumns;
				}
				else
					NewCell = mCurrentCell + mColumns + NumColumns;
			}
		}
	}
	else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		emit ColorSelected(mCells[mCurrentCell].ColorIndex);
	}

	if (NewCell != mCurrentCell)
		SelectCell(NewCell);
	else
		QWidget::keyPressEvent(event);
}

void lcColorList::resizeEvent(QResizeEvent* Event)
{
	if (mWidth == width() && mHeight == height())
		return;

	UpdateRects();

	mWidth = width();
	mHeight = height();

	QWidget::resizeEvent(Event);
}

void lcColorList::paintEvent(QPaintEvent* Event)
{
	Q_UNUSED(Event);

	QPainter Painter(this);

	Painter.fillRect(rect(), palette().brush(QPalette::Window));

	Painter.setFont(font());
	Painter.setPen(palette().color(QPalette::Text));

	for (const lcColorListGroup& Group : mGroups)
		Painter.drawText(Group.Rect, Qt::TextSingleLine | Qt::AlignLeft, Group.Name);

	Painter.setPen(palette().color(QPalette::Shadow));

	for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
	{
		const lcColor* Color = &gColorList[mCells[CellIndex].ColorIndex];

		const QRect& Rect = mCells[CellIndex].Rect;

		if (Color->Code != LC_COLOR_NOCOLOR)
		{
			QColor CellColor(Color->Value[0] * 255, Color->Value[1] * 255, Color->Value[2] * 255);

			Painter.setBrush(CellColor);
			Painter.drawRect(Rect);
		}
		else
			lcDrawNoColorRect(Painter, Rect);
	}

	if (mCurrentCell < mCells.size())
	{
		const lcColor* Color = &gColorList[mCells[mCurrentCell].ColorIndex];
		QColor EdgeColor(255 - Color->Value[0] * 255, 255 - Color->Value[1] * 255, 255 - Color->Value[2] * 255);

		Painter.setPen(EdgeColor);
		Painter.setBrush(Qt::NoBrush);

		QRect CellRect = mCells[mCurrentCell].Rect;
		CellRect.adjust(1, 1, -1, -1);
		Painter.drawRect(CellRect);
	}
}

void lcColorList::SelectCell(size_t CellIndex)
{
	if (CellIndex >= mCells.size())
		return;

	if (CellIndex == mCurrentCell)
		return;

	mCurrentCell = CellIndex;
	mColorCode = lcGetColorCode(mCells[CellIndex].ColorIndex);

	emit ColorChanged(mCells[mCurrentCell].ColorIndex);
	update();
}
