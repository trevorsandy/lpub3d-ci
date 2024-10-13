/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QtEvents>
#include <QGradient>

#include "gradients.h"
#include "hoverpoints.h"
#include "version.h"
#include "commonmenus.h"

extern QPixmap cached(const QString &img);

GradientDialog::GradientDialog(QSize bgSize,QGradient *bgGradient, QDialog *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Gradients"));

    setWhatsThis(lpubWT(WT_DIALOG_GRADIENT,windowTitle()));

    QGroupBox *displayGroup = new QGroupBox(this);
    displayGroup->setTitle("Display");
    m_renderer = new GradientRenderer(bgSize,displayGroup);
    displayGroup->setMinimumWidth(100);

    QGroupBox *menuGroup = new QGroupBox(this);
    menuGroup->setTitle(tr("Gradients"));

    QGroupBox *editorGroup = new QGroupBox(menuGroup);
    editorGroup->setTitle(tr("Color Editor"));
    m_editor = new GradientEditor(editorGroup);

    QGroupBox *typeGroup = new QGroupBox(menuGroup);
    typeGroup->setTitle(tr("Gradient Type"));
    m_linearButton = new QRadioButton(tr("Linear Gradient"), typeGroup);
    m_radialButton = new QRadioButton(tr("Radial Gradient"), typeGroup);
    m_conicalButton = new QRadioButton(tr("Conical Gradient"), typeGroup);

    QGroupBox *spreadGroup = new QGroupBox(menuGroup);
    spreadGroup->setTitle(tr("Spread Method"));
    m_padSpreadButton = new QRadioButton(tr("Pad Spread"), spreadGroup);
    m_reflectSpreadButton = new QRadioButton(tr("Reflect Spread"), spreadGroup);
    m_repeatSpreadButton = new QRadioButton(tr("Repeat Spread"), spreadGroup);

    QGroupBox *defaultsGroup = new QGroupBox(menuGroup);
    defaultsGroup->setTitle(tr("Defaults"));
    QPushButton *default1Button = new QPushButton(tr("1"), defaultsGroup);
    QPushButton *default2Button = new QPushButton(tr("2"), defaultsGroup);
    QPushButton *default3Button = new QPushButton(tr("3"), defaultsGroup);
    QPushButton *default4Button = new QPushButton(tr("Reset"), editorGroup);
    menuGroup->setFixedWidth(180);

    // Layouts
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(displayGroup);
    mainLayout->addWidget(menuGroup);

    QVBoxLayout *displayGroupLayout = new QVBoxLayout(displayGroup);
    displayGroupLayout->addWidget(m_renderer);

    QVBoxLayout *menuGroupLayout = new QVBoxLayout(menuGroup);
    menuGroupLayout->addWidget(editorGroup);
    menuGroupLayout->addWidget(typeGroup);
    menuGroupLayout->addWidget(spreadGroup);
    menuGroupLayout->addWidget(defaultsGroup);
    menuGroupLayout->addStretch(1);

    QVBoxLayout *editorGroupLayout = new QVBoxLayout(editorGroup);
    editorGroupLayout->addWidget(m_editor);

    QVBoxLayout *typeGroupLayout = new QVBoxLayout(typeGroup);
    typeGroupLayout->addWidget(m_linearButton);
    typeGroupLayout->addWidget(m_radialButton);
    typeGroupLayout->addWidget(m_conicalButton);

    QVBoxLayout *spreadGroupLayout = new QVBoxLayout(spreadGroup);
    spreadGroupLayout->addWidget(m_padSpreadButton);
    spreadGroupLayout->addWidget(m_repeatSpreadButton);
    spreadGroupLayout->addWidget(m_reflectSpreadButton);

    QHBoxLayout *defaultsGroupLayout = new QHBoxLayout(defaultsGroup);
    defaultsGroupLayout->addWidget(default1Button);
    defaultsGroupLayout->addWidget(default2Button);
    defaultsGroupLayout->addWidget(default3Button);
    editorGroupLayout->addWidget(default4Button);

    connect(m_editor, SIGNAL(gradientStopsChanged(QGradientStops)),
            m_renderer, SLOT(setGradientStops(QGradientStops)));

    connect(m_linearButton, SIGNAL(clicked()), m_renderer, SLOT(setLinearGradient()));
    connect(m_radialButton, SIGNAL(clicked()), m_renderer, SLOT(setRadialGradient()));
    connect(m_conicalButton, SIGNAL(clicked()), m_renderer, SLOT(setConicalGradient()));

    connect(m_padSpreadButton, SIGNAL(clicked()), m_renderer, SLOT(setPadSpread()));
    connect(m_reflectSpreadButton, SIGNAL(clicked()), m_renderer, SLOT(setReflectSpread()));
    connect(m_repeatSpreadButton, SIGNAL(clicked()), m_renderer, SLOT(setRepeatSpread()));

    connect(default1Button, SIGNAL(clicked()), this, SLOT(setDefault1()));
    connect(default2Button, SIGNAL(clicked()), this, SLOT(setDefault2()));
    connect(default3Button, SIGNAL(clicked()), this, SLOT(setDefault3()));
    connect(default4Button, SIGNAL(clicked()), this, SLOT(setDefault4()));

    if (bgGradient) {
        m_gradient = bgGradient;
        QTimer::singleShot(50, this, SLOT(setGradient()));
      } else {
        QTimer::singleShot(50, this, SLOT(setDefault1()));
      }

    QDialogButtonBox *buttonBox;

    buttonBox = new QDialogButtonBox(this);

    buttonBox->addButton(QDialogButtonBox::Ok);
    connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
    buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

    menuGroupLayout->addWidget(buttonBox);

    setModal(true);
}

