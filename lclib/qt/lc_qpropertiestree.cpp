#include "lc_global.h"
#include "lc_qpropertiestree.h"
#include "lc_colorpicker.h"
#include "lc_application.h"
#include "lc_model.h"
#include "lc_mainwindow.h"
#include "object.h"
#include "piece.h"
#include "camera.h"
#include "light.h"
#include "pieceinf.h"
#include "lc_library.h"
#include "lc_qutils.h"
#include "lc_viewwidget.h"
#include "lc_previewwidget.h"
/*** LPub3D Mod - Camera Globe ***/
#include "lpub_qtcompat.h"
#include "project.h"
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Axis overlay ***/
enum axis
{
	X = 0,
	Y = 1,
	Z = 2
};
/*** LPub3D Mod end ***/

// Draw an icon indicating opened/closing branches
static QIcon drawIndicatorIcon(const QPalette &palette, QStyle *style)
{
	QPixmap pix(14, 14);
	pix.fill(Qt::transparent);
	QStyleOption branchOption;
	branchOption.rect = QRect(2, 2, 9, 9); // ### hardcoded in qcommonstyle.cpp
	branchOption.palette = palette;
	branchOption.state = QStyle::State_Children;

	QPainter p;
	// Draw closed state
	p.begin(&pix);
	style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
	p.end();
	QIcon rc = pix;
	rc.addPixmap(pix, QIcon::Selected, QIcon::Off);
	// Draw opened state
	branchOption.state |= QStyle::State_Open;
	pix.fill(Qt::transparent);
	p.begin(&pix);
	style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
	p.end();

	rc.addPixmap(pix, QIcon::Normal, QIcon::On);
	rc.addPixmap(pix, QIcon::Selected, QIcon::On);
	return rc;
}

static QIcon drawCheckBox(bool value)
{
	QStyleOptionButton opt;
	opt.state |= value ? QStyle::State_On : QStyle::State_Off;
	opt.state |= QStyle::State_Enabled;
	const QStyle *style = QApplication::style();
	// Figure out size of an indicator and make sure it is not scaled down in a list view item
	// by making the pixmap as big as a list view icon and centering the indicator in it.
	// (if it is smaller, it can't be helped)
	const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
	const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
	const int listViewIconSize = indicatorWidth;
	const int pixmapWidth = indicatorWidth;
	const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

	opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
	QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
	pixmap.fill(Qt::transparent);
	{
		// Center?
		const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
		const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
		QPainter painter(&pixmap);
		painter.translate(xoff, yoff);
		style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
	}
	return QIcon(pixmap);
}

int lcQPropertiesTreeDelegate::indentation(const QModelIndex &index) const
{
	if (!m_treeWidget)
		return 0;

	QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
	int indent = 0;
	while (item->parent())
	{
		item = item->parent();
		++indent;
	}

	if (m_treeWidget->rootIsDecorated())
		++indent;

	return indent * m_treeWidget->indentation();
}

void lcQPropertiesTreeDelegate::slotEditorDestroyed(QObject *object)
{
	if (m_editedWidget == object)
	{
		m_editedWidget = nullptr;
		m_editedItem = nullptr;
	}
}

QWidget *lcQPropertiesTreeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &style, const QModelIndex &index) const
{
	Q_UNUSED(style);

	if (index.column() == 1 && m_treeWidget)
	{
		QTreeWidgetItem *item = m_treeWidget->indexToItem(index);

		if (item && (item->flags() & Qt::ItemIsEnabled))
		{
			QWidget *editor = m_treeWidget->createEditor(parent, item);
			if (editor)
			{
				editor->setAutoFillBackground(true);
				editor->installEventFilter(const_cast<lcQPropertiesTreeDelegate *>(this));

				m_editedItem = item;
				m_editedWidget = editor;

				connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));

				return editor;
			}
		}
	}

	return nullptr;
}

void lcQPropertiesTreeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index)
	editor->setGeometry(option.rect.adjusted(0, 0, 0, -1));
}

void lcQPropertiesTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	bool hasValue = true;
	if (m_treeWidget)
		hasValue = m_treeWidget->indexToItem(index)->data(0, lcQPropertiesTree::PropertyTypeRole).toInt() != lcQPropertiesTree::PropertyGroup;

	QStyleOptionViewItem opt = option;

	opt.state &= ~QStyle::State_HasFocus;

	if (index.column() == 1 && m_treeWidget)
	{
		QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
		if (m_editedItem && m_editedItem == item)
			m_disablePainting = true;
	}

	QItemDelegate::paint(painter, opt, index);
	if (option.type)
		m_disablePainting = false;

	opt.palette.setCurrentColorGroup(QPalette::Active);
	QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
	painter->save();
	painter->setPen(QPen(color));

	if (!m_treeWidget || (!m_treeWidget->lastColumn(index.column()) && hasValue))
	{
		int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
		painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
	}

	painter->restore();
}

void lcQPropertiesTreeDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
	if (m_disablePainting)
		return;

	QItemDelegate::drawDecoration(painter, option, rect, pixmap);
}

void lcQPropertiesTreeDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
	if (m_disablePainting)
		return;

	QItemDelegate::drawDisplay(painter, option, rect, text);
}

QSize lcQPropertiesTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QItemDelegate::sizeHint(option, index) + QSize(3, 4);
}

bool lcQPropertiesTreeDelegate::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::FocusOut)
	{
		QFocusEvent *fe = static_cast<QFocusEvent *>(event);
		if (fe->reason() == Qt::ActiveWindowFocusReason)
			return false;
	}
	return QItemDelegate::eventFilter(object, event);
}

lcQPropertiesTree::lcQPropertiesTree(QWidget *parent) :
	QTreeWidget(parent)
{
	setIconSize(QSize(18, 18));
	setColumnCount(2);
	QStringList labels;
	labels.append(tr("Property"));
	labels.append(tr("Value"));
	setHeaderLabels(labels);
	header()->setSectionsMovable(false);
	header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	header()->setVisible(false);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
	setEditTriggers(QAbstractItemView::EditKeyPressed);

	m_expandIcon = drawIndicatorIcon(palette(), style());
	m_checkedIcon = drawCheckBox(true);
	m_uncheckedIcon = drawCheckBox(false);

	mDelegate = new lcQPropertiesTreeDelegate(parent);
	mDelegate->setTreeWidget(this);
	setItemDelegate(mDelegate);

	SetEmpty();

	connect(header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));
}

QSize lcQPropertiesTree::sizeHint() const
{
	return QSize(200, -1);
}

void lcQPropertiesTree::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;

	QTreeWidgetItem *item = itemFromIndex(index);

	if (item->data(0, lcQPropertiesTree::PropertyTypeRole).toInt() == lcQPropertiesTree::PropertyGroup)
	{
		const QColor c = option.palette.color(QPalette::Dark);
		painter->fillRect(option.rect, c);
		opt.palette.setColor(QPalette::AlternateBase, c);
	}

	QTreeWidget::drawRow(painter, opt, index);
	QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
	painter->save();
	painter->setPen(QPen(color));
	painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
	painter->restore();
}

void lcQPropertiesTree::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Space: // Trigger Edit
		if (!mDelegate->editedItem())
		{
			if (const QTreeWidgetItem *item = currentItem())
			{
				if (item->columnCount() >= 2 && ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled)))
				{
					event->accept();
					// If the current position is at column 0, move to 1.
					QModelIndex index = currentIndex();
					if (index.column() == 0)
					{
						index = index.sibling(index.row(), 1);
						setCurrentIndex(index);
					}
					edit(index);
					return;
				}
			}
		}
		break;

	default:
		break;
	}

	QTreeWidget::keyPressEvent(event);
}

void lcQPropertiesTree::mousePressEvent(QMouseEvent *event)
{
	QTreeWidget::mousePressEvent(event);
	QTreeWidgetItem *item = itemAt(event->pos());

	if (item)
	{
		if ((item != mDelegate->editedItem()) && (event->button() == Qt::LeftButton) && (header()->logicalIndexAt(event->pos().x()) == 1) &&
			((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled)))
			editItem(item, 1);
	}
}

void lcQPropertiesTree::Update(const lcArray<lcObject*>& Selection, lcObject* Focus)
{
	lcPropertyWidgetMode Mode = LC_PROPERTY_WIDGET_EMPTY;

	if (Focus)
	{
		switch (Focus->GetType())
		{
		case lcObjectType::Piece:
			Mode = LC_PROPERTY_WIDGET_PIECE;
			break;

		case lcObjectType::Camera:
			Mode = LC_PROPERTY_WIDGET_CAMERA;
			break;

		case lcObjectType::Light:
			Mode = LC_PROPERTY_WIDGET_LIGHT;
			break;
		}
	}
	else
	{
		for (int ObjectIdx = 0; ObjectIdx < Selection.GetSize(); ObjectIdx++)
		{
			switch (Selection[ObjectIdx]->GetType())
			{
			case lcObjectType::Piece:
				if (Mode == LC_PROPERTY_WIDGET_EMPTY)
/*** LPub3D Mod - set the focus on the first piece ***/
				{
					Mode = LC_PROPERTY_WIDGET_PIECE;
					Focus = (lcObject*)Selection[ObjectIdx];
				}
/*** LPub3D Mod end ***/
				else if (Mode != LC_PROPERTY_WIDGET_PIECE)
				{
					Mode = LC_PROPERTY_WIDGET_MULTIPLE;
					ObjectIdx = Selection.GetSize();
				}
				break;

			case lcObjectType::Camera:
				if (Mode != LC_PROPERTY_WIDGET_EMPTY)
				{
					Mode = LC_PROPERTY_WIDGET_MULTIPLE;
					ObjectIdx = Selection.GetSize();
				}
				else
				{
					Mode = LC_PROPERTY_WIDGET_CAMERA;
					Focus = Selection[ObjectIdx];
				}
				break;

			case lcObjectType::Light:
				if (Mode != LC_PROPERTY_WIDGET_EMPTY)
				{
					Mode = LC_PROPERTY_WIDGET_MULTIPLE;
					ObjectIdx = Selection.GetSize();
				}
				else
				{
					Mode = LC_PROPERTY_WIDGET_LIGHT;
					Focus = Selection[ObjectIdx];
				}
				break;
			}
		}
	}

	switch (Mode)
	{
	case LC_PROPERTY_WIDGET_EMPTY:
		SetEmpty();
		break;

	case LC_PROPERTY_WIDGET_PIECE:
		SetPiece(Selection, Focus);
		break;

	case LC_PROPERTY_WIDGET_CAMERA:
		SetCamera(Focus);
		break;

	case LC_PROPERTY_WIDGET_LIGHT:
		SetLight(Focus);
		break;

	case LC_PROPERTY_WIDGET_MULTIPLE:
		SetMultiple();
		break;
	}
}

