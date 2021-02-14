#include "lc_global.h"
#include "lc_qaboutdialog.h"
#include "ui_lc_qaboutdialog.h"
#include "lc_mainwindow.h"
#include "lc_view.h"
#include "lc_glextensions.h"
#include "lc_viewwidget.h"

lcQAboutDialog::lcQAboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::lcQAboutDialog)
{
	ui->setupUi(this);

/*** LPub3D Mod - vesion build ***/
#ifdef LC_CONTINUOUS_BUILD
	ui->version->setText(tr("3DViewer - by LeoCAD Continuous Build %1").arg(QString::fromLatin1(QT_STRINGIFY(LC_CONTINUOUS_BUILD))));
#else
/*** LPub3D Mod - vesion build ***/
	ui->version->setText(tr("3DViewer - by LeoCAD Version %1 - Rev %2").arg(QString::fromLatin1(LC_VERSION_TEXT))
																	   .arg(QString::fromLatin1(LC_VERSION_BUILD)));
/*** LPub3D Mod end ***/
#endif

	lcViewWidget* Widget = gMainWindow->GetActiveView()->GetWidget();
#ifdef LC_USE_QOPENGLWIDGET
	QSurfaceFormat Format = Widget->context()->format();
#else
	QGLFormat Format = Widget->context()->format();
#endif

	int ColorDepth = Format.redBufferSize() + Format.greenBufferSize() + Format.blueBufferSize() + Format.alphaBufferSize();

	const QString QtVersionFormat = tr("Qt Version %1 (compiled with %2)\n\n");
	const QString QtVersion = QtVersionFormat.arg(qVersion(), QT_VERSION_STR);
	const QString VersionFormat = tr("OpenGL Version %1 (GLSL %2)\n%3 - %4\n\n");
	const QString Version = VersionFormat.arg(QString((const char*)glGetString(GL_VERSION)), QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)), QString((const char*)glGetString(GL_RENDERER)), QString((const char*)glGetString(GL_VENDOR)));
	const QString BuffersFormat = tr("Color Buffer: %1 bits\nDepth Buffer: %2 bits\nStencil Buffer: %3 bits\n\n");
	const QString Buffers = BuffersFormat.arg(QString::number(ColorDepth), QString::number(Format.depthBufferSize()), QString::number(Format.stencilBufferSize()));

	const QString ExtensionsFormat = tr("GL_ARB_vertex_buffer_object extension: %1\nGL_ARB_framebuffer_object extension: %2\nGL_EXT_framebuffer_object extension: %3\nGL_EXT_blend_func_separate: %4\nGL_EXT_texture_filter_anisotropic extension: %5\n");
	const QString VertexBufferObject = gSupportsVertexBufferObject ? tr("Supported") : tr("Not supported");
#ifdef LC_USE_QOPENGLWIDGET
	const QString FramebufferObjectARB = gSupportsFramebufferObject ? tr("Supported") : tr("Not supported");
	const QString FramebufferObjectEXT = tr("Not supported");
#else
	const QString FramebufferObjectARB = gSupportsFramebufferObjectARB ? tr("Supported") : tr("Not supported");
	const QString FramebufferObjectEXT = gSupportsFramebufferObjectEXT ? tr("Supported") : tr("Not supported");
#endif
	const QString BlendFuncSeparateEXT = gSupportsBlendFuncSeparate ? tr("Supported") : tr("Not supported");
	const QString Anisotropic = gSupportsAnisotropic ? tr("Supported (max %1)").arg(gMaxAnisotropy) : tr("Not supported");
	const QString Extensions = ExtensionsFormat.arg(VertexBufferObject, FramebufferObjectARB, FramebufferObjectEXT, BlendFuncSeparateEXT, Anisotropic);

	ui->info->setText(QtVersion + Version + Buffers + Extensions);
}

lcQAboutDialog::~lcQAboutDialog()
{
	delete ui;
}