void GradientDialog::setGradient(QGradient *bgGradient)
{
  QPolygonF pts;

  if (bgGradient->type() == QGradient::LinearGradient) {
      QLinearGradient *newbgGradient = (QLinearGradient*)bgGradient;
      pts << newbgGradient->start() << newbgGradient->finalStop();
      m_linearButton->animateClick();
    } else if (bgGradient->type() == QGradient::RadialGradient) {
      QRadialGradient *newbgGradient = (QRadialGradient*)bgGradient;
      pts << newbgGradient->center() << newbgGradient->focalPoint();
      m_radialButton->animateClick();
    } else {
      QConicalGradient *newbgGradient = (QConicalGradient*)bgGradient;
      QLineF l(newbgGradient->center(), QPointF(0, 0));
      l.setAngle(newbgGradient->angle());
      l.setLength(120);
      pts << newbgGradient->center() << l.p2();
      m_conicalButton->animateClick();
    }

  if (bgGradient->spread() == QGradient::PadSpread) {
      m_padSpreadButton->animateClick();
    } else if (bgGradient->spread() == QGradient::RepeatSpread) {
      m_repeatSpreadButton->animateClick();
    } else {
      m_reflectSpreadButton->animateClick();
    }

  m_editor->setGradientStops(bgGradient->stops());
  m_renderer->hoverPoints()->setPoints(pts);
  m_renderer->setGradientStops(bgGradient->stops());
}