class lcStepValidator : public QIntValidator
{
public:
	lcStepValidator(lcStep Min, lcStep Max, bool AllowEmpty)
		: QIntValidator(1, INT_MAX), mMin(Min), mMax(Max), mAllowEmpty(AllowEmpty)
	{
	}

	QValidator::State validate(QString& Input, int& Pos) const override
	{
		if (mAllowEmpty && Input.isEmpty())
			return Acceptable;

		bool Ok;
		lcStep Step = Input.toUInt(&Ok);

		if (Ok)
			return (Step >= mMin && Step <= mMax) ? Acceptable : Invalid;

		return QIntValidator::validate(Input, Pos);
	}

protected:
	lcStep mMin;
	lcStep mMax;
	bool mAllowEmpty;
};

QWidget* lcQPropertiesTree::createEditor(QWidget* Parent, QTreeWidgetItem* Item) const
{
	lcQPropertiesTree::PropertyType PropertyType = (lcQPropertiesTree::PropertyType)Item->data(0, lcQPropertiesTree::PropertyTypeRole).toInt();

	switch (PropertyType)
	{
	case PropertyGroup:
	case PropertyBool:
		return nullptr;

	case PropertyFloat:
		{
			QLineEdit* Editor = new QLineEdit(Parent);
			float Value = Item->data(0, PropertyValueRole).toFloat();
			QPointF Range = Item->data(0, PropertyRangeRole).toPointF();

			Editor->setValidator(Range.isNull() ? new QDoubleValidator(Editor) : new QDoubleValidator(Range.x(), Range.y(), 1, Editor));
			Editor->setText(lcFormatValueLocalized(Value));

			connect(Editor, &QLineEdit::returnPressed, this, &lcQPropertiesTree::slotReturnPressed);

			return Editor;
		}

/*** LPub3D Mod - Camera Globe, Custom properties ***/
	case PropertyBoolReadOnly:
		{
			QCheckBox *Editor = new QCheckBox(Parent);
			bool value = Item->data(0, PropertyValueRole).toBool();

			Editor->setChecked(value);
			Editor->setEnabled(false);
			Editor->setToolTip(tr("Property is read only"));

			return Editor;
		}

	case PropertyFloatCameraAngle:
		{
			QLineEdit *Editor = new QLineEdit(Parent);
			float value = Item->data(0, PropertyValueRole).toFloat();

			QRegExp vrx("^(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){1}\\s{0,1}(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){0,1}$");
			Editor->setValidator(new QRegExpValidator(vrx,Parent));
			Editor->setText(lcFormatValueLocalized(value));
			Editor->setToolTip(tr("Eneter respective angle or both angles. Use single space delimeter"));

			connect(Editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return Editor;
		}

	case PropertyFloatTarget:
		{
			QLineEdit *Editor = new QLineEdit(Parent);
			float value = Item->data(0, PropertyValueRole).toFloat();

			QRegExp vrx("^(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){1}\\s{0,1}(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){0,1}\\s{0,1}(?:[+\\-]{0,1}[\\d]+[,|.]{0,1}[\\d]*){0,1}$");
			Editor->setValidator(new QRegExpValidator(vrx,Parent));
			Editor->setText(lcFormatValueLocalized(value));
			Editor->setToolTip(tr("Enter respective axis or all axes. Use single space delimeter"));

			connect(Editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return Editor;
		}
/*** LPub3D Mod end ***/

	case PropertyFloatReadOnly:
		return nullptr;

	case PropertyStep:
		{
			QLineEdit* Editor = new QLineEdit(Parent);

			lcStep Value = Item->data(0, PropertyValueRole).toUInt();
			lcStep Show = partShow->data(0, PropertyValueRole).toUInt();
			lcStep Hide = partHide->data(0, PropertyValueRole).toUInt();

			if (Show && Hide)
			{
				if (Item == partShow)
					Editor->setValidator(new lcStepValidator(1, Hide - 1, false));
				else
					Editor->setValidator(new lcStepValidator(Show + 1, LC_STEP_MAX, true));
			}
			else
				Editor->setValidator(new lcStepValidator(1, LC_STEP_MAX, Item == partHide));

			if (Item != partHide || Value != LC_STEP_MAX)
				Editor->setText(QString::number(Value));

			connect(Editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return Editor;
		}

/*** LPub3D Mod - LPub3D properties ***/
	case PropertyIntReadOnly:
		{
			QLineEdit *Editor = new QLineEdit(Parent);
			quint32 value = Item->data(0, PropertyValueRole).toUInt();

			Editor->setText(QString::number(value));
			Editor->setReadOnly(true);
			Editor->setToolTip(tr("Property is read only"));

			return Editor;
		}
/*** LPub3D Mod end ***/

	case PropertyString:
		{
			QLineEdit *editor = new QLineEdit(Parent);
			QString value = Item->data(0, PropertyValueRole).toString();

			editor->setText(value);

			connect(editor, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));

			return editor;
		}

	case PropertyStringList:
	{
		QComboBox* editor = new QComboBox(Parent);

		if (Item == mCameraProjectionItem)
		{
			editor->addItems( { tr("Perspective"), tr("Orthographic") } );
		}
		else if (Item == mLightTypeItem)
		{
			for (int LightTypeIndex = 0; LightTypeIndex < static_cast<int>(lcLightType::Count); LightTypeIndex++)
				editor->addItem(lcLight::GetLightTypeString(static_cast<lcLightType>(LightTypeIndex)));
		}

		int value = Item->data(0, PropertyValueRole).toInt();
		editor->setCurrentIndex(value);

		connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));

		return editor;
	}

	case PropertyLightFormat:
		{
			QComboBox *editor = new QComboBox(Parent);

			editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
			editor->setMinimumContentsLength(1);

			QStringList formats = { QLatin1String("Blender"), QLatin1String("POVRay") };
			for (int i = 0; i < formats.size(); i++)
				editor->addItem(formats.at(i), QVariant::fromValue(i));

			int value = Item->data(0, PropertyValueRole).toInt();
			editor->setCurrentIndex(value);

			connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));

			return editor;
		}

	case PropertyLightShape:
		{
			QComboBox *editor = new QComboBox(Parent);

			editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
			editor->setMinimumContentsLength(1);

			QStringList shapes = { tr("Square"), tr("Disk"), tr("Rectangle"),  tr("Ellipse") };
			for (int i = 0; i < shapes.size(); i++)
				editor->addItem(shapes.at(i), QVariant::fromValue(i));

			int value = Item->data(0, PropertyValueRole).toInt();
			editor->setCurrentIndex(value);

			connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));

			return editor;
		}

	case PropertyColor:
		{
			QPushButton *Editor = new QPushButton(Parent);
			QColor Value = Item->data(0, PropertyValueRole).value<QColor>();

			UpdateLightColorEditor(Editor, Value);

			connect(Editor, &QPushButton::clicked, this, &lcQPropertiesTree::LightColorButtonClicked);

			return Editor;
		}

	case PropertyPieceColor:
		{
			QPushButton *editor = new QPushButton(Parent);
			int value = Item->data(0, PropertyValueRole).toInt();

			updateColorEditor(editor, value);

			connect(editor, SIGNAL(clicked()), this, SLOT(slotColorButtonClicked()));

			return editor;
		}

	case PropertyPart:
		{
			QComboBox *editor = new QComboBox(Parent);

			editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
			editor->setMinimumContentsLength(1);

			lcPiecesLibrary* Library = lcGetPiecesLibrary();
			std::vector<PieceInfo*> SortedPieces;
			SortedPieces.reserve(Library->mPieces.size());
			const lcModel* ActiveModel = gMainWindow->GetActiveModel();

			for (const auto& PartIt : Library->mPieces)
			{
				PieceInfo* Info = PartIt.second;

				if (!Info->IsModel() || !Info->GetModel()->IncludesModel(ActiveModel))
					SortedPieces.push_back(PartIt.second);
			}

			auto PieceCompare = [](PieceInfo* Info1, PieceInfo* Info2)
			{
				return strcmp(Info1->m_strDescription, Info2->m_strDescription) < 0;
			};

			std::sort(SortedPieces.begin(), SortedPieces.end(), PieceCompare);

			for (PieceInfo* Info : SortedPieces)
				editor->addItem(Info->m_strDescription, QVariant::fromValue((void*)Info));

			PieceInfo *info = (PieceInfo*)Item->data(0, PropertyValueRole).value<void*>();
			editor->setCurrentIndex(editor->findData(QVariant::fromValue((void*)info)));

			connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetValue(int)));

			return editor;
		}
	}

	return nullptr;
}

