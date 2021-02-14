#include "lc_global.h"
#include "lc_qcolorlist.h"
#include "lc_application.h"
#include "lc_library.h"
#include "lc_colors.h"

lcQColorList::lcQColorList(QWidget *parent)
	: QWidget(parent)
{
	setFocusPolicy(Qt::StrongFocus);

/*** LPub3D Mod - Exclude colour group if empty ***/
	mNumColorGroups = 0;
/*** LPub3D Mod end ***/

	UpdateCells();

	connect(lcGetPiecesLibrary(), &lcPiecesLibrary::ColorsLoaded, this, &lcQColorList::ColorsLoaded);
}

void lcQColorList::UpdateCells()
{
	mCells.clear();
	mGroupRects.clear();

	mNumColorGroups = 0;
	mColumns = 14;
	mRows = 0;

	for (int GroupIdx = 0; GroupIdx < LC_NUM_COLORGROUPS; GroupIdx++)
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];

/*** LPub3D Mod - Exclude colour group if empty ***/
		if ((int)Group->Colors.size())
			mNumColorGroups++;
/*** LPub3D Mod end ***/  

		for (int ColorIndex : Group->Colors)
			mCells.emplace_back(lcColorListCell{ QRect(), ColorIndex });

		mRows += ((int)Group->Colors.size() + mColumns - 1) / mColumns;
	}

	QFontMetrics Metrics(font());
	int TextHeight = 0;

/*** LPub3D Mod - Exclude colour group if empty ***/
	for (int GroupIdx = 0; GroupIdx < mNumColorGroups; GroupIdx++)
/*** LPub3D Mod end ***/ 
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];

		mGroupRects.emplace_back(Metrics.boundingRect(rect(), Qt::TextSingleLine | Qt::AlignCenter, Group->Name));

		TextHeight += mGroupRects[GroupIdx].height();
	}

	mPreferredHeight = TextHeight + 10 * mRows;

	setMinimumHeight(TextHeight + 5 * mRows);
}

void lcQColorList::UpdateRects()
{
	QFontMetrics Metrics(font());
	int TextHeight = 0;

/*** LPub3D Mod - Exclude colour group if empty ***/
	for (int GroupIdx = 0; GroupIdx < mNumColorGroups; GroupIdx++)
/*** LPub3D Mod end ***/ 
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];

		mGroupRects[GroupIdx] = Metrics.boundingRect(rect(), Qt::TextSingleLine | Qt::AlignCenter, Group->Name);

		TextHeight += mGroupRects[GroupIdx].height();
	}

	mPreferredHeight = TextHeight + 10 * mRows;

	float CellWidth = (float)(width() + 1) / (float)mColumns;
	float CellHeight = (float)(height() - TextHeight) / (float)mRows;

	while (CellWidth / CellHeight > 1.5f)
	{
		mColumns++;
		mRows = 0;

/*** LPub3D Mod - Exclude colour group if empty ***/
		for (int GroupIdx = 0; GroupIdx < mNumColorGroups; GroupIdx++)
/*** LPub3D Mod end ***/ 
		{
			lcColorGroup* Group = &gColorGroups[GroupIdx];
			mRows += ((int)Group->Colors.size() + mColumns - 1) / mColumns;
		}

		CellWidth = (float)(width() + 1) / (float)mColumns;
		CellHeight = (float)(height() - TextHeight) / (float)mRows;

/*** LPub3D Mod - Exclude colour group if empty ***/
		if (mRows <= mNumColorGroups)
/*** LPub3D Mod end ***/ 
			break;
	}

	while (CellHeight / CellWidth > 1.5f)
	{
		mColumns--;
		mRows = 0;

/*** LPub3D Mod - Exclude colour group if empty ***/
		for (int GroupIdx = 0; GroupIdx < mNumColorGroups; GroupIdx++)
/*** LPub3D Mod end ***/ 
		{
			lcColorGroup* Group = &gColorGroups[GroupIdx];
			mRows += ((int)Group->Colors.size() + mColumns - 1) / mColumns;
		}

		CellWidth = (float)(width() + 1) / (float)mColumns;
		CellHeight = (float)(height() - TextHeight) / (float)mRows;

		if (mColumns <= 5)
			break;
	}

	int CurCell = 0;
	float GroupY = 0.0f;
	int TotalRows = 1;

/*** LPub3D Mod - Exclude colour group if empty ***/
	for (int GroupIdx = 0; GroupIdx < mNumColorGroups; GroupIdx++)