void GradientDialog::setDefault(int config)
{
    QGradientStops stops;
    switch (config) {
    case 1:
        stops << QGradientStop(0.00, QColor::fromRgba(0));
        stops << QGradientStop(0.04, QColor::fromRgba(0xff131360));
        stops << QGradientStop(0.08, QColor::fromRgba(0xff202ccc));
        stops << QGradientStop(0.42, QColor::fromRgba(0xff93d3f9));
        stops << QGradientStop(0.51, QColor::fromRgba(0xffb3e6ff));
        stops << QGradientStop(0.73, QColor::fromRgba(0xffffffec));
        stops << QGradientStop(0.92, QColor::fromRgba(0xff5353d9));
        stops << QGradientStop(0.96, QColor::fromRgba(0xff262666));
        stops << QGradientStop(1.00, QColor::fromRgba(0));
        m_linearButton->animateClick();
        m_repeatSpreadButton->animateClick();
        break;

    case 2:
        stops << QGradientStop(0.00, QColor::fromRgba(0xffffffff));
        stops << QGradientStop(0.11, QColor::fromRgba(0xfff9ffa0));
        stops << QGradientStop(0.13, QColor::fromRgba(0xfff9ff99));
        stops << QGradientStop(0.14, QColor::fromRgba(0xfff3ff86));
        stops << QGradientStop(0.49, QColor::fromRgba(0xff93b353));
        stops << QGradientStop(0.87, QColor::fromRgba(0xff264619));
        stops << QGradientStop(0.96, QColor::fromRgba(0xff0c1306));
        stops << QGradientStop(1.00, QColor::fromRgba(0));
        m_radialButton->animateClick();
        m_padSpreadButton->animateClick();
        break;

    case 3:
        stops << QGradientStop(0.00, QColor::fromRgba(0));
        stops << QGradientStop(0.10, QColor::fromRgba(0xffe0cc73));
        stops << QGradientStop(0.17, QColor::fromRgba(0xffc6a006));
        stops << QGradientStop(0.46, QColor::fromRgba(0xff600659));
        stops << QGradientStop(0.72, QColor::fromRgba(0xff0680ac));
        stops << QGradientStop(0.92, QColor::fromRgba(0xffb9d9e6));
        stops << QGradientStop(1.00, QColor::fromRgba(0));
        m_conicalButton->animateClick();
        m_padSpreadButton->animateClick();
        break;

    case 4:
        stops << QGradientStop(0.00, QColor::fromRgba(0xff000000));
        stops << QGradientStop(1.00, QColor::fromRgba(0xffffffff));
        break;

    default:
        qWarning("bad default: %d\n", config);
        break;
    }

    QPolygonF pts;
    int h_off = m_renderer->width() / 10;
    int v_off = m_renderer->height() / 8;
    pts << QPointF(m_renderer->width() / 2, m_renderer->height() / 2)
        << QPointF(m_renderer->width() / 2 - h_off, m_renderer->height() / 2 - v_off);

    m_editor->setGradientStops(stops);
    m_renderer->hoverPoints()->setPoints(pts);
    m_renderer->setGradientStops(stops);
}

QGradient GradientDialog::getGradient()
{
  return m_renderer->getGradient();
}

void GradientDialog::accept()
{
   QDialog::accept();
//  if (gradientModified) {
//    QDialog::accept();
//  } else {
//    QDialog::reject();
//  }
}

void GradientDialog::cancel()
{
  QDialog::reject();
}

GradientRenderer::GradientRenderer(QSize bgSize, QWidget *parent)
    : QWidget(parent)
    , m_prefer_image(false)
{
#ifdef QT_OPENGL_SUPPORT
    glw = 0;
    m_use_opengl = false;
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    f.setStencil(true);
    f.setAlpha(true);
    f.setAlphaBufferSize(8);
    QGLFormat::setDefaultFormat(f);
#endif
    m_size = bgSize;
    m_tile = QPixmap(128, 128);
    m_tile.fill(Qt::white);
    QPainter pt(&m_tile);
    QColor color(230, 230, 230);
    pt.fillRect(0, 0, 64, 64, color);
    pt.fillRect(64, 64, 64, 64, color);
    pt.end();

    m_hoverPoints = new HoverPoints(this, HoverPoints::CircleShape);
    m_hoverPoints->setPointSize(QSize(20, 20));
    m_hoverPoints->setConnectionType(HoverPoints::NoConnection);
    m_hoverPoints->setEditable(false);

    QVector<QPointF> points;
    points << QPointF(100, 100) << QPointF(200, 200);
    m_hoverPoints->setPoints(points);

    m_spread = QGradient::PadSpread;
    m_gradientType = Qt::LinearGradientPattern;

    setMaximumSize(m_size);
    setMinimumSize(m_size);
}

void GradientRenderer::setGradientStops(const QGradientStops &stops)
{
    m_stops = stops;
    update();
}

QGradient GradientRenderer::getGradient()
{
   QPolygonF pts = m_hoverPoints->points();
   QGradient g;

   if (m_gradientType == Qt::LinearGradientPattern) {
       g = QLinearGradient(pts.at(0), pts.at(1));

   } else if (m_gradientType == Qt::RadialGradientPattern) {
       QLineF line(pts[0], pts[1]);
       if (line.length() > 132) {
           line.setLength(132);
         }
       g = QRadialGradient(line.p1(),  qMin(m_size.width(), m_size.height()) / 3.0, line.p2());

   } else {
       QLineF l1(pts.at(0), pts.at(1));
       QLineF l2(0, 0, 1, 0);
       // l1.angle deprecated, use qMin(l1.angleTo(l2), l2.angleTo(l1)) instead
       // qreal angle = l1.angle(QLineF(0, 0, 1, 0));
       qreal angle = qMin(l1.angleTo(l2), l2.angleTo(l1));
       if (l1.dy() > 0)
           angle = 360 - angle;
       g = QConicalGradient(pts.at(0), angle);
   }

   for (int i=0; i<m_stops.size(); ++i)
       g.setColorAt(m_stops.at(i).first, m_stops.at(i).second);

   g.setSpread(m_spread);
   return g;

}