void lcQPropertiesTree::updateColorEditor(QPushButton *editor, int value) const
{
	QImage img(12, 12, QImage::Format_ARGB32);
	img.fill(0);

	lcColor* color = &gColorList[value];
	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(QColor::fromRgbF(color->Value[0], color->Value[1], color->Value[2]));
	painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
	painter.end();

	editor->setStyleSheet("Text-align:left");
	editor->setIcon(QPixmap::fromImage(img));
	editor->setText(color->Name);
}

void lcQPropertiesTree::UpdateLightColorEditor(QPushButton* Editor, QColor Color) const
{
	QImage Image(12, 12, QImage::Format_ARGB32);
	Image.fill(0);

	QPainter Painter(&Image);
	Painter.setCompositionMode(QPainter::CompositionMode_Source);
	Painter.setPen(Qt::darkGray);
	Painter.setBrush(Color);
	Painter.drawRect(0, 0, Image.width() - 1, Image.height() - 1);
	Painter.end();

	Editor->setStyleSheet("Text-align:left");
	Editor->setIcon(QPixmap::fromImage(Image));
	Editor->setText(Color.name().toUpper());
}

void lcQPropertiesTree::slotToggled(bool Value)
{
	QTreeWidgetItem* Item = mDelegate->editedItem();
	lcModel* Model = gMainWindow->GetActiveModel();
	lcObject* Focus = Model->GetFocusObject();

	if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

		if (Light)
		{
			if (Item == lightEnableCutoff)
			{
				lcLightProperties Props = Light->GetLightProperties();
				Props.mEnableCutoff = Value;
				Model->UpdateLight(Light, Props, LC_LIGHT_USE_CUTOFF);
			}
			else if (Item == mLightCastShadowItem)
			{
				Model->SetLightCastShadow(Light, Value);
			}
		}
	}
}

void lcQPropertiesTree::slotReturnPressed()
{
	QLineEdit* Editor = (QLineEdit*)sender();
	QTreeWidgetItem* Item = mDelegate->editedItem();
	lcModel* Model = gMainWindow->GetActiveModel();

	if (mWidgetMode == LC_PROPERTY_WIDGET_PIECE)
	{
		lcPiece* Piece = (mFocus && mFocus->IsPiece()) ? (lcPiece*)mFocus : nullptr;

		if (Item == partPositionX || Item == partPositionY || Item == partPositionZ)
		{
			lcVector3 Center;
			lcMatrix33 RelativeRotation;
			Model->GetMoveRotateTransform(Center, RelativeRotation);
			lcVector3 Position = Center;
			float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
			if (Item == partPositionX)
				Position[X] = Value;
			else if (Item == partPositionY)
				Position[Z] = -Value;
			else if (Item == partPositionZ)
				Position[Y] = Value;
/*** LPub3D Mod end ***/

			lcVector3 Distance = Position - Center;

			Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
		}
		else if (Item == partRotationX || Item == partRotationY || Item == partRotationZ)
		{
			lcVector3 InitialRotation;
			if (Piece)
				InitialRotation = lcMatrix44ToEulerAngles(Piece->mModelWorld) * LC_RTOD;
			else
				InitialRotation = lcVector3(0.0f, 0.0f, 0.0f);
			lcVector3 Rotation = InitialRotation;

			float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
			if (Item == partRotationX)
				Rotation[X] = Value;
			else if (Item == partRotationY)
				Rotation[Z] = -Value;
			else if (Item == partRotationZ)
				Rotation[Y] = Value;
/*** LPub3D Mod end ***/

			Model->RotateSelectedObjects(Rotation - InitialRotation, true, false, true, true);
		}
		else if (Item == partShow)
		{
			bool Ok = false;
			lcStep Step = Editor->text().toUInt(&Ok);

			if (Ok)
				Model->SetSelectedPiecesStepShow(Step);
		}
		else if (Item == partHide)
		{
			QString Text = Editor->text();

			if (Text.isEmpty())
				Model->SetSelectedPiecesStepHide(LC_STEP_MAX);
			else
			{
				bool Ok = false;
				lcStep Step = Text.toUInt(&Ok);

				if (Ok)
					Model->SetSelectedPiecesStepHide(Step);
			}
		}
	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_CAMERA)
	{
		lcCamera* Camera = (mFocus && mFocus->IsCamera()) ? (lcCamera*)mFocus : nullptr;

		if (Camera)
		{
/*** LPub3D Mod - Camera Globe, camera name ***/
			QString Name = Camera->GetName();
			bool isDefault = Name.isEmpty();
/*** LPub3D Mod end ***/

			if (Item == cameraPositionX || Item == cameraPositionY || Item == cameraPositionZ)
			{
				lcVector3 Center = Camera->mPosition;
				lcVector3 Position = Center;
				float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
				if (Item == cameraPositionX)
					Position[X] = Value;
				else if (Item == cameraPositionY)
					Position[Z] = -Value;
				else if (Item == cameraPositionZ)
					Position[Y] = Value;
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
				if (isDefault) {
					Camera->mPosition = Position;
					Model->MoveDefaultCamera(Camera, Distance);
				} else
/*** LPub3D Mod end ***/
					Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == cameraTargetX || Item == cameraTargetY || Item == cameraTargetZ)
			{
				lcVector3 Center = Camera->mTargetPosition;
				lcVector3 Position = Center;

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
				QStringList ValueList = QString(Editor->text()).trimmed().split(" ");

				float FirstValue  = 0.0f;
				float SecondValue = 0.0f;
				float ThirdValue  = 0.0f;

				switch (ValueList.size()){
				case 1:
					FirstValue  = lcParseValueLocalized(ValueList.first());
					break;
				case 2:
					FirstValue  = lcParseValueLocalized(ValueList.first());
					SecondValue = lcParseValueLocalized(ValueList.last());
					break;
				case 3:
					FirstValue  = lcParseValueLocalized(ValueList.at(0));
					SecondValue = lcParseValueLocalized(ValueList.at(1));
					ThirdValue  = lcParseValueLocalized(ValueList.at(2));
					break;
				}

				bool hasSingleVal = ValueList.size() == 1;
				bool hasOneOther  = ValueList.size() == 2;
				bool hasTwoOther  = ValueList.size() == 3;

				if (Item == cameraTargetX) {
					if (hasSingleVal) {
						Position[X] =  FirstValue;
					}
					else if (hasOneOther) {
						Position[Z] = -SecondValue;
					}
					else if (hasTwoOther) {
						Position[Z] = -SecondValue;
						Position[Y] =  ThirdValue;
					}
				} else if (Item == cameraTargetY) {
					if (hasSingleVal) {
						Position[Z] = -FirstValue;
					}
					else if (hasOneOther) {
						Position[X] =  SecondValue;
						Position[Z] = -FirstValue;
					}
					else if (hasTwoOther) {
						Position[X] =  FirstValue ;
						Position[Z] = -SecondValue;
						Position[Y] =  ThirdValue;
					}
				} else if (Item == cameraTargetZ) {
					if (hasSingleVal) {
						Position[Y] =  FirstValue;
					}
					else if (hasOneOther) {
						Position[Z] = -FirstValue;
						Position[Y] =  SecondValue;
					}
					else if (hasTwoOther) {
						Position[X] =  FirstValue ;
						Position[Z] = -SecondValue;
						Position[Y] =  ThirdValue;
					}
				}
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
				if (isDefault) {
					Camera->mTargetPosition = Position;
					Model->MoveDefaultCamera(Camera, Distance);
				} else
/*** LPub3D Mod end ***/
					Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == cameraUpX || Item == cameraUpY || Item == cameraUpZ)
			{
				lcVector3 Center = Camera->mUpVector;
				lcVector3 Position = Center;
				float Value = lcParseValueLocalized(Editor->text());

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
				if (Item == cameraUpX)
					Position[X] = Value;
				else if (Item == cameraUpY)
					Position[Z] = -Value;
				else if (Item == cameraUpZ)
					Position[Y] = Value;
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

/*** LPub3D Mod - Camera Globe  ***/
				if (isDefault) {
					Camera->mUpVector = Position;
					Model->MoveDefaultCamera(Camera, Distance);
				} else
/*** LPub3D Mod end ***/
					Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
/*** LPub3D Mod - Camera Globe ***/
			else if (Item == cameraGlobeLatitude || Item == cameraGlobeLongitude /*|| Item == cameraGlobeDistance*/)
			{
				QStringList ValueList = QString(Editor->text()).trimmed().split(" ",SkipEmptyParts);
				bool hasOther = ValueList.size() == 2;

				float Value = lcParseValueLocalized(ValueList.first());
				float OtherValue = hasOther ? lcParseValueLocalized(ValueList.last()) : 0.0f;

				float Latitude, Longitude, Distance;
				Camera->GetAngles(Latitude,Longitude,Distance);
				if (Item == cameraGlobeLatitude) {
					Latitude = Value;
					if (hasOther)
					   Longitude = OtherValue;
				} else if (Item == cameraGlobeLongitude) {
					if (hasOther)
						Latitude = OtherValue;
					Longitude = Value;
				}

				Model->SetCameraGlobe(Camera, Latitude, Longitude, Distance);
			}
			else if (Item == pictureImageSizeWidth || Item == pictureImageSizeHeight)
			{
				float Value = lcParseValueLocalized(Editor->text());

				if (Item == pictureImageSizeWidth) {
					lcGetActiveProject()->SetImageSize(
								int(Value),
								lcGetActiveProject()->GetImageHeight());
				} else if (Item == pictureImageSizeHeight) {
					lcGetActiveProject()->SetImageSize(
								lcGetActiveProject()->GetImageWidth(),
								int(Value));
				}
			}
/*** LPub3D Mod end ***/
			else if (Item == cameraFOV)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetCameraFOV(Camera, Value);
			}
			else if (Item == cameraNear)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetCameraZNear(Camera, Value);
			}
			else if (Item == cameraFar)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetCameraZFar(Camera, Value);
			}
			else if (Item == mCameraNameItem)
			{
				QString Value = Editor->text();

				Model->SetCameraName(Camera, Value);
			}
/*** LPub3D Mod - Camera Globe ***/
			lcArray<lcObject*> Selection;
			Update(Selection, Camera);
/*** LPub3D Mod end ***/
		}
	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		lcLight* Light  = (mFocus && mFocus->IsLight()) ? (lcLight*)mFocus : nullptr;

		if (Light)
		{
			lcLightProperties Props = Light->GetLightProperties();

			QString Name = Light->GetName();

			if (Item == lightPositionX || Item == lightPositionY || Item == lightPositionZ)
			{
				lcVector3 Center = Light->GetPosition();
				lcVector3 Position = Center;

				float Value = lcParseValueLocalized(Editor->text());
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
				if (Item == lightPositionX)
					Position[X] = Value;
				else if (Item == lightPositionY)
					Position[Z] = -Value;
				else if (Item == lightPositionZ)
					Position[Y] = Value;
/*** LPub3D Mod end ***/

				lcVector3 Distance = Position - Center;

				Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == lightTargetX || Item == lightTargetY || Item == lightTargetZ)
			{
//				lcVector3 Center = Light->mTargetPosition;
//				lcVector3 Position = Center;
//				float Value = lcParseValueLocalized(Editor->text());
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
//				if (Item == lightTargetX)
//					Position[X] = Value;
//				else if (Item == lightTargetY)
//					Position[Z] = -Value;
//				else if (Item == lightTargetZ)
//					Position[Y] = Value;
/*** LPub3D Mod end ***/

//				lcVector3 Distance = Position - Center;

//				Model->MoveSelectedObjects(Distance, Distance, false, false, true, true);
			}
			else if (Item == mLightSpotConeAngleItem)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetSpotLightConeAngle(Light, Value);
			}
			else if (Item == mLightSpotPenumbraAngleItem)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetSpotLightPenumbraAngle(Light, Value);
			}
			else if (Item == mLightSpotTightnessItem)
			{
				float Value = lcParseValueLocalized(Editor->text());

				Model->SetSpotLightTightness(Light, Value);
			}
			else if (Item == lightFactorA || Item == lightFactorB)
			{
				float Value = lcParseValueLocalized(Editor->text());
				if (Item == lightFactorA)
					Props.mLightFactor[0] = Value;
				else if (Item == lightFactorB)
					Props.mLightFactor[1] = Value;

				Model->UpdateLight(Light, Props, LC_LIGHT_FACTOR);
			}
			else if (Item == lightDiffuse)
			{
				Props.mLightDiffuse = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_DIFFUSE);
			}
			else if (Item == lightSpecular)
			{
				Props.mLightSpecular = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_SPECULAR);
			}
			else if (Item == lightExponent)
			{
				Props.mSpotExponent = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_EXPONENT);
			}
			else if (Item == lightCutoff)
			{
				Props.mSpotCutoff = lcParseValueLocalized(Editor->text());

				Model->UpdateLight(Light, Props, LC_LIGHT_CUTOFF);
			}
			else if (Item == lightAreaGridRows || Item == lightAreaGridColumns)
			{
				float Value = lcParseValueLocalized(Editor->text());
				if (Item == lightAreaGridRows)
					Props.mAreaGrid[0] = Value;
				else if (Item == lightAreaGridColumns)
					Props.mAreaGrid[1] = Value;

				Model->UpdateLight(Light, Props, LC_LIGHT_AREA_GRID);
			}

			else if (Item == mLightNameItem)
			{
				QString Value = Editor->text();

				Model->SetLightName(Light, Value);
			}
		}
	}
}