/*** LPub3D Mod end ***/ 	
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];
		int CurColumn = 0;
		int NumRows = 0;

		mGroupRects[GroupIdx] = QRect(0, (int)GroupY, width(), mGroupRects[GroupIdx].height());
		GroupY += mGroupRects[GroupIdx].height();

		for (size_t ColorIdx = 0; ColorIdx < Group->Colors.size(); ColorIdx++)
		{
			const int Left = CellWidth * CurColumn - 1;
			const int Right = (CurColumn + 1) * CellWidth - 1;
			const int Top = GroupY + CellHeight * NumRows;
			const int Bottom = (TotalRows != mRows) ? GroupY + CellHeight * (NumRows + 1) : height();

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

void lcQColorList::ColorsLoaded()
{
	UpdateCells();
	UpdateRects();

	setCurrentColor(lcGetColorIndex(mColorCode));

	update();
}

QSize lcQColorList::sizeHint() const
{
	return QSize(200, mPreferredHeight);
}

void lcQColorList::setCurrentColor(int ColorIndex)
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

bool lcQColorList::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip)
	{
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

		for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
		{
			if (!mCells[CellIndex].Rect.contains(helpEvent->pos()))
				continue;

			lcColor* color = &gColorList[mCells[CellIndex].ColorIndex];
			QColor rgb(color->Value[0] * 255, color->Value[1] * 255, color->Value[2] * 255);

			QImage image(16, 16, QImage::Format_RGB888);
			image.fill(rgb);
			QPainter painter(&image);
			painter.setPen(Qt::darkGray);
			painter.drawRect(0, 0, image.width() - 1, image.height() - 1);
			painter.end();

			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::WriteOnly);
			image.save(&buffer, "PNG");
			buffer.close();

			int colorIndex = mCells[CellIndex].ColorIndex;
			const char* format = "<table><tr><td style=\"vertical-align:middle\"><img src=\"data:image/png;base64,%1\"/></td><td>%2 (%3)</td></tr></table>";
			QString text = QString(format).arg(QString(buffer.data().toBase64()), gColorList[colorIndex].Name, QString::number(gColorList[colorIndex].Code));

			QToolTip::showText(helpEvent->globalPos(), text);
			return true;
		}

		QToolTip::hideText();
		event->ignore();

		return true;
	}
	else if (event->type() == QEvent::ShortcutOverride)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::KeypadModifier)
		{
			switch (keyEvent->key())
			{
			case Qt::Key_Left:
			case Qt::Key_Right:
			case Qt::Key_Up:
			case Qt::Key_Down:
				keyEvent->accept();
			default:
				break;
			}
		}
	}

	return QWidget::event(event);
}

void lcQColorList::mousePressEvent(QMouseEvent* MouseEvent)
{
	for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
	{
		if (!mCells[CellIndex].Rect.contains(MouseEvent->pos()))
			continue;

		SelectCell(CellIndex);
		emit colorSelected(mCells[CellIndex].ColorIndex);

		break;
	}

	mDragStartPosition = MouseEvent->pos();
}

void lcQColorList::mouseMoveEvent(QMouseEvent* MouseEvent)
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

void lcQColorList::keyPressEvent(QKeyEvent *event)
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

/*** LPub3D Mod - Exclude colour group if empty ***/
		for (CurGroup = 0; CurGroup < mNumColorGroups; CurGroup++)
/*** LPub3D Mod end ***/ 		
		{
			int NumColors = (int)gColorGroups[CurGroup].Colors.size();

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
				size_t NumColors = gColorGroups[CurGroup - 1].Colors.size();
				size_t NumColumns = NumColors % mColumns;

				if (NumColumns < Column + 1)
					NewCell = mCurrentCell - NumColumns - mColumns;
				else
					NewCell = mCurrentCell - NumColumns;
			}
		}
		else if (event->key() == Qt::Key_Down)
		{
			int NumColors = (int)gColorGroups[CurGroup].Colors.size();

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
		emit colorSelected(mCells[mCurrentCell].ColorIndex);
	}

	if (NewCell != mCurrentCell)
		SelectCell(NewCell);
	else
		QWidget::keyPressEvent(event);
}

void lcQColorList::resizeEvent(QResizeEvent* Event)
{
	if (mWidth == width() && mHeight == height())
		return;

	UpdateRects();

	mWidth = width();
	mHeight = height();

	QWidget::resizeEvent(Event);
}

void lcQColorList::paintEvent(QPaintEvent* Event)
{
	Q_UNUSED(Event);

	QPainter Painter(this);

	Painter.fillRect(rect(), palette().brush(QPalette::Window));

	Painter.setFont(font());
	Painter.setPen(palette().color(QPalette::Text));

/*** LPub3D Mod - Exclude colour group if empty ***/
	for (int GroupIdx = 0; GroupIdx < mNumColorGroups; GroupIdx++)
/*** LPub3D Mod end ***/ 	
	{
		lcColorGroup* Group = &gColorGroups[GroupIdx];

		Painter.drawText(mGroupRects[GroupIdx], Qt::TextSingleLine | Qt::AlignLeft, Group->Name);
	}

	Painter.setPen(palette().color(QPalette::Shadow));

	for (size_t CellIndex = 0; CellIndex < mCells.size(); CellIndex++)
	{
		lcColor* Color = &gColorList[mCells[CellIndex].ColorIndex];
		QColor CellColor(Color->Value[0] * 255, Color->Value[1] * 255, Color->Value[2] * 255);

		Painter.setBrush(CellColor);
		Painter.drawRect(mCells[CellIndex].Rect);
	}

	if (mCurrentCell < mCells.size())
	{
		lcColor* Color = &gColorList[mCells[mCurrentCell].ColorIndex];
		QColor EdgeColor(255 - Color->Value[0] * 255, 255 - Color->Value[1] * 255, 255 - Color->Value[2] * 255);
		QColor CellColor(Color->Value[0] * 255, Color->Value[1] * 255, Color->Value[2] * 255);

		Painter.setPen(EdgeColor);
		Painter.setBrush(CellColor);

		QRect CellRect = mCells[mCurrentCell].Rect;
		CellRect.adjust(1, 1, -1, -1);
		Painter.drawRect(CellRect);

		/*
		if (GetFocus() == this)
		{
			rc.DeflateRect(2, 2);
			dc.DrawFocusRect(rc);
		}
		*/
	}
}

void lcQColorList::SelectCell(size_t CellIndex)
{
	if (CellIndex >= mCells.size())
		return;

	if (CellIndex == mCurrentCell)
		return;

	mCurrentCell = CellIndex;
	mColorCode = lcGetColorCode(mCells[CellIndex].ColorIndex);

	emit colorChanged(mCells[mCurrentCell].ColorIndex);
	update();
}