void GradientRenderer::paint(QPainter *p)
{
    QPolygonF pts = m_hoverPoints->points();

    QGradient g;

    if (m_gradientType == Qt::LinearGradientPattern) {
        g = QLinearGradient(pts.at(0), pts.at(1));

    } else if (m_gradientType == Qt::RadialGradientPattern) {
        g = QRadialGradient(pts.at(0), qMin(width(), height()) / 3.0, pts.at(1));

    } else {
        QLineF l1(pts.at(0), pts.at(1));
        //qreal angle = l1.angle(QLineF(0, 0, 1, 0));
        QLineF l2(0, 0, 1, 0);
        qreal angle = qMin(l1.angleTo(l2), l2.angleTo(l1));
        if (l1.dy() > 0)
            angle = 360 - angle;
        g = QConicalGradient(pts.at(0), angle);
    }

    for (int i=0; i<m_stops.size(); ++i)
        g.setColorAt(m_stops.at(i).first, m_stops.at(i).second);

    g.setSpread(m_spread);

    p->setBrush(g);
    p->setPen(Qt::NoPen);

    p->drawRect(rect());

}

void GradientRenderer::paintEvent(QPaintEvent *e)
{
#ifdef Q_WS_QWS
    static QPixmap *static_image = 0;
#else
    static QImage *static_image = 0;
#endif
    QPainter painter;
    if (preferImage()
#ifdef QT_OPENGL_SUPPORT
        && !m_use_opengl
#endif
        ) {
        if (!static_image || static_image->size() != size()) {
            delete static_image;
#ifdef Q_WS_QWS
            static_image = new QPixmap(size());
#else
            static_image = new QImage(size(), QImage::Format_RGB32);
#endif
        }
        painter.begin(static_image);

        int o = 10;

        QBrush bg = palette().brush(QPalette::Window);
        painter.fillRect(0, 0, o, o, bg);
        painter.fillRect(width() - o, 0, o, o, bg);
        painter.fillRect(0, height() - o, o, o, bg);
        painter.fillRect(width() - o, height() - o, o, o, bg);
    } else {
#ifdef QT_OPENGL_SUPPORT
        if (m_use_opengl) {
            painter.begin(glw);
            painter.fillRect(QRectF(0, 0, glw->width(), glw->height()), palette().color(backgroundRole()));
        } else {
            painter.begin(this);
        }
#else
        painter.begin(this);
#endif
    }

    painter.setClipRect(e->rect());

    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath clipPath;

    QRect r = rect();
    qreal left = r.x() + 1;
    qreal top = r.y() + 1;
    qreal right = r.right();
    qreal bottom = r.bottom();
    qreal radius2 = 8 * 2;

    clipPath.moveTo(right - radius2, top);
    clipPath.arcTo(right - radius2, top, radius2, radius2, 90, -90);
    clipPath.arcTo(right - radius2, bottom - radius2, radius2, radius2, 0, -90);
    clipPath.arcTo(left, bottom - radius2, radius2, radius2, 270, -90);
    clipPath.arcTo(left, top, radius2, radius2, 180, -90);
    clipPath.closeSubpath();

    painter.save();
    painter.setClipPath(clipPath, Qt::IntersectClip);
    painter.drawTiledPixmap(rect(), m_tile);

    // client painting
    QGradient bgGradient = getGradient();
    painter.setBrush(bgGradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    painter.restore();

    int level = 180;
    painter.setPen(QPen(QColor(level, level, level), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(clipPath);

    if (preferImage()
#ifdef QT_OPENGL_SUPPORT
        && !m_use_opengl
#endif
        ) {
        painter.end();
        painter.begin(this);
#ifdef Q_WS_QWS
        painter.drawPixmap(e->rect(), *static_image, e->rect());
#else
        painter.drawImage(e->rect(), *static_image, e->rect());
#endif
    }

#ifdef QT_OPENGL_SUPPORT
    if (m_use_opengl && (inherits("PathDeformRenderer") || inherits("PathStrokeRenderer") || inherits("CompositionRenderer")))
        glw->swapBuffers();
#endif
}

#ifdef QT_OPENGL_SUPPORT
void GradientRenderer::enableOpenGL(bool use_opengl)
{
    m_use_opengl = use_opengl;

    if (!glw) {
        glw = new GLWidget(this);
        glw->setAutoFillBackground(false);
        glw->disableAutoBufferSwap();
        QApplication::postEvent(this, new QResizeEvent(size(), size()));
    }

    if (use_opengl) {
        glw->show();
    } else {
        glw->hide();
    }

    update();
}
#endif

GradientEditor::GradientEditor(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(1);
    vbox->setMargin(1);

    m_red_shade = new ShadeWidget(ShadeWidget::RedShade, this);
    m_green_shade = new ShadeWidget(ShadeWidget::GreenShade, this);
    m_blue_shade = new ShadeWidget(ShadeWidget::BlueShade, this);
    m_alpha_shade = new ShadeWidget(ShadeWidget::ARGBShade, this);

    vbox->addWidget(m_red_shade);
    vbox->addWidget(m_green_shade);
    vbox->addWidget(m_blue_shade);
    vbox->addWidget(m_alpha_shade);

    connect(m_red_shade, SIGNAL(colorsChanged()), this, SLOT(pointsUpdated()));
    connect(m_green_shade, SIGNAL(colorsChanged()), this, SLOT(pointsUpdated()));
    connect(m_blue_shade, SIGNAL(colorsChanged()), this, SLOT(pointsUpdated()));
    connect(m_alpha_shade, SIGNAL(colorsChanged()), this, SLOT(pointsUpdated()));
}

inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}

void GradientEditor::pointsUpdated()
{
    qreal w = m_alpha_shade->width();

    QGradientStops stops;

    QPolygonF points;

    points += m_red_shade->points();
    points += m_green_shade->points();
    points += m_blue_shade->points();
    points += m_alpha_shade->points();

    lpub_sort(points.begin(), points.end(), x_less_than);

    for (int i=0; i<points.size(); ++i) {
        qreal x = int(points.at(i).x());
        if (i+1 < points.size() && x == points.at(i+1).x())
            continue;
        QColor color((0x00ff0000 & m_red_shade->colorAt(int(x))) >> 16,
                     (0x0000ff00 & m_green_shade->colorAt(int(x))) >> 8,
                     (0x000000ff & m_blue_shade->colorAt(int(x))),
                     (0xff000000 & m_alpha_shade->colorAt(int(x))) >> 24);

        if (x / w > 1)
            return;

        stops << QGradientStop(x / w, color);
    }

    m_alpha_shade->setGradientStops(stops);

    emit gradientStopsChanged(stops);
}


static void set_shade_points(const QPolygonF &points, ShadeWidget *shade)
{
    shade->hoverPoints()->setPoints(points);
    shade->hoverPoints()->setPointLock(0, HoverPoints::LockToLeft);
    shade->hoverPoints()->setPointLock(points.size() - 1, HoverPoints::LockToRight);
    shade->update();
}

void GradientEditor::setGradientStops(const QGradientStops &stops)
{
    QPolygonF pts_red, pts_green, pts_blue, pts_alpha;

    qreal h_red = m_red_shade->height();
    qreal h_green = m_green_shade->height();
    qreal h_blue = m_blue_shade->height();
    qreal h_alpha = m_alpha_shade->height();

    for (int i=0; i<stops.size(); ++i) {
        qreal pos = stops.at(i).first;
        QRgb color = stops.at(i).second.rgba();
        pts_red << QPointF(pos * m_red_shade->width(), h_red - qRed(color) * h_red / 255);
        pts_green << QPointF(pos * m_green_shade->width(), h_green - qGreen(color) * h_green / 255);
        pts_blue << QPointF(pos * m_blue_shade->width(), h_blue - qBlue(color) * h_blue / 255);
        pts_alpha << QPointF(pos * m_alpha_shade->width(), h_alpha - qAlpha(color) * h_alpha / 255);
    }

    set_shade_points(pts_red, m_red_shade);
    set_shade_points(pts_green, m_green_shade);
    set_shade_points(pts_blue, m_blue_shade);
    set_shade_points(pts_alpha, m_alpha_shade);

}

ShadeWidget::ShadeWidget(ShadeType type, QWidget *parent)
    : QWidget(parent), m_shade_type(type), m_alpha_gradient(QLinearGradient(0, 0, 0, 0))
{

    // Checkers background
    if (m_shade_type == ARGBShade) {
        QPixmap pm(20, 20);
        QPainter pmp(&pm);
        pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
        pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
        pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
        pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
        pmp.end();
        QPalette pal = palette();
        pal.setBrush(backgroundRole(), QBrush(pm));
        setAutoFillBackground(true);
        setPalette(pal);

    } else {
        setAttribute(Qt::WA_NoBackground);

    }

    QPolygonF points;
    points << QPointF(0, sizeHint().height())
           << QPointF(sizeHint().width(), 0);

    m_hoverPoints = new HoverPoints(this, HoverPoints::CircleShape);
    m_hoverPoints->setPoints(points);
    m_hoverPoints->setPointLock(0, HoverPoints::LockToLeft);
    m_hoverPoints->setPointLock(1, HoverPoints::LockToRight);
    m_hoverPoints->setSortType(HoverPoints::XSort);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(m_hoverPoints, SIGNAL(pointsChanged(QPolygonF)), this, SIGNAL(colorsChanged()));
}


QPolygonF ShadeWidget::points() const
{
    return m_hoverPoints->points();
}


uint ShadeWidget::colorAt(int x)
{
    generateShade();

    QPolygonF pts = m_hoverPoints->points();
    for (int i=1; i < pts.size(); ++i) {
        if (pts.at(i-1).x() <= x && pts.at(i).x() >= x) {
            QLineF l(pts.at(i-1), pts.at(i));
            l.setLength(l.length() * ((x - l.x1()) / l.dx()));
            return m_shade.pixel(qRound(qMin(l.x2(), (qreal(m_shade.width() - 1)))),
                                 qRound(qMin(l.y2(), qreal(m_shade.height() - 1))));
        }
    }
    return 0;
}


void ShadeWidget::setGradientStops(const QGradientStops &stops)
{
    if (m_shade_type == ARGBShade) {
        m_alpha_gradient = QLinearGradient(0, 0, width(), 0);

        for (int i=0; i<stops.size(); ++i) {
            QColor c = stops.at(i).second;
            m_alpha_gradient.setColorAt(stops.at(i).first, QColor(c.red(), c.green(), c.blue()));
        }

        m_shade = QImage();
        generateShade();
        update();
    }
}


void ShadeWidget::paintEvent(QPaintEvent *)
{
    generateShade();

    QPainter p(this);
    p.drawImage(0, 0, m_shade);

    p.setPen(QColor(146, 146, 146));
    p.drawRect(0, 0, width() - 1, height() - 1);
}


void ShadeWidget::generateShade()
{
    if (m_shade.isNull() || m_shade.size() != size()) {

        if (m_shade_type == ARGBShade) {
            m_shade = QImage(size(), QImage::Format_ARGB32_Premultiplied);
            m_shade.fill(0);

            QPainter p(&m_shade);
            p.fillRect(rect(), m_alpha_gradient);

            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QLinearGradient fade(0, 0, 0, height());
            fade.setColorAt(0, QColor(0, 0, 0, 255));
            fade.setColorAt(1, QColor(0, 0, 0, 0));
            p.fillRect(rect(), fade);

        } else {
            m_shade = QImage(size(), QImage::Format_RGB32);
            QLinearGradient shade(0, 0, 0, height());
            shade.setColorAt(1, Qt::black);

            if (m_shade_type == RedShade)
                shade.setColorAt(0, Qt::red);
            else if (m_shade_type == GreenShade)
                shade.setColorAt(0, Qt::green);
            else
                shade.setColorAt(0, Qt::blue);

            QPainter p(&m_shade);
            p.fillRect(rect(), shade);
        }
    }


}