void lcQPropertiesTree::slotSetValue(int Value)
{
	QTreeWidgetItem* Item = mDelegate->editedItem();
	lcModel* Model = gMainWindow->GetActiveModel();

	if (mWidgetMode == LC_PROPERTY_WIDGET_PIECE)
	{
		if (Item == mPieceColorItem)
		{
			Model->SetSelectedPiecesColorIndex(Value);

			QPushButton *editor = (QPushButton*)mDelegate->editor();
			updateColorEditor(editor, Value);
		}
		else if (Item == mPieceIdItem)
		{
			QComboBox *editor = (QComboBox*)sender();

			PieceInfo* Info = (PieceInfo*)editor->itemData(Value).value<void*>();
			Model->SetSelectedPiecesPieceInfo(Info);

/*** LPub3D Mod - preview widget for LPub3D ***/
			lcPreferences& Preferences = lcGetPreferences();
			if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition != lcPreviewPosition::Floating)
			{
/*** LPub3D Mod end ***/
				int ColorIndex = gDefaultColor;
				lcObject* Focus = gMainWindow->GetActiveModel()->GetFocusObject();
				if (Focus && Focus->IsPiece())
					ColorIndex = ((lcPiece*)Focus)->GetColorIndex();
				quint32 ColorCode = lcGetColorCode(ColorIndex);
				gMainWindow->PreviewPiece(Info->mFileName, ColorCode, false);
/*** LPub3D Mod - preview widget for LPub3D ***/
			}
/*** LPub3D Mod end ***/
		}
	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_CAMERA)
	{
		lcObject* Focus = Model->GetFocusObject();

		if (Focus && Focus->IsCamera())
		{
			lcCamera* Camera = (lcCamera*)Focus;

			if (Item == mCameraProjectionItem)
			{
				Model->SetCameraOrthographic(Camera, Value == 1);
			}
		}
	}
	else if (mWidgetMode == LC_PROPERTY_WIDGET_LIGHT)
	{
		lcObject* Focus = Model->GetFocusObject();
		lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

		if (Light)
		{
			lcLightProperties Props = Light->GetLightProperties();

			if (Item == mLightTypeItem)
			{
				Model->SetLightType(Light, static_cast<lcLightType>(Value));
			}
			else if (Item == lightShape)
			{
				Props.mLightShape = static_cast<lcLightShape>(Value);
				Model->UpdateLight(Light, Props, LC_LIGHT_SHAPE);
			}
			else if (Item == lightFormat)
			{
				Props.mPOVRayLight =  Value;
				Model->UpdateLight(Light, Props, LC_LIGHT_POVRAY);
			}
		}
	}
}

void lcQPropertiesTree::slotColorButtonClicked()
{
	int ColorIndex = gDefaultColor;
	lcObject* Focus = gMainWindow->GetActiveModel()->GetFocusObject();

	if (Focus && Focus->IsPiece())
		ColorIndex = ((lcPiece*)Focus)->GetColorIndex();

	QWidget* Button = (QWidget*)sender();

	if (!Button)
		return;

	lcColorPickerPopup* Popup = new lcColorPickerPopup(Button, ColorIndex);
	connect(Popup, &lcColorPickerPopup::Selected, this, &lcQPropertiesTree::slotSetValue);
	Popup->setMinimumSize(qMax(300, width()), qMax(200, static_cast<int>(width() * 2 / 3)));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	QScreen* Screen = Button->screen();
	const QRect ScreenRect = Screen ? Screen->geometry() : QRect();
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	QScreen* Screen = QGuiApplication::screenAt(Button->mapToGlobal(Button->rect().bottomLeft()));
	const QRect ScreenRect = Screen ? Screen->geometry() : QApplication::desktop()->geometry();
#else
	const QRect ScreenRect = QApplication::desktop()->geometry();
#endif

	int x = mapToGlobal(QPoint(0, 0)).x();
	int y = Button->mapToGlobal(Button->rect().bottomLeft()).y();

	if (x < ScreenRect.left())
		x = ScreenRect.left();
	if (y < ScreenRect.top())
		y = ScreenRect.top();

	if (x + Popup->width() > ScreenRect.right())
		x = ScreenRect.right() - Popup->width();
	if (y + Popup->height() > ScreenRect.bottom())
		y = ScreenRect.bottom() - Popup->height();

	Popup->move(QPoint(x, y));
	Popup->setFocus();
	Popup->show();
}

void lcQPropertiesTree::LightColorButtonClicked()
{
	lcModel* Model = gMainWindow->GetActiveModel();
	lcObject* Focus = Model->GetFocusObject();
	lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

	if (!Light)
		return;

	QColor Color = QColorDialog::getColor(lcQColorFromVector3(Light->GetColor()), this, tr("Select Light Color"));

	if (!Color.isValid())
		return;

	Model->SetLightColor(Light, lcVector3FromQColor(Color));

	QPushButton* Editor = qobject_cast<QPushButton*>(mDelegate->editor());

	if (Editor)
		UpdateLightColorEditor(Editor, Color);
}

