#pragma once

#include <QGLWidget>

class lcViewWidget : public QGLWidget
{
public:
	lcViewWidget(QWidget* Parent, lcGLWidget* View);
	~lcViewWidget();

	lcGLWidget* GetView() const
	{
		return mView;
	}

	void SetView(lcGLWidget* View);
/*** LPub3D Mod - preview widget for LPub3D ***/
	void SetPreviewPosition(const QRect& ParentRect, const QPoint &ViewPos = QPoint());
	void SetPreferredSize(const QSize &PreferredSize)
	{
		mPreferredSize = PreferredSize;
	}
/*** LPub3D Mod end ***/

	QSize sizeHint() const override;

protected:
	float GetDeviceScale() const
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
		return windowHandle()->devicePixelRatio();
#else
		return 1.0f;
#endif
	}

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

	lcGLWidget* mView;
	QSize mPreferredSize;
	int mWheelAccumulator;
};
