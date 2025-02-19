#pragma once

class lcViewWidget : public QOpenGLWidget
{
	Q_OBJECT

public:
	lcViewWidget(QWidget* Parent, lcView* View);
	~lcViewWidget();

	lcView* GetView() const;
	void SetView(lcView* View);

	void UpdateMousePosition();

	QSize sizeHint() const override;

/*** LPub3D Mod - preview widget for LPub3D ***/
	void SetPreviewPosition(const QRect& ParentRect, const QPoint &ViewPos = QPoint());
	void SetPreferredSize(const QSize &PreferredSize)
	{
		mPreferredSize = PreferredSize;
	}
/*** LPub3D Mod end ***/

protected:
	float GetDeviceScale() const
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
		return devicePixelRatioF();
#else
		return devicePixelRatio();
#endif
	}

	void initializeGL() override;
	void resizeGL(int Width, int Height) override;
	void paintGL() override;
	void focusInEvent(QFocusEvent* FocusEvent) override;
	void focusOutEvent(QFocusEvent* FocusEvent) override;
	void keyPressEvent(QKeyEvent* KeyEvent) override;
	void keyReleaseEvent(QKeyEvent* KeyEvent) override;
	void mousePressEvent(QMouseEvent* MouseEvent) override;
	void mouseReleaseEvent(QMouseEvent* MouseEvent) override;
	void mouseDoubleClickEvent(QMouseEvent* MouseEvent) override;
	void mouseMoveEvent(QMouseEvent* MouseEvent) override;
	void wheelEvent(QWheelEvent* WheelEvent) override;
	void dragEnterEvent(QDragEnterEvent* DragEnterEvent) override;
	void dragLeaveEvent(QDragLeaveEvent* DragLeaveEvent) override;
	void dragMoveEvent(QDragMoveEvent* DragMoveEvent) override;
	void dropEvent(QDropEvent* DropEvent) override;

	std::unique_ptr<lcView> mView;
	QSize mPreferredSize;
	int mWheelAccumulator = 0;
};