QTreeWidgetItem *lcQPropertiesTree::addProperty(QTreeWidgetItem *parent, const QString& label, PropertyType propertyType)
{
	QTreeWidgetItem *newItem;

	if (parent)
		newItem = new QTreeWidgetItem(parent, QStringList(label));
	else
		newItem = new QTreeWidgetItem(this, QStringList(label));

	newItem->setData(0, PropertyTypeRole, QVariant(propertyType));

	if (propertyType != PropertyFloatReadOnly)
		newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

	newItem->setExpanded(true);

	if (propertyType == PropertyGroup)
	{
		newItem->setFirstColumnSpanned(true);
		newItem->setIcon(0, m_expandIcon);
	}

	return newItem;
}

void lcQPropertiesTree::SetEmpty()
{
	clear();

	mPieceAttributesItem = nullptr;
	partPosition = nullptr;
	partPositionX = nullptr;
	partPositionY = nullptr;
	partPositionZ = nullptr;
	partRotation = nullptr;
	partRotationX = nullptr;
	partRotationY = nullptr;
	partRotationZ = nullptr;
	partVisibility = nullptr;
	partShow = nullptr;
	partHide = nullptr;
	partAppearance = nullptr;
	mPieceColorItem = nullptr;
	mPieceIdItem = nullptr;
/*** LPub3D Mod - part properties ***/
	partFileName = nullptr;
	partModel = nullptr;
	partType = nullptr;
	partIsSubmodel = nullptr;
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe ***/
	cameraGlobe = nullptr;
	cameraGlobeLatitude = nullptr;
	cameraGlobeLongitude = nullptr;
	cameraGlobeDistance = nullptr;
/*** LPub3D Mod end ***/
	cameraPosition = nullptr;
	cameraPositionX = nullptr;
	cameraPositionY = nullptr;
	cameraPositionZ = nullptr;
	cameraTarget = nullptr;
	cameraTargetX = nullptr;
	cameraTargetY = nullptr;
	cameraTargetZ = nullptr;
	cameraUp = nullptr;
	cameraUpX = nullptr;
	cameraUpY = nullptr;
	cameraUpZ = nullptr;
	mCameraAttributesItem = nullptr;
	mCameraProjectionItem = nullptr;
	cameraFOV = nullptr;
	cameraNear = nullptr;
	cameraFar = nullptr;
	mCameraNameItem = nullptr;

/*** LPub3D Mod - Camera Globe ***/
	picture = nullptr;
	pictureModelScale = nullptr;
	pictureResolution = nullptr;
	picturePageSizeWidth = nullptr;
	picturePageSizeHeight = nullptr;
	pictureImageSizeWidth = nullptr;
	pictureImageSizeHeight = nullptr;
/*** LPub3D Mod end ***/

	lightConfiguration = nullptr;
	lightPosition = nullptr;
	lightPositionX = nullptr;
	lightPositionY = nullptr;
	lightPositionZ = nullptr;
	lightTarget = nullptr;
	lightTargetX = nullptr;
	lightTargetY = nullptr;
	lightTargetZ = nullptr;
	mLightColorItem = nullptr;
	mLightAttributesItem = nullptr;
	lightDiffuse = nullptr;
	lightSpecular = nullptr;
	lightCutoff = nullptr;
	lightEnableCutoff = nullptr;
	lightExponent = nullptr;
	mLightTypeItem = nullptr;
	lightFactorA = nullptr;
	lightFactorB = nullptr;
	mLightNameItem = nullptr;
	mLightSpotConeAngleItem = nullptr;
	mLightSpotPenumbraAngleItem = nullptr;
	mLightSpotTightnessItem = nullptr;
	lightShape = nullptr;
	lightFormat = nullptr;
	mLightCastShadowItem = nullptr;
	lightAreaGridRows = nullptr;
	lightAreaGridColumns = nullptr;

	mWidgetMode = LC_PROPERTY_WIDGET_EMPTY;
	mFocus = nullptr;
}

void lcQPropertiesTree::SetPiece(const lcArray<lcObject*>& Selection, lcObject* Focus)
{
	if (mWidgetMode != LC_PROPERTY_WIDGET_PIECE)
	{
		SetEmpty();

		mPieceAttributesItem = addProperty(nullptr, tr("Piece Attributes"), PropertyGroup);
		mPieceIdItem = addProperty(mPieceAttributesItem, tr("Part"), PropertyPart);
		mPieceColorItem = addProperty(mPieceAttributesItem, tr("Color"), PropertyPieceColor);

/*** LPub3D Mod - Add LPub3D attributes ***/
		partFileName = addProperty(partAppearance, tr("File ID"), PropertyPart);
		partModel = addProperty(partAppearance, tr("Model"), PropertyPart);
		partType = addProperty(partAppearance, tr("Type"), PropertyPart);
		partIsSubmodel = addProperty(partAppearance, tr("Submodel"), PropertyPart);
/*** LPub3D Mod end ***/

		partVisibility = addProperty(nullptr, tr("Visible Steps"), PropertyGroup);
		partShow = addProperty(partVisibility, tr("Show"), PropertyStep);
		partHide = addProperty(partVisibility, tr("Hide"), PropertyStep);

/*** LPub3D Mod - Switch Rotation and Position attributes ***/
		partRotation  = addProperty(nullptr, tr("Rotation"), PropertyGroup);
		partRotationX = addProperty(partRotation, tr("X"), PropertyFloat);
		partRotationY = addProperty(partRotation, tr("Y"), PropertyFloat);
		partRotationZ = addProperty(partRotation, tr("Z"), PropertyFloat);

		partPosition  = addProperty(nullptr, tr("Position"), PropertyGroup);
		partPositionX = addProperty(partPosition, tr("X"), PropertyFloat);
		partPositionY = addProperty(partPosition, tr("Y"), PropertyFloat);
		partPositionZ = addProperty(partPosition, tr("Z"), PropertyFloat);
/*** LPub3D Mod end ***/

		mWidgetMode = LC_PROPERTY_WIDGET_PIECE;
	}

	lcModel* Model = gMainWindow->GetActiveModel();
	lcPiece* Piece = (Focus && Focus->IsPiece()) ? (lcPiece*)Focus : nullptr;
	mFocus = Piece;

	lcVector3 Position;
	lcMatrix33 RelativeRotation;
	Model->GetMoveRotateTransform(Position, RelativeRotation);
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	partPositionX->setText(1, lcFormatValueLocalized(Position[X]));
	partPositionX->setData(0, PropertyValueRole, Position[X]);
	partPositionY->setText(1, lcFormatValueLocalized(-Position[Z]));
	partPositionY->setData(0, PropertyValueRole, -Position[Z]);
	partPositionZ->setText(1, lcFormatValueLocalized(Position[Y]));
	partPositionZ->setData(0, PropertyValueRole, Position[Y]);
/*** LPub3D Mod end ***/

	lcVector3 Rotation;
	if (Piece)
		Rotation = lcMatrix44ToEulerAngles(Piece->mModelWorld) * LC_RTOD;
	else
		Rotation = lcVector3(0.0f, 0.0f, 0.0f);
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	partRotationX->setText(1, lcFormatValueLocalized(Rotation[X]));
	partRotationX->setData(0, PropertyValueRole, Rotation[X]);
	partRotationY->setText(1, lcFormatValueLocalized(-Rotation[Z]));
	partRotationY->setData(0, PropertyValueRole, -Rotation[Z]);
	partRotationZ->setText(1, lcFormatValueLocalized(Rotation[Y]));
	partRotationZ->setData(0, PropertyValueRole, Rotation[Y]);
/*** LPub3D Mod end ***/

	lcStep Show = 0;
	lcStep Hide = 0;
	int ColorIndex = gDefaultColor;
	PieceInfo* Info = nullptr;

/*** LPub3D Mod - Add Model Properties ***/
	const lcModelProperties& ModelInfo = Model->GetProperties();
/*** LPub3D Mod end ***/
/*** LPub3D Mod - preview widget for LPub3D ***/
	lcPreferences& Preferences = lcGetPreferences();

	if (Piece)
	{
		Show = Piece->GetStepShow();
		Hide = Piece->GetStepHide();
/*** LPub3D Mod - preview widget for LPub3D ***/
		Info = Piece->mPieceInfo;
		ColorIndex = Info->IsModel() ? lcGetColorIndex(LDRAW_MATERIAL_COLOUR) : Piece->GetColorIndex();
		if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition == lcPreviewPosition::Dockable)
		{
/*** LPub3D Mod end ***/
			quint32 ColorCode = lcGetColorCode(ColorIndex);
			gMainWindow->PreviewPiece(Info->mFileName, ColorCode, false);
/*** LPub3D Mod - preview widget for LPub3D ***/
		}
/*** LPub3D Mod end ***/
	}
	else
	{
		bool FirstPiece = true;

		for (int ObjectIdx = 0; ObjectIdx < Selection.GetSize(); ObjectIdx++)
		{
			lcObject* Object = Selection[ObjectIdx];

			if (!Object->IsPiece())
				continue;

			lcPiece* SelectedPiece = (lcPiece*)Object;

			if (FirstPiece)
			{
				Show = SelectedPiece->GetStepShow();
				Hide = SelectedPiece->GetStepHide();
				ColorIndex = SelectedPiece->GetColorIndex();
				Info = SelectedPiece->mPieceInfo;
/*** LPub3D Mod - preview widget for LPub3D ***/
				if (Preferences.mPreviewEnabled && Preferences.mPreviewPosition == lcPreviewPosition::Dockable)
				{
					quint32 ColorCode = lcGetColorCode(ColorIndex);
					gMainWindow->PreviewPiece(Info->mFileName, ColorCode, false);
				}
/*** LPub3D Mod end ***/
				FirstPiece = false;
			}
			else
			{
				if (SelectedPiece->GetStepShow() != Show)
					Show = 0;

				if (SelectedPiece->GetStepHide() != Hide)
					Hide = 0;

				if (SelectedPiece->GetColorIndex() != ColorIndex)
					ColorIndex = gDefaultColor;

				if (SelectedPiece->mPieceInfo != Info)
					Info = nullptr;
			}
		}
	}

	partShow->setText(1, QString::number(Show));
	partShow->setData(0, PropertyValueRole, Show);
	partHide->setText(1, Hide == LC_STEP_MAX ? QString() : QString::number(Hide));
	partHide->setData(0, PropertyValueRole, Hide);

	QImage img(16, 16, QImage::Format_ARGB32);
	img.fill(0);

	lcColor* color = &gColorList[ColorIndex];
	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(QColor::fromRgbF(color->Value[0], color->Value[1], color->Value[2]));
	painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
	painter.end();

	mPieceColorItem->setIcon(1, QIcon(QPixmap::fromImage(img)));
	mPieceColorItem->setText(1, color->Name);
	mPieceColorItem->setData(0, PropertyValueRole, ColorIndex);

	QString text = Info ? Info->m_strDescription : QString();
	mPieceIdItem->setText(1, text);
	mPieceIdItem->setToolTip(1, text);
	mPieceIdItem->setData(0, PropertyValueRole, QVariant::fromValue((void*)Info));

/*** LPub3D Mod - Add LPub3D attributes ***/
	partFileName->setText(1, Info ? Info->mFileName : QString());
	partFileName->setData(0, PropertyValueRole, QVariant::fromValue((void*)Info));

	partModel->setText(1, ModelInfo.mFileName);
	partModel->setData(0, PropertyValueRole, QVariant::fromValue((void*)Info));

	partType->setText(1, Info ? Info->mZipFileType == lcZipFileType::Official ? QString("Official Part") : QString("Unofficial Part") : QString());
	partType->setData(0, PropertyValueRole, QVariant::fromValue((void*)Info));

	partIsSubmodel->setText(1, Info ? Info->IsModel() ? QString("Yes") : QString("No") : QString());
	partIsSubmodel->setData(0, PropertyValueRole, QVariant::fromValue((void*)Info));
/*** LPub3D Mod end ***/
}

void lcQPropertiesTree::SetCamera(lcObject* Focus)
{
	if (mWidgetMode != LC_PROPERTY_WIDGET_CAMERA)
	{
		SetEmpty();

		mCameraAttributesItem = addProperty(nullptr, tr("Camera Attributes"), PropertyGroup);
		mCameraNameItem = addProperty(mCameraAttributesItem, tr("Name"), PropertyString);
		mCameraProjectionItem = addProperty(mCameraAttributesItem, tr("Projection"), PropertyStringList);
		cameraFOV = addProperty(mCameraAttributesItem, tr("FOV"), PropertyFloat);
		cameraNear = addProperty(mCameraAttributesItem, tr("Near"), PropertyFloat);
		cameraFar = addProperty(mCameraAttributesItem, tr("Far"), PropertyFloat);

/*** LPub3D Mod - Camera Globe ***/
		cameraGlobe = addProperty(nullptr, tr("Camera Globe"), PropertyGroup);
		cameraGlobeLatitude = addProperty(cameraGlobe, tr("Latitude"), PropertyFloatCameraAngle);
		cameraGlobeLongitude = addProperty(cameraGlobe, tr("Longitude"), PropertyFloatCameraAngle);
		cameraGlobeDistance = addProperty(cameraGlobe, tr("Distance"), PropertyIntReadOnly);
/*** LPub3D Mod end ***/

		cameraPosition = addProperty(nullptr, tr("Position"), PropertyGroup);
		cameraPositionX = addProperty(cameraPosition, tr("X"), PropertyFloat);
		cameraPositionY = addProperty(cameraPosition, tr("Y"), PropertyFloat);
		cameraPositionZ = addProperty(cameraPosition, tr("Z"), PropertyFloat);

		cameraTarget = addProperty(nullptr, tr("Target"), PropertyGroup);
		cameraTargetX = addProperty(cameraTarget, tr("X"), PropertyFloatTarget);
		cameraTargetY = addProperty(cameraTarget, tr("Y"), PropertyFloatTarget);
		cameraTargetZ = addProperty(cameraTarget, tr("Z"), PropertyFloatTarget);

		cameraUp = addProperty(nullptr, tr("Up"), PropertyGroup);
		cameraUpX = addProperty(cameraUp, tr("X"), PropertyFloat);
		cameraUpY = addProperty(cameraUp, tr("Y"), PropertyFloat);
		cameraUpZ = addProperty(cameraUp, tr("Z"), PropertyFloat);

/*** LPub3D Mod - Camera Globe ***/
		picture = addProperty(nullptr, tr("Model Image"), PropertyGroup);
		pictureModelScale = addProperty(picture, tr("Scale"), PropertyFloatReadOnly);
		pictureResolution = addProperty(picture, tr("Resolution"), PropertyFloatReadOnly);
		picturePageSizeWidth = addProperty(picture, tr("Page Width"), PropertyFloatReadOnly);
		picturePageSizeHeight = addProperty(picture, tr("Page Height"), PropertyFloatReadOnly);
		pictureImageSizeWidth = addProperty(picture, tr("Image Width"), PropertyFloat);
		pictureImageSizeHeight = addProperty(picture, tr("Image Height"), PropertyFloat);
/*** LPub3D Mod end ***/

		mWidgetMode = LC_PROPERTY_WIDGET_CAMERA;
	}

	lcCamera* Camera = (Focus && Focus->IsCamera()) ? (lcCamera*)Focus : nullptr;
	mFocus = Camera;

	lcVector3 Position(0.0f, 0.0f, 0.0f);
	lcVector3 Target(0.0f, 0.0f, 0.0f);
	lcVector3 UpVector(0.0f, 0.0f, 0.0f);
/*** LPub3D Mod - Camera Globe ***/
	float Latitude      = 0.0f;
	float Longitude     = 0.0f;
	float ModelScale    = 0.0f;
	float Distance      = 0.0f;
	float Resolution    = 0.0f;
	int PageSizeWidth   = 0;
	int PageSizeHeight  = 0;
	int ImageSizeWidth  = 0;
	int ImageSizeHeight = 0;
/*** LPub3D Mod end ***/
	bool Ortho = false;
	float FoV = 60.0f;
	float ZNear = 1.0f;
	float ZFar = 100.0f;
	QString Name;

	if (Camera)
	{
/*** LPub3D Mod - Camera Globe ***/
		Camera->GetAngles(Latitude,Longitude,Distance);

		Position = Camera->mPosition; // normalizeDegrees(Camera->mPosition);
		Target	 = Camera->mTargetPosition; // normalizeDegrees(Camera->mTargetPosition);
		UpVector = Camera->mUpVector; // normalizeDegrees(Camera->mUpVector);
/*** LPub3D Mod end ***/

		Ortho = Camera->IsOrtho();
		FoV = Camera->m_fovy;
		ZNear = Camera->m_zNear;
		ZFar = Camera->m_zFar;
		Name = Camera->GetName();
/*** LPub3D Mod - Camera Globe ***/
		if (Name.isEmpty())
			Name = "Default";
		ModelScale      = Camera->GetScale();
		Resolution      = lcGetActiveProject()->GetResolution();
		PageSizeWidth   = lcGetActiveProject()->GetPageWidth();
		PageSizeHeight  = lcGetActiveProject()->GetPageHeight();
		ImageSizeWidth  = lcGetActiveProject()->GetImageWidth();
		ImageSizeHeight = lcGetActiveProject()->GetImageHeight();
/*** LPub3D Mod end ***/
	}

/*** LPub3D Mod - Camera Globe ***/
	cameraGlobeLatitude->setText(1, lcFormatValueLocalized(Latitude));
	cameraGlobeLatitude->setData(0, PropertyValueRole, Latitude);
	cameraGlobeLongitude->setText(1, lcFormatValueLocalized(Longitude));
	cameraGlobeLongitude->setData(0, PropertyValueRole, Longitude);
	cameraGlobeDistance->setText(1, lcFormatValueLocalized(qRound(Distance)));
	cameraGlobeDistance->setData(0, PropertyValueRole, qRound(Distance));
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	cameraPositionX->setText(1, lcFormatValueLocalized(Position[X]));
	cameraPositionX->setData(0, PropertyValueRole, Position[X]);
	cameraPositionY->setText(1, lcFormatValueLocalized(-Position[Z]));
	cameraPositionY->setData(0, PropertyValueRole, -Position[Z]);
	cameraPositionZ->setText(1, lcFormatValueLocalized(Position[Y]));
	cameraPositionZ->setData(0, PropertyValueRole, Position[Y]);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	cameraTargetX->setText(1, lcFormatValueLocalized(Target[X]));
	cameraTargetX->setData(0, PropertyValueRole, Target[X]);
	cameraTargetY->setText(1, lcFormatValueLocalized(-Target[Z]));
	cameraTargetY->setData(0, PropertyValueRole, -Target[Z]);
	cameraTargetZ->setText(1, lcFormatValueLocalized(Target[Y]));
	cameraTargetZ->setData(0, PropertyValueRole, Target[Y]);
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	cameraUpX->setText(1, lcFormatValueLocalized(UpVector[X]));
	cameraUpX->setData(0, PropertyValueRole, UpVector[X]);
	cameraUpY->setText(1, lcFormatValueLocalized(-UpVector[Z]));
	cameraUpY->setData(0, PropertyValueRole, -UpVector[Z]);
	cameraUpZ->setText(1, lcFormatValueLocalized(UpVector[Y]));
	cameraUpZ->setData(0, PropertyValueRole, UpVector[Y]);
/*** LPub3D Mod end ***/

	mCameraProjectionItem->setText(1, Ortho ? tr("Orthographic") : tr("Perspective"));
	mCameraProjectionItem->setData(0, PropertyValueRole, Ortho);
	cameraFOV->setText(1, lcFormatValueLocalized(FoV));
	cameraFOV->setData(0, PropertyValueRole, FoV);
	cameraNear->setText(1, lcFormatValueLocalized(ZNear));
	cameraNear->setData(0, PropertyValueRole, ZNear);
	cameraFar->setText(1, lcFormatValueLocalized(ZFar));
	cameraFar->setData(0, PropertyValueRole, ZFar);

	mCameraNameItem->setText(1, Name);
	mCameraNameItem->setData(0, PropertyValueRole, Name);

/*** LPub3D Mod - Camera Globe ***/
	pictureModelScale->setText(1, lcFormatValueLocalized(ModelScale));
	pictureModelScale->setData(0, PropertyValueRole, ModelScale);
	pictureResolution->setText(1, lcFormatValueLocalized(Resolution));
	pictureResolution->setData(0, PropertyValueRole, Resolution);
	picturePageSizeWidth->setText(1, lcFormatValueLocalized(PageSizeWidth));
	picturePageSizeWidth->setData(0, PropertyValueRole, PageSizeWidth);
	picturePageSizeHeight->setText(1, lcFormatValueLocalized(PageSizeHeight));
	picturePageSizeHeight->setData(0, PropertyValueRole, PageSizeHeight);
	pictureImageSizeWidth->setText(1, lcFormatValueLocalized(ImageSizeWidth));
	pictureImageSizeWidth->setData(0, PropertyValueRole, ImageSizeWidth);
	pictureImageSizeHeight->setText(1, lcFormatValueLocalized(ImageSizeHeight));
	pictureImageSizeHeight->setData(0, PropertyValueRole, ImageSizeHeight);
/*** LPub3D Mod end ***/

}

void lcQPropertiesTree::SetLight(lcObject* Focus)
{
	lcLight* Light = (Focus && Focus->IsLight()) ? (lcLight*)Focus : nullptr;

	QString Name = tr("Light");
	QString ExponentLabel = tr("Exponent");
	QString FactorALabel = QLatin1String("FactorA");
	QString Format, Shape, ExponentToolTip, FactorAToolTip, FactorBToolTip;
	lcLightType LightType = lcLightType::Point;
	lcLightShape ShapeIndex = LC_LIGHT_SHAPE_UNDEFINED;
	int FormatIndex = 0;
	float Diffuse = 0.0f;
	float Specular = 0.0f;
	float Cutoff = 0.0f;
	float Exponent = 0.0f;
	bool EnableCutoff = false;
	bool POVRayLight = false;
	bool CastShadow = true;
	PropertyType TargetProperty = PropertyFloat;
	lcVector3 Position(0.0f, 0.0f, 0.0f);
	lcVector3 Target(0.0f, 0.0f, 0.0f);
	QColor Color(Qt::white);
	float SpotConeAngle = 0.0f, SpotPenumbraAngle = 0.0f, SpotTightness = 0.0f;
	lcVector2 Factor(0.0f, 0.0f);
	lcVector2 AreaGrid(0.0f, 0.0f);

	if (Light)
	{
		Name = Light->GetName();

		POVRayLight = Light->mPOVRayLight;
		FormatIndex = static_cast<int>(POVRayLight);
		Format = POVRayLight ? QLatin1String("POVRay") : QLatin1String("Blender");

		CastShadow = Light->GetCastShadow();
		Position = Light->GetPosition();
//		Target = Light->mTargetPosition;
		Color = lcQColorFromVector3(Light->GetColor());
		SpotConeAngle = Light->GetSpotConeAngle();
		SpotPenumbraAngle = Light->GetSpotPenumbraAngle();

		Factor = Light->mLightFactor;
		LightType = Light->GetLightType();

		switch(LightType)
		{
		case lcLightType::Point:
			FactorALabel = tr("Radius (m)");
			FactorAToolTip = tr("The light size for shadow sampling in metres.");
			ExponentLabel = tr("Exponent");
			break;
		case lcLightType::Spot:
			FactorBToolTip = tr("The softness of the spotlight edge.");
			ExponentLabel = tr("Power");

			if (POVRayLight)
			{
				FactorALabel = tr("Radius (°)");
				FactorAToolTip = tr("The angle between the \"hot-spot\" edge at the beam center and the center line.");
			}
			else
			{
				FactorALabel = tr("Radius (m)");
				FactorAToolTip = tr("Shadow soft size - Light size in metres for shadow sampling.");
			}
			break;
		case lcLightType::Sun:
			FactorALabel = tr("Angle (°)");
			FactorAToolTip = tr("Angular diamater of the sun as seen from the Earth.");
			ExponentLabel = tr("Strength");
			break;
		case lcLightType::Area:
			ExponentLabel = tr("Power");

			if (POVRayLight)
			{
				Factor = Light->mAreaSize;
				FactorALabel = tr("Width");
				FactorAToolTip = tr("The width (X direction) of the area light in units.");
				FactorBToolTip = tr("The height (Y direction) of the area light in units.");
				ExponentLabel = tr("Power");
			}
			else
			{
				FactorALabel = tr("Width (m)");
				FactorAToolTip = tr("The width (X direction) of the area light in metres.");
				FactorBToolTip = tr("The height (Y direction) of the area light in units.");
			}
			break;
		default:
			break;
		}

		ShapeIndex = static_cast<lcLightShape>(Light->GetLightShape());

		switch(ShapeIndex)
		{
		case LC_LIGHT_SHAPE_SQUARE:
			Shape = tr("Square");
			break;
		case LC_LIGHT_SHAPE_DISK:
			Shape = POVRayLight ? tr("Circle") : tr("Disk");
			break;
		case LC_LIGHT_SHAPE_RECTANGLE:
			Shape = tr("Rectangle");
			break;
		case LC_LIGHT_SHAPE_ELLIPSE:
			Shape = tr("Ellipse");
			break;
		default:
			break;
		}

		Diffuse = Light->mLightDiffuse;
		Specular = Light->mLightSpecular;

		if (POVRayLight)
		{
			Exponent = Light->mPOVRayExponent;
			ExponentToolTip = tr("Intensity of the light with typical range of 0 to 1.0.");
		}
		else
		{
			Exponent = Light->mSpotExponent;
			ExponentToolTip = tr("Intensity of the light in watts.");
		}

		ExponentLabel = LightType == lcLightType::Sun ? tr("Strength") : tr("Power");
		Cutoff = Light->mSpotCutoff;
		EnableCutoff = Light->mEnableCutoff;
		TargetProperty = LightType != lcLightType::Point ? PropertyFloat : PropertyFloatReadOnly;
		SpotTightness = Light->GetSpotTightness();
		AreaGrid = Light->mAreaGrid;
	}

	if (mWidgetMode != LC_PROPERTY_WIDGET_LIGHT || mLightType != LightType || mLightShape != ShapeIndex || mPOVRayLight != POVRayLight)
	{
		SetEmpty();

		// Attributes
		mLightAttributesItem = addProperty(nullptr, tr("Light Attributes"), PropertyGroup);
		mLightNameItem = addProperty(mLightAttributesItem, tr("Name"), PropertyString);
		mLightTypeItem = addProperty(mLightAttributesItem, tr("Type"), PropertyStringList);
		mLightColorItem = addProperty(mLightAttributesItem, tr("Color"), PropertyColor);
		mLightCastShadowItem = addProperty(mLightAttributesItem, tr("Cast Shadows"), PropertyBool);

		if (LightType == lcLightType::Spot)
		{
			mLightSpotConeAngleItem = addProperty(mLightAttributesItem, tr("Spot Cone Angle"), PropertyFloat);
			mLightSpotConeAngleItem->setToolTip(1, tr("The angle (in degrees) of the spot light's beam."));

			mLightSpotPenumbraAngleItem = addProperty(mLightAttributesItem, tr("Spot Penumbra Angle"), PropertyFloat);
			mLightSpotPenumbraAngleItem->setToolTip(1, tr("The angle (in degrees) over which the intensity of the spot light falls off to zero."));

			mLightSpotTightnessItem = addProperty(mLightAttributesItem, tr("Spot Tightness"), PropertyFloat);
			mLightSpotTightnessItem->setToolTip(1, tr("Additional exponential spotlight edge softening (POV-Ray only)."));
		}

		lightExponent = addProperty(mLightAttributesItem, ExponentLabel, PropertyFloat);

		if ((LightType == lcLightType::Point || LightType == lcLightType::Sun) && !POVRayLight)
			lightFactorA = addProperty(mLightAttributesItem, FactorALabel, PropertyFloat);

		if (LightType == lcLightType::Area)
		{
			lightShape = addProperty(mLightAttributesItem, tr("Shape"), PropertyLightShape);
			lightFactorA = addProperty(mLightAttributesItem, FactorALabel, PropertyFloat);

			if (ShapeIndex == LC_LIGHT_SHAPE_RECTANGLE || ShapeIndex == LC_LIGHT_SHAPE_ELLIPSE || POVRayLight)
				lightFactorB = addProperty(mLightAttributesItem, tr("Height (Y)"), PropertyFloat);
			else
				FactorAToolTip = tr("The size of the area light grid in metres.");

			if (POVRayLight)
			{
				lightAreaGridRows = addProperty(mLightAttributesItem, tr("Grid Rows"), PropertyFloat);
				lightAreaGridColumns = addProperty(mLightAttributesItem, tr("Grid Columns"), PropertyFloat);
			}
		}

		if (!POVRayLight)
		{
			if (LightType != lcLightType::Sun)
			{
				lightEnableCutoff = addProperty(mLightAttributesItem, tr("Cutoff"), PropertyBool);
				lightCutoff = addProperty(mLightAttributesItem, tr("Cutoff Distance"), PropertyFloat);
			}

			lightDiffuse = addProperty(mLightAttributesItem, tr("Diffuse"), PropertyFloat);
			lightSpecular = addProperty(mLightAttributesItem, tr("Specular"), PropertyFloat);
		}

		// Configuration
		lightConfiguration = addProperty(nullptr, tr("Configuration"), PropertyGroup);
		lightFormat = addProperty(lightConfiguration, tr("Format"), PropertyLightFormat);

		// Position
		lightPosition = addProperty(nullptr, tr("Position"), PropertyGroup);
		lightPositionX = addProperty(lightPosition, tr("X"), PropertyFloat);
		lightPositionY = addProperty(lightPosition, tr("Y"), PropertyFloat);
		lightPositionZ = addProperty(lightPosition, tr("Z"), PropertyFloat);

		// Target Position
		if (LightType != lcLightType::Point && !(LightType == lcLightType::Area && POVRayLight))
		{
			lightTarget = addProperty(nullptr, tr("Target"), PropertyGroup);
			lightTargetX = addProperty(lightTarget, tr("X"), TargetProperty);
			lightTargetY = addProperty(lightTarget, tr("Y"), TargetProperty);
			lightTargetZ = addProperty(lightTarget, tr("Z"), TargetProperty);
		}

		mWidgetMode = LC_PROPERTY_WIDGET_LIGHT;
		mLightType = LightType;
		mLightShape = ShapeIndex;
		mPOVRayLight = POVRayLight;
	}

	mFocus = Light;

/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	lightPositionX->setText(1, lcFormatValueLocalized(Position[X]));
	lightPositionX->setData(0, PropertyValueRole, Position[X]);
	lightPositionY->setText(1, lcFormatValueLocalized(-Position[Z]));
	lightPositionY->setData(0, PropertyValueRole, -Position[Z]);
	lightPositionZ->setText(1, lcFormatValueLocalized(Position[Y]));
	lightPositionZ->setData(0, PropertyValueRole, Position[Y]);
/*** LPub3D Mod end ***/

	if (LightType != lcLightType::Point && !(LightType == lcLightType::Area && POVRayLight))
	{
/*** LPub3D Mod - Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		lightTargetX->setText(1, lcFormatValueLocalized(Target[X]));
		lightTargetX->setData(0, PropertyValueRole, Target[X]);
		lightTargetY->setText(1, lcFormatValueLocalized(-Target[Z]));
		lightTargetY->setData(0, PropertyValueRole, -Target[Z]);
		lightTargetZ->setText(1, lcFormatValueLocalized(Target[Y]));
		lightTargetZ->setData(0, PropertyValueRole, Target[Y]);
/*** LPub3D Mod end ***/
	}

	QImage ColorImage(16, 16, QImage::Format_ARGB32);
	ColorImage.fill(0);

	QPainter painter(&ColorImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.setPen(Qt::darkGray);
	painter.setBrush(Color);
	painter.drawRect(0, 0, ColorImage.width() - 1, ColorImage.height() - 1);
	painter.end();

	mLightColorItem->setIcon(1, QIcon(QPixmap::fromImage(ColorImage)));
	mLightColorItem->setText(1, Color.name().toUpper());
	mLightColorItem->setData(0, PropertyValueRole, Color);

	lightFormat->setText(1, Format);
	lightFormat->setData(0, PropertyValueRole, FormatIndex);

	mLightTypeItem->setText(1, lcLight::GetLightTypeString(LightType));
	mLightTypeItem->setData(0, PropertyValueRole, static_cast<int>(LightType));

	mLightCastShadowItem->setCheckState(1, CastShadow ? Qt::Checked : Qt::Unchecked);
	mLightCastShadowItem->setData(0, PropertyValueRole, CastShadow);

	lightExponent->setText(1, lcFormatValueLocalized(Exponent));
	lightExponent->setData(0, PropertyValueRole, Exponent);
	lightExponent->setToolTip(1, ExponentToolTip);

	if ((LightType == lcLightType::Point || LightType == lcLightType::Sun) && !POVRayLight)
	{
		lightFactorA->setText(1, lcFormatValueLocalized(Factor[0]));
		lightFactorA->setData(0, PropertyValueRole, Factor[0]);
		lightFactorA->setToolTip(1, FactorAToolTip);
	}
	else if (LightType == lcLightType::Area)
	{
		lightShape->setText(1, Shape);
		lightShape->setData(0, PropertyValueRole, ShapeIndex);
		lightShape->setToolTip(1, tr("Suggested shape of the arealight."));

		lightFactorA->setText(1, lcFormatValueLocalized(Factor[0]));
		lightFactorA->setData(0, PropertyValueRole, Factor[0]);
		lightFactorA->setToolTip(1, FactorAToolTip);

		if (ShapeIndex == LC_LIGHT_SHAPE_RECTANGLE || ShapeIndex == LC_LIGHT_SHAPE_ELLIPSE || POVRayLight)
		{
			lightFactorB->setText(1, lcFormatValueLocalized(Factor[1]));
			lightFactorB->setData(0, PropertyValueRole, Factor[1]);
			lightFactorB->setToolTip(1, FactorBToolTip);
		}

		if (POVRayLight)
		{
			lightAreaGridRows->setText(1, lcFormatValueLocalized(AreaGrid[0]));
			lightAreaGridRows->setData(0, PropertyValueRole, AreaGrid[0]);
			lightAreaGridRows->setToolTip(1, tr("The number of sample rows in the area light."));

			lightAreaGridColumns->setText(1, lcFormatValueLocalized(AreaGrid[1]));
			lightAreaGridColumns->setData(0, PropertyValueRole, AreaGrid[1]);
			lightAreaGridColumns->setToolTip(1, tr("The number of sample columns in the area light."));
		}
	}
	else if (LightType == lcLightType::Spot)
	{
		mLightSpotConeAngleItem->setText(1, lcFormatValueLocalized(SpotConeAngle));
		mLightSpotConeAngleItem->setData(0, PropertyValueRole, SpotConeAngle);
		mLightSpotConeAngleItem->setData(0, PropertyRangeRole, QPointF(1.0, 180.0));

		mLightSpotPenumbraAngleItem->setText(1, lcFormatValueLocalized(SpotPenumbraAngle));
		mLightSpotPenumbraAngleItem->setData(0, PropertyValueRole, SpotPenumbraAngle);
		mLightSpotPenumbraAngleItem->setData(0, PropertyRangeRole, QPointF(0.0, 180.0));

		mLightSpotTightnessItem->setText(1, lcFormatValueLocalized(SpotTightness));
		mLightSpotTightnessItem->setData(0, PropertyValueRole, SpotTightness);
		mLightSpotTightnessItem->setData(0, PropertyRangeRole, QPointF(0.0, 100.0));
	}

	if (!POVRayLight)
	{
		if (LightType != lcLightType::Sun)
		{
			lightEnableCutoff->setCheckState(1, EnableCutoff ? Qt::Checked : Qt::Unchecked);
			lightEnableCutoff->setData(0, PropertyValueRole, EnableCutoff);

			lightCutoff->setText(1, lcFormatValueLocalized(Cutoff));
			lightCutoff->setData(0, PropertyValueRole, Cutoff);
			lightCutoff->setToolTip(1, tr("Distance at which the light influence will be set to 0."));
		}

		lightDiffuse->setText(1, lcFormatValueLocalized(Diffuse));
		lightDiffuse->setData(0, PropertyValueRole, Diffuse);
		lightDiffuse->setToolTip(1, tr("Diffuse reflection multiplier factor."));

		lightSpecular->setText(1, lcFormatValueLocalized(Specular));
		lightSpecular->setData(0, PropertyValueRole, Specular);
		lightSpecular->setToolTip(1, tr("Specular reflection multiplier factor."));
	}

	mLightNameItem->setText(1, Name);
	mLightNameItem->setData(0, PropertyValueRole, QVariant::fromValue(Name));
}

void lcQPropertiesTree::SetMultiple()
{
	if (mWidgetMode != LC_PROPERTY_WIDGET_MULTIPLE)
	{
		SetEmpty();

		addProperty(nullptr, tr("Multiple Objects Selected"), PropertyGroup);

		mWidgetMode = LC_PROPERTY_WIDGET_MULTIPLE;
	}

	mFocus = nullptr;
}

bool lcQPropertiesTree::lastColumn(int column) const
{
	return header()->visualIndex(column) == columnCount() - 1;
}
