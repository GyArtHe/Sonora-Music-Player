#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QSettings>
#include <QShortcut>
#include <QStackedWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QInputDialog>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QBuffer>
#include <QImage>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QSizePolicy>
#include <QRandomGenerator>
#include <QUrl>
#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPolygonF>
#include <QPainterPath>
#include <QTransform>
#include <QAbstractSlider>
#include <QDebug>
#include <QMediaMetaData>
#include <algorithm>
#include <cmath>
#include <functional>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <mmdeviceapi.h>
#  include <endpointvolume.h>
#endif

namespace {

constexpr qreal kPi = 3.14159265358979323846;

class CoverWidget : public QWidget
{
public:
    using QWidget::QWidget;
    QPixmap cover;
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        QPainterPath path;
        path.addRoundedRect(rect(), 22, 22);
        p.setClipPath(path);

        if (!cover.isNull()) {
            QSize targetSize = size();
            QSize scaledSize = cover.size();
            scaledSize.scale(targetSize, Qt::KeepAspectRatioByExpanding);

            QRect targetRect(QPoint(0, 0), scaledSize);
            targetRect.moveCenter(rect().center());

            p.drawPixmap(targetRect, cover);
        } else {
            QLinearGradient grad(0, 0, width(), height());
            grad.setColorAt(0, QColor("#3e3e42"));
            grad.setColorAt(1, QColor("#252526"));
            p.fillPath(path, grad);

            p.setPen(QColor(255, 255, 255, 235));
            QFont f = font();
            f.setPixelSize(54);
            p.setFont(f);
            p.drawText(rect(), Qt::AlignCenter, "\u266B");
        }
    }
};

QIcon makePlayIcon(const QColor &color, int size = 22)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    QPainterPath path;
    path.moveTo(s * 0.30, s * 0.18);
    path.lineTo(s * 0.78, s * 0.50);
    path.lineTo(s * 0.30, s * 0.82);
    path.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawPath(path);
    return QIcon(pm);
}

QIcon makePauseIcon(const QColor &color, int size = 22)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    const qreal barW = s * 0.16;
    const qreal radius = barW * 0.45;
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(QRectF(s * 0.28, s * 0.20, barW, s * 0.60), radius, radius);
    p.drawRoundedRect(QRectF(s * 0.55, s * 0.20, barW, s * 0.60), radius, radius);
    return QIcon(pm);
}

QIcon makeNextIcon(const QColor &color, int size = 20)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    const qreal s = size;
    for (int i = 0; i < 2; ++i) {
        const qreal x = s * (0.22 + i * 0.30);
        QPainterPath tri;
        tri.moveTo(x, s * 0.24);
        tri.lineTo(x + s * 0.26, s * 0.50);
        tri.lineTo(x, s * 0.76);
        tri.closeSubpath();
        p.drawPath(tri);
    }
    return QIcon(pm);
}

QIcon makePrevIcon(const QColor &color, int size = 20)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    const qreal s = size;
    for (int i = 0; i < 2; ++i) {
        const qreal x = s * (0.72 - i * 0.30);
        QPainterPath tri;
        tri.moveTo(x, s * 0.24);
        tri.lineTo(x - s * 0.26, s * 0.50);
        tri.lineTo(x, s * 0.76);
        tri.closeSubpath();
        p.drawPath(tri);
    }
    return QIcon(pm);
}

QIcon makeMinimizeIcon(const QColor &color, int size = 14)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.4));
    p.drawLine(QPointF(size * 0.20, size * 0.62), QPointF(size * 0.80, size * 0.62));
    return QIcon(pm);
}

QIcon makeCloseIcon(const QColor &color, int size = 14)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.4, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(size * 0.24, size * 0.24), QPointF(size * 0.76, size * 0.76));
    p.drawLine(QPointF(size * 0.76, size * 0.24), QPointF(size * 0.24, size * 0.76));
    return QIcon(pm);
}

QIcon makeGearIcon(const QColor &color, int size = 18)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(size / 2.0, size / 2.0);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    const int teeth = 8;
    const qreal outer = size * 0.42;
    const qreal inner = size * 0.32;
    QPainterPath gear;
    for (int i = 0; i < teeth * 2; ++i) {
        const qreal r = (i % 2 == 0) ? outer : inner;
        const qreal a = kPi * i / teeth - kPi / 2.0;
        const QPointF pt(r * std::cos(a), r * std::sin(a));
        if (i == 0) gear.moveTo(pt);
        else        gear.lineTo(pt);
    }
    gear.closeSubpath();
    p.drawPath(gear);
    p.setCompositionMode(QPainter::CompositionMode_Clear);
    p.drawEllipse(QPointF(0, 0), size * 0.13, size * 0.13);
    return QIcon(pm);
}

QIcon makeHeartIcon(bool filled, const QColor &color, int size = 20)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    QPainterPath heart;
    heart.moveTo(s * 0.50, s * 0.84);
    heart.cubicTo(s * 0.06, s * 0.56, s * 0.10, s * 0.16, s * 0.50, s * 0.34);
    heart.cubicTo(s * 0.90, s * 0.16, s * 0.94, s * 0.56, s * 0.50, s * 0.84);
    heart.closeSubpath();
    if (filled) {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
    } else {
        p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
    }
    p.drawPath(heart);
    return QIcon(pm);
}

QIcon makeCircleIcon(bool filled, const QColor &color, int size = 16)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    if (filled) {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
    } else {
        p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
    }
    p.drawEllipse(QRectF(size*0.15, size*0.15, size*0.7, size*0.7));
    return QIcon(pm);
}

QIcon makePlusIcon(const QColor &color, int size = 16)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(size * 0.50, size * 0.22), QPointF(size * 0.50, size * 0.78));
    p.drawLine(QPointF(size * 0.22, size * 0.50), QPointF(size * 0.78, size * 0.50));
    return QIcon(pm);
}

QIcon makeSmallCloseIcon(const QColor &color, int size = 12)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(size * 0.25, size * 0.25), QPointF(size * 0.75, size * 0.75));
    p.drawLine(QPointF(size * 0.75, size * 0.25), QPointF(size * 0.25, size * 0.75));
    return QIcon(pm);
}

QIcon makeSpeakerIcon(const QColor &color, int size = 18)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    const qreal s = size;
    QPainterPath body;
    body.moveTo(s * 0.16, s * 0.38);
    body.lineTo(s * 0.34, s * 0.38);
    body.lineTo(s * 0.54, s * 0.20);
    body.lineTo(s * 0.54, s * 0.80);
    body.lineTo(s * 0.34, s * 0.62);
    body.lineTo(s * 0.16, s * 0.62);
    body.closeSubpath();
    p.drawPath(body);
    p.setPen(QPen(color, 1.4, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(s * 0.50, s * 0.32, s * 0.28, s * 0.36), -60 * 16, 120 * 16);
    p.drawArc(QRectF(s * 0.56, s * 0.20, s * 0.34, s * 0.60), -60 * 16, 120 * 16);
    return QIcon(pm);
}

QIcon makeTrashIcon(const QColor &color, int size = 18)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    p.setPen(QPen(color, s * 0.085, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    p.drawLine(QPointF(s * 0.17, s * 0.27), QPointF(s * 0.83, s * 0.27));
    p.drawLine(QPointF(s * 0.40, s * 0.27), QPointF(s * 0.42, s * 0.17));
    p.drawLine(QPointF(s * 0.42, s * 0.17), QPointF(s * 0.58, s * 0.17));
    p.drawLine(QPointF(s * 0.58, s * 0.17), QPointF(s * 0.60, s * 0.27));

    QPainterPath body;
    body.moveTo(s * 0.26, s * 0.36);
    body.lineTo(s * 0.31, s * 0.84);
    body.lineTo(s * 0.69, s * 0.84);
    body.lineTo(s * 0.74, s * 0.36);
    p.drawPath(body);

    p.drawLine(QPointF(s * 0.43, s * 0.46), QPointF(s * 0.45, s * 0.74));
    p.drawLine(QPointF(s * 0.57, s * 0.46), QPointF(s * 0.55, s * 0.74));

    return QIcon(pm);
}

QIcon makePencilIcon(const QColor &color, int size = 18)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;

    QPainterPath body;
    body.moveTo(s * 0.24, s * 0.76);
    body.lineTo(s * 0.30, s * 0.58);
    body.lineTo(s * 0.66, s * 0.22);
    body.lineTo(s * 0.78, s * 0.34);
    body.lineTo(s * 0.42, s * 0.70);
    body.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawPath(body);

    QPainterPath tip;
    tip.moveTo(s * 0.24, s * 0.76);
    tip.lineTo(s * 0.30, s * 0.58);
    tip.lineTo(s * 0.42, s * 0.70);
    tip.closeSubpath();
    p.setBrush(color.darker(160));
    p.drawPath(tip);

    return QIcon(pm);
}

const QColor kAccent   = QColor("#f1f1f1");
const QColor kAccentHi = QColor("#cccccc");
const QColor kIconGray = QColor("#a0a0a0");
const QColor kWhite    = QColor("#f1f1f1");
const QColor kRed      = QColor("#e81123");

void drawArcArrowHead(QPainter &p, const QPointF &center, qreal radius,
                      qreal angleDeg, qreal arrowSize, const QColor &color)
{
    const qreal rad = angleDeg * kPi / 180.0;
    const QPointF pos(center.x() + radius * std::cos(rad),
                      center.y() - radius * std::sin(rad));
    const QPointF tangent(-std::sin(rad), -std::cos(rad));
    const qreal ang = std::atan2(tangent.y(), tangent.x());
    QPolygonF head;
    head << QPointF(arrowSize, 0.0)
         << QPointF(-arrowSize * 0.55, arrowSize * 0.72)
         << QPointF(-arrowSize * 0.55, -arrowSize * 0.72);
    QTransform t;
    t.translate(pos.x(), pos.y());
    t.rotateRadians(ang);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawPolygon(t.map(head));
}

QIcon makeRepeatIcon(const QColor &color, int size = 20)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    const QPointF c(s * 0.5, s * 0.5);
    const qreal r = s * 0.32;
    const qreal thickness = s * 0.09;
    p.setPen(QPen(color, thickness, Qt::SolidLine, Qt::FlatCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(c.x() - r, c.y() - r, 2 * r, 2 * r), 30 * 16, 120 * 16);
    p.drawArc(QRectF(c.x() - r, c.y() - r, 2 * r, 2 * r), 210 * 16, 120 * 16);
    drawArcArrowHead(p, c, r, 150.0, s * 0.14, color);
    drawArcArrowHead(p, c, r, 330.0, s * 0.14, color);
    return QIcon(pm);
}

QIcon makeRepeatOneIcon(const QColor &color, int size = 20)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    const QPointF c(s * 0.5, s * 0.5);
    const qreal r = s * 0.32;
    const qreal thickness = s * 0.09;
    p.setPen(QPen(color, thickness, Qt::SolidLine, Qt::FlatCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(c.x() - r, c.y() - r, 2 * r, 2 * r), 30 * 16, 120 * 16);
    p.drawArc(QRectF(c.x() - r, c.y() - r, 2 * r, 2 * r), 210 * 16, 120 * 16);
    drawArcArrowHead(p, c, r, 150.0, s * 0.14, color);
    drawArcArrowHead(p, c, r, 330.0, s * 0.14, color);
    QFont f = p.font();
    f.setPixelSize(qMax(8, int(s * 0.46)));
    f.setBold(true);
    p.setFont(f);
    p.setPen(color);
    p.drawText(QRectF(0, 0, s, s * 1.02), Qt::AlignCenter, "1");
    return QIcon(pm);
}

QIcon makeShuffleIcon(const QColor &color, int size = 20)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    p.setPen(QPen(color, s * 0.090, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawLine(QPointF(s * 0.16, s * 0.76), QPointF(s * 0.60, s * 0.26));
    p.drawLine(QPointF(s * 0.16, s * 0.26), QPointF(s * 0.60, s * 0.76));
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    auto drawHead = [&](qreal tx, qreal ty, qreal dx, qreal dy) {
        const qreal ang = std::atan2(dy, dx);
        const qreal a = s * 0.145;
        QPolygonF poly;
        poly << QPointF(a, 0.0)
             << QPointF(-a * 0.5, a * 0.78)
             << QPointF(-a * 0.5, -a * 0.78);
        QTransform t;
        t.translate(tx, ty);
        t.rotateRadians(ang);
        p.drawPolygon(t.map(poly));
    };
    drawHead(s * 0.655, s * 0.195, 0.707, -0.707);
    drawHead(s * 0.645, s * 0.795, 0.700,  0.714);
    return QIcon(pm);
}

class ClickableSlider : public QSlider
{
public:
    explicit ClickableSlider(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSlider(orientation, parent) {}

protected:
    void mousePressEvent(QMouseEvent *ev) override
    {
        if (ev->button() != Qt::LeftButton
            || orientation() != Qt::Horizontal
            || maximum() <= minimum()) {
            QSlider::mousePressEvent(ev);
            return;
        }

        setSliderDown(true);
        setValue(valueForX(ev->position().x()));
        emit sliderMoved(value());
        ev->accept();
    }

    void mouseMoveEvent(QMouseEvent *ev) override
    {
        if (!isSliderDown()) {
            QSlider::mouseMoveEvent(ev);
            return;
        }

        setValue(valueForX(ev->position().x()));
        emit sliderMoved(value());
        ev->accept();
    }

    void mouseReleaseEvent(QMouseEvent *ev) override
    {
        if (isSliderDown() && ev->button() == Qt::LeftButton) {
            setValue(valueForX(ev->position().x()));
            setSliderDown(false);
            emit sliderMoved(value());
            ev->accept();
            return;
        }
        QSlider::mouseReleaseEvent(ev);
    }

private:
    int valueForX(qreal x) const
    {
        const qreal sliderWidth = qMax(1, width());
        const qreal ratio = qBound<qreal>(0.0, x / sliderWidth, 1.0);
        return minimum() + int(std::lround(ratio * (maximum() - minimum())));
    }
};

class EqSlider : public QSlider
{
public:
    explicit EqSlider(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSlider(orientation, parent) {}

protected:
    void paintEvent(QPaintEvent *) override
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        const QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt,
                                                     QStyle::SC_SliderGroove, this);
        const QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt,
                                                     QStyle::SC_SliderHandle, this);
        if (groove.isEmpty())
            return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);

        p.setBrush(QColor("#3e3e42"));
        p.drawRoundedRect(groove, 3, 3);

        p.setBrush(QColor("#f1f1f1"));
        p.drawEllipse(handle.center(), 6, 6);
    }
};

class RoundScrollBar : public QScrollBar
{
public:
    explicit RoundScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QScrollBar(orientation, parent)
    {
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        const QRect slider = style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                                     QStyle::SC_ScrollBarSlider, this);
        if (slider.isEmpty())
            return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#4d4d4d"));

        const qreal thickness = 6.0;
        QRectF handle;
        if (orientation() == Qt::Horizontal)
            handle = QRectF(slider.left(), (height() - thickness) / 2.0,
                            slider.width(), thickness);
        else
            handle = QRectF((width() - thickness) / 2.0, slider.top(),
                            thickness, slider.height());

        p.drawRoundedRect(handle, thickness / 2.0, thickness / 2.0);
    }
};

#ifdef Q_OS_WIN
// Громкость устройства через Windows Core Audio (IAudioEndpointVolume).
static const CLSID kCLSIDMMDeviceEnumerator =
    {0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
static const IID kIIDIMMDeviceEnumerator =
    {0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
static const IID kIIDIAudioEndpointVolume =
    {0x5CDF2C82, 0x841E, 0x4546, {0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A}};
static const IID kIIDIUnknown =
    {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const IID kIIDIAudioEndpointVolumeCallback =
    {0x657804FA, 0xD6AD, 0x4496, {0x8A, 0x60, 0x35, 0x27, 0x8F, 0x8A, 0x4B, 0x2A}};

class VolumeCallback : public IAudioEndpointVolumeCallback
{
public:
    explicit VolumeCallback(std::function<void(int)> handler)
        : m_handler(std::move(handler)) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (!ppv)
            return E_POINTER;
        if (riid == kIIDIUnknown || riid == kIIDIAudioEndpointVolumeCallback) {
            *ppv = static_cast<IAudioEndpointVolumeCallback *>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ULONG(InterlockedIncrement(&m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        const LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0)
            delete this;
        return ULONG(count);
    }

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA data) override
    {
        if (data && m_handler) {
            const int percent = qBound(0, int(qRound(data->fMasterVolume * 100.0f)), 100);
            m_handler(percent);
        }
        return S_OK;
    }

private:
    LONG m_refCount = 1;
    std::function<void(int)> m_handler;
};

class SystemVolume
{
public:
    SystemVolume()
    {
        const HRESULT initResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        m_comInitialized = SUCCEEDED(initResult);

        IMMDeviceEnumerator *enumerator = nullptr;
        HRESULT hr = CoCreateInstance(kCLSIDMMDeviceEnumerator, nullptr, CLSCTX_ALL,
                                      kIIDIMMDeviceEnumerator,
                                      reinterpret_cast<void **>(&enumerator));
        if (FAILED(hr) || !enumerator)
            return;

        IMMDevice *device = nullptr;
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        enumerator->Release();
        if (FAILED(hr) || !device)
            return;

        hr = device->Activate(kIIDIAudioEndpointVolume, CLSCTX_ALL, nullptr,
                              reinterpret_cast<void **>(&m_endpoint));
        device->Release();
    }

    ~SystemVolume()
    {
        if (m_endpoint && m_callback) {
            m_endpoint->UnregisterControlChangeNotify(m_callback);
            m_callback->Release();
            m_callback = nullptr;
        }
        if (m_endpoint)
            m_endpoint->Release();
        if (m_comInitialized)
            CoUninitialize();
    }

    void setChangeCallback(std::function<void(int)> handler)
    {
        if (!m_endpoint)
            return;

        if (m_callback) {
            m_endpoint->UnregisterControlChangeNotify(m_callback);
            m_callback->Release();
            m_callback = nullptr;
        }
        if (!handler)
            return;

        m_callback = new VolumeCallback(std::move(handler));
        if (FAILED(m_endpoint->RegisterControlChangeNotify(m_callback))) {
            m_callback->Release();
            m_callback = nullptr;
        }
    }

    bool isValid() const { return m_endpoint != nullptr; }

    int volume() const
    {
        if (!m_endpoint)
            return -1;
        float level = 0.0f;
        if (FAILED(m_endpoint->GetMasterVolumeLevelScalar(&level)))
            return -1;
        return qBound(0, int(qRound(level * 100.0f)), 100);
    }

    void setVolume(int percent)
    {
        if (!m_endpoint)
            return;
        m_endpoint->SetMasterVolumeLevelScalar(qBound(0, percent, 100) / 100.0f, nullptr);
    }

private:
    IAudioEndpointVolume *m_endpoint = nullptr;
    IAudioEndpointVolumeCallback *m_callback = nullptr;
    bool m_comInitialized = false;
};

static SystemVolume &systemVolume()
{
    static SystemVolume *instance = new SystemVolume();
    return *instance;
}
#endif


QByteArray id3SyncSafe(int value)
{
    QByteArray bytes(4, 0);
    bytes[0] = char((value >> 21) & 0x7F);
    bytes[1] = char((value >> 14) & 0x7F);
    bytes[2] = char((value >> 7) & 0x7F);
    bytes[3] = char(value & 0x7F);
    return bytes;
}

int id3SyncSafeValue(const QByteArray &bytes)
{
    if (bytes.size() < 4)
        return -1;
    return ((bytes[0] & 0x7F) << 21) | ((bytes[1] & 0x7F) << 14) |
           ((bytes[2] & 0x7F) << 7) | (bytes[3] & 0x7F);
}

QByteArray id3TextFrame(const char *id, const QString &text)
{
    QByteArray payload;
    payload.append(char(0x03));
    payload.append(text.toUtf8());

    QByteArray frame(id, 4);
    frame.append(id3SyncSafe(payload.size()));
    frame.append(char(0));
    frame.append(char(0));
    frame.append(payload);
    return frame;
}

QByteArray id3ApicFrame(const QByteArray &image)
{
    if (image.isEmpty())
        return QByteArray();

    QByteArray payload;
    payload.append(char(0x00));
    payload.append("image/jpeg");
    payload.append(char(0));
    payload.append(char(0x03));
    payload.append(char(0));
    payload.append(image);

    QByteArray frame("APIC", 4);
    frame.append(id3SyncSafe(payload.size()));
    frame.append(char(0));
    frame.append(char(0));
    frame.append(payload);
    return frame;
}

bool writeMp3Tags(const QString &path, const QString &title,
                  const QString &artist, const QByteArray &coverJpeg)
{
    QFile input(path);
    if (!input.open(QIODevice::ReadOnly))
        return false;
    const QByteArray data = input.readAll();
    input.close();

    qint64 audioOffset = 0;
    qint64 tagEnd = 0;
    int version = 0;

    if (data.size() >= 10 && data.startsWith("ID3")) {
        const uchar *header = reinterpret_cast<const uchar *>(data.constData());
        version = header[3];
        const int size = id3SyncSafeValue(data.mid(6, 4));
        if (size < 0)
            return false;
        tagEnd = 10 + size;
        audioOffset = tagEnd + ((header[5] & 0x10) ? 10 : 0);
        if (audioOffset > data.size())
            return false;
    }

    QByteArray kept;
    if (version == 3 || version == 4) {
        qint64 pos = 10;
        while (pos + 10 <= tagEnd) {
            const QByteArray frameId = data.mid(int(pos), 4);
            if (frameId.isEmpty() || frameId.at(0) == '\0')
                break;

            int frameSize = 0;
            if (version == 4) {
                frameSize = id3SyncSafeValue(data.mid(int(pos) + 4, 4));
            } else {
                const uchar *s = reinterpret_cast<const uchar *>(data.constData() + pos + 4);
                frameSize = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
            }
            if (frameSize < 0 || pos + 10 + frameSize > tagEnd)
                break;

            if (frameId != "TIT2" && frameId != "TPE1" && frameId != "APIC") {
                kept.append(frameId);
                kept.append(id3SyncSafe(frameSize));
                kept.append(data.mid(int(pos) + 8, 2));
                kept.append(data.mid(int(pos) + 10, frameSize));
            }
            pos += 10 + frameSize;
        }
    }

    QByteArray frames = kept;
    if (!title.isEmpty())
        frames.append(id3TextFrame("TIT2", title));
    if (!artist.isEmpty())
        frames.append(id3TextFrame("TPE1", artist));
    frames.append(id3ApicFrame(coverJpeg));

    QByteArray tag("ID3", 3);
    tag.append(char(4));
    tag.append(char(0));
    tag.append(char(0));
    tag.append(id3SyncSafe(frames.size()));
    tag.append(frames);

    const QByteArray result = tag + data.mid(int(audioOffset));

    const QString backupPath = path + ".bak";
    QFile::remove(backupPath);
    if (!QFile::copy(path, backupPath))
        return false;

    const QString tempPath = path + ".tmp";
    QFile output(tempPath);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const bool written = (output.write(result) == result.size());
    output.close();
    if (!written) {
        QFile::remove(tempPath);
        return false;
    }

    if (!QFile::remove(path)) {
        QFile::remove(tempPath);
        return false;
    }
    if (!QFile::rename(tempPath, path)) {
        QFile::rename(backupPath, path);
        return false;
    }
    return true;
}

quint32 mp4U32(const QByteArray &data, int offset)
{
    const uchar *p = reinterpret_cast<const uchar *>(data.constData()) + offset;
    return (quint32(p[0]) << 24) | (quint32(p[1]) << 16) |
           (quint32(p[2]) << 8) | quint32(p[3]);
}

void mp4WriteU32(QByteArray &data, int offset, quint32 value)
{
    data[offset]     = char((value >> 24) & 0xFF);
    data[offset + 1] = char((value >> 16) & 0xFF);
    data[offset + 2] = char((value >> 8) & 0xFF);
    data[offset + 3] = char(value & 0xFF);
}

QByteArray mp4Box(const QByteArray &type, const QByteArray &payload)
{
    QByteArray box(8, 0);
    mp4WriteU32(box, 0, quint32(8 + payload.size()));
    box.replace(4, 4, type.leftJustified(4, '\0'));
    box.append(payload);
    return box;
}

QByteArray mp4DataBox(quint32 dataType, const QByteArray &payload)
{
    QByteArray body(8, 0);
    mp4WriteU32(body, 0, dataType);
    body.append(payload);
    return mp4Box("data", body);
}

struct Mp4BoxInfo
{
    int start = 0;
    int size = 0;
    QByteArray type;
};

QVector<Mp4BoxInfo> mp4Children(const QByteArray &data, int from, int to)
{
    QVector<Mp4BoxInfo> result;
    int pos = from;
    while (pos + 8 <= to) {
        const quint32 size = mp4U32(data, pos);
        const QByteArray type = data.mid(pos + 4, 4);
        int header = 8;
        int total = int(size);
        if (size == 1) {
            if (pos + 16 > to)
                break;
            const quint64 largeSize = (quint64(mp4U32(data, pos + 8)) << 32) |
                                      mp4U32(data, pos + 12);
            total = int(qMin<quint64>(largeSize, quint64(to - pos)));
            header = 16;
        } else if (size == 0) {
            total = to - pos;
        }
        if (total < header || pos + total > to)
            break;
        result.append({pos, total, type});
        pos += total;
    }
    return result;
}

int mp4Find(const QVector<Mp4BoxInfo> &boxes, const char *type)
{
    const QByteArray wanted(type, 4);
    for (int i = 0; i < boxes.size(); ++i) {
        if (boxes.at(i).type == wanted)
            return i;
    }
    return -1;
}

void adjustChunkOffsets(QByteArray &data, int from, int to, qint32 delta)
{
    int pos = from;
    while (pos + 8 <= to) {
        const quint32 size = mp4U32(data, pos);
        if (size < 8 || pos + int(size) > to)
            break;
        const QByteArray type = data.mid(pos + 4, 4);
        if (type == "stco") {
            const quint32 count = mp4U32(data, pos + 12);
            int p = pos + 16;
            for (quint32 i = 0; i < count && p + 4 <= pos + int(size); ++i, p += 4) {
                const quint32 value = quint32(qint64(mp4U32(data, p)) + delta);
                mp4WriteU32(data, p, value);
            }
        } else if (type == "co64") {
            const quint32 count = mp4U32(data, pos + 12);
            int p = pos + 16;
            for (quint32 i = 0; i < count && p + 8 <= pos + int(size); ++i, p += 8) {
                quint64 value = (quint64(mp4U32(data, p)) << 32) | mp4U32(data, p + 4);
                value = quint64(qint64(value) + delta);
                mp4WriteU32(data, p, quint32(value >> 32));
                mp4WriteU32(data, p + 4, quint32(value & 0xFFFFFFFF));
            }
        } else if (type == "trak" || type == "mdia" || type == "minf" ||
                   type == "stbl" || type == "edts" || type == "dinf" ||
                   type == "udta") {
            adjustChunkOffsets(data, pos + 8, pos + int(size), delta);
        }
        pos += int(size);
    }
}

bool writeM4aTags(const QString &path, const QString &title, const QString &artist,
                  const QByteArray &coverJpeg)
{
    QFile input(path);
    if (!input.open(QIODevice::ReadOnly))
        return false;
    const QByteArray data = input.readAll();
    input.close();

    const QVector<Mp4BoxInfo> top = mp4Children(data, 0, data.size());
    const int moovIndex = mp4Find(top, "moov");
    if (moovIndex < 0)
        return false;
    const Mp4BoxInfo moov = top.at(moovIndex);

    const int mdatIndex = mp4Find(top, "mdat");
    const int mdatStart = (mdatIndex >= 0) ? top.at(mdatIndex).start : -1;

    QByteArray keptItems;
    QByteArray keptUdta;
    QByteArray metaHeader;

    const QVector<Mp4BoxInfo> moovChildren =
        mp4Children(data, moov.start + 8, moov.start + moov.size);

    const int udtaIndex = mp4Find(moovChildren, "udta");
    if (udtaIndex >= 0) {
        const Mp4BoxInfo udta = moovChildren.at(udtaIndex);
        const QVector<Mp4BoxInfo> udtaChildren =
            mp4Children(data, udta.start + 8, udta.start + udta.size);

        for (const Mp4BoxInfo &child : udtaChildren) {
            if (child.type != "meta")
                keptUdta.append(data.mid(child.start, child.size));
        }

        const int metaIndex = mp4Find(udtaChildren, "meta");
        if (metaIndex >= 0) {
            const Mp4BoxInfo meta = udtaChildren.at(metaIndex);
            metaHeader = data.mid(meta.start + 8, 4);

            const QVector<Mp4BoxInfo> metaChildren =
                mp4Children(data, meta.start + 12, meta.start + meta.size);

            const int hdlrIndex = mp4Find(metaChildren, "hdlr");
            if (hdlrIndex >= 0) {
                const Mp4BoxInfo hdlr = metaChildren.at(hdlrIndex);
                metaHeader.append(data.mid(hdlr.start, hdlr.size));
            }

            const int ilstIndex = mp4Find(metaChildren, "ilst");
            if (ilstIndex >= 0) {
                const Mp4BoxInfo ilst = metaChildren.at(ilstIndex);
                const QVector<Mp4BoxInfo> items =
                    mp4Children(data, ilst.start + 8, ilst.start + ilst.size);

                for (const Mp4BoxInfo &item : items) {
                    const bool isTarget = (item.type == QByteArray("\xA9" "nam", 4) ||
                                           item.type == QByteArray("\xA9" "ART", 4) ||
                                           item.type == "covr");
                    if (!isTarget)
                        keptItems.append(data.mid(item.start, item.size));
                }
            }
        }
    }

    if (metaHeader.isEmpty()) {
        metaHeader = QByteArray(4, 0);
        QByteArray hdlrPayload(24, 0);
        hdlrPayload.replace(8, 4, "mdir");
        hdlrPayload.append('\0');
        metaHeader.append(mp4Box("hdlr", hdlrPayload));
    }

    QByteArray items = keptItems;
    if (!title.isEmpty())
        items.append(mp4Box(QByteArray("\xA9" "nam", 4), mp4DataBox(1, title.toUtf8())));
    if (!artist.isEmpty())
        items.append(mp4Box(QByteArray("\xA9" "ART", 4), mp4DataBox(1, artist.toUtf8())));
    if (!coverJpeg.isEmpty())
        items.append(mp4Box("covr", mp4DataBox(13, coverJpeg)));

    const QByteArray meta = mp4Box("meta", metaHeader + mp4Box("ilst", items));

    QByteArray udtaPayload = keptUdta;
    udtaPayload.append(meta);

    QByteArray moovPayload;
    for (const Mp4BoxInfo &child : moovChildren) {
        if (child.type != "udta")
            moovPayload.append(data.mid(child.start, child.size));
    }
    moovPayload.append(mp4Box("udta", udtaPayload));

    const int delta = (8 + moovPayload.size()) - moov.size;

    if (delta != 0 && mdatStart > moov.start)
        adjustChunkOffsets(moovPayload, 0, moovPayload.size(), delta);

    const QByteArray newMoov = mp4Box("moov", moovPayload);

    QByteArray result;
    result.reserve(data.size() + delta);
    for (int i = 0; i < top.size(); ++i) {
        if (i == moovIndex)
            result.append(newMoov);
        else
            result.append(data.mid(top.at(i).start, top.at(i).size));
    }

    const QString backupPath = path + ".bak";
    QFile::remove(backupPath);
    if (!QFile::copy(path, backupPath))
        return false;

    const QString tempPath = path + ".tmp";
    QFile output(tempPath);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const bool written = (output.write(result) == result.size());
    output.close();
    if (!written) {
        QFile::remove(tempPath);
        return false;
    }

    if (!QFile::remove(path)) {
        QFile::remove(tempPath);
        return false;
    }
    if (!QFile::rename(tempPath, path)) {
        QFile::rename(backupPath, path);
        return false;
    }
    return true;
}

}

QJsonObject Track::toJson() const
{
    QJsonObject obj;
    obj["name"]     = name;
    obj["path"]     = filePath;
    obj["duration"] = durationMs;
    obj["favorite"] = favorite;
    obj["artist"]   = artist;
    obj["title"]    = title;
    obj["playlists"] = QJsonArray::fromStringList(playlists);
    return obj;
}

Track Track::fromJson(const QJsonObject &obj)
{
    Track t;
    t.name       = obj["name"].toString();
    t.filePath   = obj["path"].toString();
    t.durationMs = static_cast<qint64>(obj["duration"].toDouble(0));
    t.favorite   = obj["favorite"].toBool(false);
    t.artist     = obj["artist"].toString();
    t.title      = obj["title"].toString();
    t.playlists  = obj["playlists"].toVariant().toStringList();
    return t;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Sonora");
    resize(700, 540);
    setMinimumSize(640, 480);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUi();

    m_playPauseShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    m_playPauseShortcut->setContext(Qt::WindowShortcut);
    connect(m_playPauseShortcut, &QShortcut::activated,
            this, &MainWindow::togglePlay);

    loadPlaylist();
    updatePlaylistButtons();

    scanMusicFolder();

    QSettings settings;
    QString lastPath = settings.value("lastTrackPath").toString();
    int lastIndex = -1;
    if (!lastPath.isEmpty()) {
        for (int i = 0; i < m_playlist.size(); ++i) {
            if (m_playlist[i].filePath == lastPath) {
                lastIndex = i;
                break;
            }
        }
    }

    if (lastIndex != -1) {
        loadTrack(lastIndex);
    } else if (!m_playlist.isEmpty()) {
        loadTrack(0);
    } else {
        refreshPlaylist();
    }
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_WIN
    systemVolume().setChangeCallback(nullptr);
#endif
    savePlaylist();
}

void MainWindow::setupUi()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, QColor(241, 241, 241));
    darkPalette.setColor(QPalette::Base, QColor(37, 37, 38));
    darkPalette.setColor(QPalette::AlternateBase, QColor(45, 45, 48));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::ToolTipText, QColor(241, 241, 241));
    darkPalette.setColor(QPalette::Text, QColor(241, 241, 241));
    darkPalette.setColor(QPalette::Button, QColor(62, 62, 66));
    darkPalette.setColor(QPalette::ButtonText, QColor(241, 241, 241));
    darkPalette.setColor(QPalette::BrightText, QColor(232, 17, 35));
    darkPalette.setColor(QPalette::Link, QColor(241, 241, 241));
    darkPalette.setColor(QPalette::Highlight, QColor(77, 77, 77));
    darkPalette.setColor(QPalette::HighlightedText, QColor(241, 241, 241));
    qApp->setPalette(darkPalette);

    qApp->setStyleSheet(
        "QToolTip { color: #f1f1f1; background-color: #1e1e1e; border: 1px solid #3e3e42; }"
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 0px; border: none; }"
        "QScrollBar::handle:vertical { background: #4d4d4d; min-height: 20px; border: none; border-radius: 5px; margin: 2px; }"
        "QScrollBar::handle:vertical:hover { background: #5e5e5e; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar:horizontal { background: transparent; height: 10px; margin: 0px; border: none; }"
        "QScrollBar::handle:horizontal { background: #4d4d4d; min-width: 20px; border: none; border-radius: 5px; margin: 2px; }"
        "QScrollBar::handle:horizontal:hover { background: #5e5e5e; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
        "QFileDialog { background-color: #252526; }"
        "QFileDialog QWidget { color: #f1f1f1; }"
        "QFileDialog QPushButton { background-color: #3e3e42; color: #f1f1f1; border: 1px solid #4d4d4d; border-radius: 4px; padding: 4px 12px; }"
        "QFileDialog QPushButton:hover { background-color: #4d4d4d; border: 1px solid #5e5e5e; }"
        "QFileDialog QLineEdit { background-color: #1e1e1e; color: #f1f1f1; border: 1px solid #3e3e42; padding: 2px; }"
        "QFileDialog QTreeView { background-color: #1e1e1e; color: #f1f1f1; border: 1px solid #3e3e42; }"
        "QFileDialog QListView { background-color: #1e1e1e; color: #f1f1f1; border: 1px solid #3e3e42; }"
        "QFileDialog QComboBox { background-color: #1e1e1e; color: #f1f1f1; border: 1px solid #3e3e42; }"
        "QFileDialog QHeaderView::section { background-color: #252526; color: #a0a0a0; border: none; border-right: 1px solid #3e3e42; border-bottom: 1px solid #3e3e42; padding: 4px; }"
        "QInputDialog { background-color: #252526; }"
        "QInputDialog QLabel { color: #f1f1f1; }"
        "QInputDialog QLineEdit { background-color: #1e1e1e; color: #f1f1f1; border: 1px solid #3e3e42; padding: 4px; }"
        "QInputDialog QPushButton { background-color: #3e3e42; color: #f1f1f1; border: 1px solid #4d4d4d; border-radius: 4px; padding: 4px 12px; }"
        );

    QWidget *central = new QWidget(this);
    central->setObjectName("central");
    central->setStyleSheet("#central { background: transparent; }");
    setCentralWidget(central);

    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QWidget *titleBar = new QWidget(central);
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(22);
    titleBar->setStyleSheet(
        "#titleBar { background-color: #1e1e1e;"
        " border-top-left-radius: 12px; border-top-right-radius: 12px; }"
        );

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 0, 2, 0);
    titleLayout->setSpacing(2);

    QLabel *iconLabel = new QLabel(titleBar);
    const QPixmap iconPm = QIcon(":/Sonora.png").pixmap(128, 128);
    iconLabel->setPixmap(iconPm.scaled(21, 21,
                                       Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation));
    iconLabel->setFixedSize(21, 21);
    iconLabel->setStyleSheet("background: transparent;");
    titleLayout->addWidget(iconLabel, 0, Qt::AlignVCenter);

    QLabel *appTitle = new QLabel("Sonora");
    appTitle->setStyleSheet("color: #a0a0a0; font-size: 13px; background: transparent;");
    titleLayout->addWidget(appTitle, 0, Qt::AlignVCenter);
    titleLayout->addStretch();

    const QString winBtnStyle =
        "QPushButton { background: transparent; border: none; border-radius: 6px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.10); }";

    m_minButton = new QPushButton(titleBar);
    m_minButton->setFixedSize(30, 20);
    m_minButton->setIcon(makeMinimizeIcon(kIconGray));
    m_minButton->setIconSize(QSize(14, 14));
    m_minButton->setCursor(Qt::PointingHandCursor);
    m_minButton->setStyleSheet(winBtnStyle);
    m_minButton->setToolTip("Свернуть");
    connect(m_minButton, &QPushButton::clicked, this, &MainWindow::onMinimizeWindow);
    titleLayout->addWidget(m_minButton);

    m_closeButton = new QPushButton(titleBar);
    m_closeButton->setFixedSize(30, 20);
    m_closeButton->setIcon(makeCloseIcon(kIconGray));
    m_closeButton->setIconSize(QSize(14, 14));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 6px; }"
        "QPushButton:hover { background: #e81123; }"
        );
    m_closeButton->setToolTip("Закрыть");
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseWindow);
    titleLayout->addWidget(m_closeButton);

    rootLayout->addWidget(titleBar);

    m_outerWidget = new QWidget(central);
    m_outerWidget->setObjectName("outer");
    m_outerWidget->setStyleSheet(
        "#outer { background-color: #252526; border-bottom-left-radius: 12px;"
        " border-bottom-right-radius: 12px; }"
        );
    QHBoxLayout *outerLayout = new QHBoxLayout(m_outerWidget);
    outerLayout->setContentsMargins(0, 14, 14, 14);
    outerLayout->setSpacing(0);
    rootLayout->addWidget(m_outerWidget, 1);

    setupLeftPanel();
    m_leftPanel->setParent(m_outerWidget);
    m_leftPanel->setFixedWidth(284);
    outerLayout->addWidget(m_leftPanel, 0);

    QWidget *divider = new QWidget(m_outerWidget);
    divider->setFixedWidth(1);
    divider->setStyleSheet("background-color: #3e3e42;");
    outerLayout->addWidget(divider);

    setupRightPanel();
    m_rightPanel->setParent(m_outerWidget);
    outerLayout->addWidget(m_rightPanel, 1);

    m_player = new QMediaPlayer(this);
    m_audioOut = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOut);
    m_audioOut->setVolume(0.8);

    connect(m_player, &QMediaPlayer::positionChanged,
            this, &MainWindow::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &MainWindow::onDurationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &MainWindow::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::onPlaybackStateChanged);
    connect(m_player, &QMediaPlayer::metaDataChanged,
            this, &MainWindow::onMetaDataChanged);

    setupSettingsOverlay();
    m_settingsOverlay->hide();

    setupEditOverlay();
}

void MainWindow::setupLeftPanel()
{
    m_leftPanel = new QWidget();
    m_leftPanel->setObjectName("leftPanel");
    m_leftPanel->setStyleSheet("#leftPanel { background: transparent; }");

    QVBoxLayout *layout = new QVBoxLayout(m_leftPanel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *headerRow = new QWidget();
    headerRow->setFixedHeight(38);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(14, 0, 0, 0);
    headerLayout->setSpacing(0);

    m_settingsButton = new QPushButton();
    m_settingsButton->setFixedSize(34, 34);
    m_settingsButton->setIcon(makeGearIcon(kIconGray, 18));
    m_settingsButton->setIconSize(QSize(18, 18));
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 17px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); }"
        );
    m_settingsButton->setToolTip("Настройки");
    connect(m_settingsButton, &QPushButton::clicked,
            this, &MainWindow::toggleSettings);
    headerLayout->addWidget(m_settingsButton);

    m_editTrackButton = new QPushButton();
    m_editTrackButton->setFixedSize(34, 34);
    m_editTrackButton->setIcon(makePencilIcon(kIconGray, 18));
    m_editTrackButton->setIconSize(QSize(18, 18));
    m_editTrackButton->setCursor(Qt::PointingHandCursor);
    m_editTrackButton->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 17px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); }"
        );
    m_editTrackButton->setToolTip("Редактировать трек");
    connect(m_editTrackButton, &QPushButton::clicked,
            this, &MainWindow::openTrackEditor);
    headerLayout->addWidget(m_editTrackButton);

    headerLayout->addStretch();
    layout->addWidget(headerRow);

    layout->addSpacing(18);
    m_coverWidget = new CoverWidget();
    m_coverWidget->setFixedSize(184, 184);
    layout->addWidget(m_coverWidget, 0, Qt::AlignHCenter);

    layout->addSpacing(14);
    m_titleLabel = new QLabel("Нет трека");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_titleLabel->setFixedHeight(24);
    m_titleLabel->setStyleSheet(
        "color: #f1f1f1; font-size: 17px; font-weight: bold; background: transparent;");
    layout->addWidget(m_titleLabel);

    m_artistLabel = new QLabel("Добавьте музыку");
    m_artistLabel->setAlignment(Qt::AlignCenter);
    m_artistLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_artistLabel->setFixedHeight(18);
    m_artistLabel->setStyleSheet(
        "color: #a0a0a0; font-size: 12px; background: transparent;");
    layout->addWidget(m_artistLabel);

    layout->addStretch(1);

    QWidget *progressBlock = new QWidget();
    QVBoxLayout *progressLayout = new QVBoxLayout(progressBlock);
    progressLayout->setContentsMargins(35, 0, 35, 0);
    progressLayout->setSpacing(4);

    m_progressSlider = new ClickableSlider(Qt::Horizontal);
    m_progressSlider->setObjectName("progressSlider");
    m_progressSlider->setMinimumHeight(18);
    m_progressSlider->setRange(0, 0);
    m_progressSlider->setCursor(Qt::PointingHandCursor);
    m_progressSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 6px; background: #3e3e42; border-radius: 3px; }"
        "QSlider::sub-page:horizontal { background: #c0c0c0; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 12px; margin: -4px 0;"
        " background: #f1f1f1; border-radius: 6px; }"
        );
    connect(m_progressSlider, &QSlider::sliderMoved,
            this, &MainWindow::onSeekClicked);
    progressLayout->addWidget(m_progressSlider);

    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->setContentsMargins(0, 0, 0, 0);
    m_currentTime = new QLabel("0:00");
    m_currentTime->setStyleSheet("color: #a0a0a0; font-size: 11px;");
    timeLayout->addWidget(m_currentTime);
    timeLayout->addStretch();
    m_totalTime = new QLabel("0:00");
    m_totalTime->setStyleSheet("color: #a0a0a0; font-size: 11px;");
    timeLayout->addWidget(m_totalTime);
    progressLayout->addLayout(timeLayout);

    layout->addWidget(progressBlock);
    layout->addSpacing(13);

    QWidget *controlsRow = new QWidget();
    controlsRow->setFixedHeight(60);
    QHBoxLayout *ctrlLayout = new QHBoxLayout(controlsRow);
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setSpacing(5);
    ctrlLayout->addStretch();

    const QString flatBtn =
        "QPushButton { background: transparent; border: none; border-radius: 20px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); border-radius: 20px; }";

    m_heartButton = new QPushButton();
    m_heartButton->setFixedSize(40, 40);
    m_heartButton->setIcon(makeHeartIcon(false, kIconGray, 20));
    m_heartButton->setIconSize(QSize(20, 20));
    m_heartButton->setCursor(Qt::PointingHandCursor);
    m_heartButton->setStyleSheet(flatBtn);
    m_heartButton->setToolTip("В избранное");
    connect(m_heartButton, &QPushButton::clicked,
            this, &MainWindow::toggleFavoriteCurrent);
    ctrlLayout->addWidget(m_heartButton);

    m_prevButton = new QPushButton();
    m_prevButton->setFixedSize(40, 40);
    m_prevButton->setIcon(makePrevIcon(kWhite, 20));
    m_prevButton->setIconSize(QSize(20, 20));
    m_prevButton->setCursor(Qt::PointingHandCursor);
    m_prevButton->setStyleSheet(flatBtn);
    m_prevButton->setToolTip("Предыдущий трек");
    connect(m_prevButton, &QPushButton::clicked,
            this, &MainWindow::playPrevious);
    ctrlLayout->addWidget(m_prevButton);

    m_playButton = new QPushButton();
    m_playButton->setFixedSize(56, 56);
    m_playButton->setIcon(makePlayIcon(kWhite, 22));
    m_playButton->setIconSize(QSize(22, 22));
    m_playButton->setCursor(Qt::PointingHandCursor);
    m_playButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); border: none; border-radius: 28px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); border-radius: 28px; }"
        );
    m_playButton->setToolTip("Воспроизвести / пауза");
    connect(m_playButton, &QPushButton::clicked, this, &MainWindow::togglePlay);
    ctrlLayout->addWidget(m_playButton);

    m_nextButton = new QPushButton();
    m_nextButton->setFixedSize(40, 40);
    m_nextButton->setIcon(makeNextIcon(kWhite, 20));
    m_nextButton->setIconSize(QSize(20, 20));
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_nextButton->setStyleSheet(flatBtn);
    m_nextButton->setToolTip("Следующий трек");
    connect(m_nextButton, &QPushButton::clicked, this, &MainWindow::playNext);
    ctrlLayout->addWidget(m_nextButton);

    m_modeButton = new QPushButton();
    m_modeButton->setFixedSize(40, 40);
    m_modeButton->setIconSize(QSize(20, 20));
    m_modeButton->setCursor(Qt::PointingHandCursor);
    m_modeButton->setStyleSheet(flatBtn);
    connect(m_modeButton, &QPushButton::clicked,
            this, &MainWindow::cyclePlaybackMode);
    ctrlLayout->addWidget(m_modeButton);

    ctrlLayout->addStretch();
    layout->addWidget(controlsRow);
    layout->addSpacing(20);

    updateModeButton();
}

void MainWindow::setupRightPanel()
{
    m_rightPanel = new QWidget();
    m_rightPanel->setObjectName("rightPanel");
    m_rightPanel->setStyleSheet("#rightPanel { background: transparent; }");

    QVBoxLayout *layout = new QVBoxLayout(m_rightPanel);
    layout->setContentsMargins(14, 14, 0, 0);
    layout->setSpacing(10);

    QWidget *filterRow = new QWidget();
    QHBoxLayout *filterLayout = new QHBoxLayout(filterRow);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->setSpacing(8);

    const QString baseStyle =
        "QPushButton { "
        "  border: none; "
        "  border-radius: 14px; "
        "  font-size: 12px; "
        "  padding: 0px 12px; "
        "  outline: none; "
        "}";

    const QString activeStyle = baseStyle +
                                "QPushButton { background-color: rgba(255,255,255,0.15); color: #f1f1f1; } "
                                "QPushButton:hover { background-color: rgba(255,255,255,0.20); }";

    const QString idleStyle = baseStyle +
                              "QPushButton { background-color: rgba(255,255,255,0.03); color: #a0a0a0; } "
                              "QPushButton:hover { color: #f1f1f1; background-color: rgba(255,255,255,0.08); }";

    m_allFilterBtn = new QPushButton("Все");
    m_allFilterBtn->setFixedSize(66, 28);
    m_allFilterBtn->setCursor(Qt::PointingHandCursor);
    m_allFilterBtn->setStyleSheet(activeStyle);
    connect(m_allFilterBtn, &QPushButton::clicked,
            this, &MainWindow::setFilterAll);

    m_favFilterBtn = new QPushButton("Избранное");
    m_favFilterBtn->setFixedSize(100, 28);
    m_favFilterBtn->setCursor(Qt::PointingHandCursor);
    m_favFilterBtn->setStyleSheet(idleStyle);
    connect(m_favFilterBtn, &QPushButton::clicked,
            this, &MainWindow::setFilterFavorites);

    m_filterButtonsWidget = new QWidget();
    m_filterButtonsWidget->setStyleSheet("background: transparent;");
    m_filterButtonsLayout = new QHBoxLayout(m_filterButtonsWidget);
    m_filterButtonsLayout->setContentsMargins(0, 0, 0, 0);
    m_filterButtonsLayout->setSpacing(8);

    QWidget *filterContent = new QWidget();
    filterContent->setStyleSheet("background: transparent;");
    QHBoxLayout *filterContentLayout = new QHBoxLayout(filterContent);
    filterContentLayout->setContentsMargins(0, 0, 0, 0);
    filterContentLayout->setSpacing(8);
    filterContentLayout->addWidget(m_allFilterBtn, 0, Qt::AlignTop);
    filterContentLayout->addWidget(m_favFilterBtn, 0, Qt::AlignTop);
    filterContentLayout->addWidget(m_filterButtonsWidget, 0, Qt::AlignTop);
    filterContentLayout->addStretch();

    QScrollArea *filterScroll = new QScrollArea();
    filterScroll->setWidgetResizable(true);
    filterScroll->setWidget(filterContent);
    filterScroll->setFixedHeight(42);
    filterScroll->setFrameShape(QFrame::NoFrame);
    filterScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    filterScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    filterScroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
        );

    filterScroll->setHorizontalScrollBar(new RoundScrollBar(Qt::Horizontal));
    filterLayout->addWidget(filterScroll, 1);

    m_countLabel = new QLabel("(0)");
    m_countLabel->setAlignment(Qt::AlignCenter);
    m_countLabel->setFixedHeight(28);
    m_countLabel->setStyleSheet(
        "QLabel { color: #a0a0a0; font-size: 12px;"
        " background-color: rgba(255,255,255,0.03);"
        " border: none; border-radius: 14px; padding: 0px 12px; }"
        );

    QWidget *countBox = new QWidget();
    countBox->setFixedHeight(28);
    QHBoxLayout *countLayout = new QHBoxLayout(countBox);
    countLayout->setContentsMargins(0, 0, 0, 0);
    countLayout->addWidget(m_countLabel);
    filterLayout->addWidget(countBox, 0, Qt::AlignTop);

    layout->addWidget(filterRow);

    m_listWidget = new QListWidget();
    m_listWidget->viewport()->setAutoFillBackground(false);
    m_listWidget->setStyleSheet(
        "QListWidget { background: transparent; border: none; outline: none; }"
        "QListWidget::item { "
        "  background-color: rgba(255, 255, 255, 0.03); "
        "  border: none; "
        "  border-radius: 8px; "
        "  margin: 2px 0; "
        "}"
        "QListWidget::item:hover { "
        "  background-color: rgba(255, 255, 255, 0.08); "
        "}"
        "QListWidget::item:selected { "
        "  background-color: rgba(255, 255, 255, 0.15); "
        "}"
        );
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setVerticalScrollBar(new RoundScrollBar(Qt::Vertical));

    connect(m_listWidget, &QListWidget::itemClicked,
            this, &MainWindow::onPlaylistItemClicked);
    layout->addWidget(m_listWidget, 1);
}

void MainWindow::setupSettingsOverlay()
{
    m_settingsOverlay = new QWidget(m_outerWidget);
    m_settingsOverlay->setObjectName("settingsOverlay");
    m_settingsOverlay->setStyleSheet(
        "#settingsOverlay { background-color: #2d2d30;"
        " border-top-left-radius: 0px;"
        " border-top-right-radius: 0px;"
        " border-bottom-left-radius: 12px;"
        " border-bottom-right-radius: 0px; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(m_settingsOverlay);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(12);

    QWidget *headerRow = new QWidget();
    headerRow->setFixedHeight(38);
    QHBoxLayout *topRow = new QHBoxLayout(headerRow);
    topRow->setContentsMargins(0, 0, 0, 0);

    QPushButton *closeBtn = new QPushButton();
    closeBtn->setFixedSize(34, 34);
    closeBtn->setIcon(makeCloseIcon(kIconGray, 16));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 17px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); }"
        );
    closeBtn->setToolTip("Закрыть настройки");
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::toggleSettings);
    topRow->addWidget(closeBtn);
    topRow->addStretch();
    layout->addWidget(headerRow);

    m_addTrackButton = new QPushButton("  Добавить трек");
    m_addTrackButton->setFixedHeight(40);
    m_addTrackButton->setIcon(makePlusIcon(kWhite, 16));
    m_addTrackButton->setIconSize(QSize(16, 16));
    m_addTrackButton->setCursor(Qt::PointingHandCursor);
    m_addTrackButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); color: #f1f1f1; border: none;"
        " border-radius: 10px; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); }"
        );
    connect(m_addTrackButton, &QPushButton::clicked, this, &MainWindow::addTracks);

    m_playlistActionBtn = new QPushButton("  Добавить плейлист");
    m_playlistActionBtn->setFixedHeight(40);
    m_playlistActionBtn->setCursor(Qt::PointingHandCursor);
    m_playlistActionBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); color: #f1f1f1; border: none;"
        " border-radius: 10px; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); }"
        "QPushButton:disabled { color: #7a7a7a; }"
        );
    connect(m_playlistActionBtn, &QPushButton::clicked, this, &MainWindow::onPlaylistActionClicked);

    m_addToPlaylistLabel = new QLabel();
    m_addToPlaylistLabel->setContentsMargins(0, 0, 0, 8);
    m_addToPlaylistLabel->setAlignment(Qt::AlignCenter);
    m_addToPlaylistLabel->setStyleSheet("color: #a0a0a0; font-size: 12px; font-weight: bold;");
    m_addToPlaylistLabel->hide();

    m_playlistStack = new QStackedWidget();
    m_playlistStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_playlistRowWidget = new QWidget();
    QVBoxLayout *playlistRowPageLayout = new QVBoxLayout(m_playlistRowWidget);
    playlistRowPageLayout->setContentsMargins(0, 0, 0, 0);
    playlistRowPageLayout->setSpacing(10);

    QWidget *playlistRow = new QWidget();
    QHBoxLayout *playlistRowLayout = new QHBoxLayout(playlistRow);
    playlistRowLayout->setContentsMargins(0, 0, 0, 0);
    playlistRowLayout->setSpacing(8);
    playlistRowLayout->addWidget(m_playlistActionBtn, 1);

    m_cancelEditBtn = new QPushButton("Отмена");
    m_cancelEditBtn->setFixedHeight(40);
    m_cancelEditBtn->setCursor(Qt::PointingHandCursor);
    m_cancelEditBtn->setStyleSheet(
        "QPushButton { background-color: #3e3e42; color: #f1f1f1; border: none;"
        " border-radius: 10px; font-size: 13px; }"
        "QPushButton:hover { background-color: #4d4d4d; }"
        );
    m_cancelEditBtn->hide();
    connect(m_cancelEditBtn, &QPushButton::clicked,
            this, &MainWindow::cancelAddToPlaylist);
    playlistRowLayout->addWidget(m_cancelEditBtn, 1);

    m_renamePlaylistBtn = new QPushButton();
    m_renamePlaylistBtn->setFixedSize(40, 40);
    m_renamePlaylistBtn->setIcon(makePencilIcon(kWhite, 18));
    m_renamePlaylistBtn->setIconSize(QSize(18, 18));
    m_renamePlaylistBtn->setCursor(Qt::PointingHandCursor);
    m_renamePlaylistBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); border: none; border-radius: 10px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); }"
        );
    m_renamePlaylistBtn->setToolTip("Переименовать плейлист");
    m_renamePlaylistBtn->hide();
    connect(m_renamePlaylistBtn, &QPushButton::clicked,
            this, &MainWindow::onRenamePlaylistClicked);
    playlistRowLayout->addWidget(m_renamePlaylistBtn, 0);

    m_deletePlaylistBtn = new QPushButton();
    m_deletePlaylistBtn->setFixedSize(40, 40);
    m_deletePlaylistBtn->setIcon(makeTrashIcon(kWhite, 18));
    m_deletePlaylistBtn->setIconSize(QSize(18, 18));
    m_deletePlaylistBtn->setCursor(Qt::PointingHandCursor);
    m_deletePlaylistBtn->setStyleSheet(
        "QPushButton { background-color: #e81123; border: none; border-radius: 10px; }"
        "QPushButton:hover { background-color: #f1707a; }"
        "QPushButton:pressed { background-color: #c50f1f; }"
        );
    m_deletePlaylistBtn->setToolTip("Удалить плейлист");
    m_deletePlaylistBtn->hide();
    connect(m_deletePlaylistBtn, &QPushButton::clicked,
            this, &MainWindow::onDeletePlaylistClicked);
    playlistRowLayout->addWidget(m_deletePlaylistBtn, 0);

    playlistRowPageLayout->addStretch();
    playlistRowPageLayout->addWidget(m_addTrackButton);
    playlistRowPageLayout->addWidget(m_addToPlaylistLabel);
    playlistRowPageLayout->addWidget(playlistRow);
    m_playlistStack->addWidget(m_playlistRowWidget);

    m_deleteConfirmWidget = new QWidget();
    QVBoxLayout *confirmLayout = new QVBoxLayout(m_deleteConfirmWidget);
    confirmLayout->setContentsMargins(0, 0, 0, 0);
    confirmLayout->setSpacing(0);
    confirmLayout->addStretch();

    m_deleteConfirmLabel = new QLabel();
    m_deleteConfirmLabel->setWordWrap(true);
    m_deleteConfirmLabel->setFixedHeight(35);
    m_deleteConfirmLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_deleteConfirmLabel->setStyleSheet("color: #a0a0a0; font-size: 12px; font-weight: bold;");
    confirmLayout->addWidget(m_deleteConfirmLabel);

    QHBoxLayout *confirmButtons = new QHBoxLayout();
    confirmButtons->setContentsMargins(0, 0, 0, 0);
    confirmButtons->setSpacing(8);

    QPushButton *confirmYesBtn = new QPushButton("Да");
    confirmYesBtn->setFixedHeight(40);
    confirmYesBtn->setCursor(Qt::PointingHandCursor);
    confirmYesBtn->setStyleSheet(
        "QPushButton { background-color: #e81123; color: #ffffff; border: none;"
        " border-radius: 8px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #f1707a; }"
        );
    connect(confirmYesBtn, &QPushButton::clicked,
            this, &MainWindow::onConfirmDeletePlaylist);
    confirmButtons->addWidget(confirmYesBtn, 1);

    QPushButton *confirmNoBtn = new QPushButton("Нет");
    confirmNoBtn->setFixedHeight(40);
    confirmNoBtn->setCursor(Qt::PointingHandCursor);
    confirmNoBtn->setStyleSheet(
        "QPushButton { background-color: #3e3e42; color: #f1f1f1; border: none;"
        " border-radius: 8px; font-size: 13px; }"
        "QPushButton:hover { background-color: #4d4d4d; }"
        );
    connect(confirmNoBtn, &QPushButton::clicked,
            this, &MainWindow::hideDeleteConfirm);
    confirmButtons->addWidget(confirmNoBtn, 1);

    confirmLayout->addLayout(confirmButtons);
    m_playlistStack->addWidget(m_deleteConfirmWidget);

    m_newPlaylistWidget = new QWidget();
    QVBoxLayout *newPlaylistLayout = new QVBoxLayout(m_newPlaylistWidget);
    newPlaylistLayout->setContentsMargins(0, 0, 0, 0);
    newPlaylistLayout->setSpacing(8);
    newPlaylistLayout->addStretch();

    QLabel *newPlaylistHint = new QLabel("Имя (не более 10 символов)");
    newPlaylistHint->setAlignment(Qt::AlignCenter);
    newPlaylistHint->setStyleSheet("color: #a0a0a0; font-size: 12px; font-weight: bold;");
    newPlaylistLayout->addWidget(newPlaylistHint);

    m_newPlaylistEdit = new QLineEdit();
    m_newPlaylistEdit->setMaxLength(10);
    m_newPlaylistEdit->setAlignment(Qt::AlignCenter);
    m_newPlaylistEdit->setFixedHeight(30);
    m_newPlaylistEdit->setStyleSheet(
        "QLineEdit { background-color: #1e1e1e; color: #f1f1f1;"
        " border: 1px solid #3e3e42; border-radius: 8px; padding: 0 8px; font-size: 12px; }"
        "QLineEdit:focus { border: 1px solid #5e5e5e; }"
        );
    connect(m_newPlaylistEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onCreatePlaylistConfirmed);
    newPlaylistLayout->addWidget(m_newPlaylistEdit);

    QHBoxLayout *newPlaylistButtons = new QHBoxLayout();
    newPlaylistButtons->setContentsMargins(0, 0, 0, 0);
    newPlaylistButtons->setSpacing(8);

    QPushButton *createPlaylistBtn = new QPushButton("Создать");
    createPlaylistBtn->setFixedHeight(40);
    createPlaylistBtn->setCursor(Qt::PointingHandCursor);
    createPlaylistBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); color: #f1f1f1; border: none;"
        " border-radius: 8px; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); }"
        );
    connect(createPlaylistBtn, &QPushButton::clicked,
            this, &MainWindow::onCreatePlaylistConfirmed);
    newPlaylistButtons->addWidget(createPlaylistBtn, 1);

    QPushButton *cancelPlaylistBtn = new QPushButton("Отмена");
    cancelPlaylistBtn->setFixedHeight(40);
    cancelPlaylistBtn->setCursor(Qt::PointingHandCursor);
    cancelPlaylistBtn->setStyleSheet(
        "QPushButton { background-color: #3e3e42; color: #f1f1f1; border: none;"
        " border-radius: 8px; font-size: 13px; }"
        "QPushButton:hover { background-color: #4d4d4d; }"
        );
    connect(cancelPlaylistBtn, &QPushButton::clicked,
            this, &MainWindow::hideCreatePlaylist);
    newPlaylistButtons->addWidget(cancelPlaylistBtn, 1);

    newPlaylistLayout->addLayout(newPlaylistButtons);

    QShortcut *cancelPlaylistShortcut =
        new QShortcut(QKeySequence(Qt::Key_Escape), m_newPlaylistWidget);
    cancelPlaylistShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(cancelPlaylistShortcut, &QShortcut::activated,
            this, &MainWindow::hideCreatePlaylist);

    m_playlistStack->addWidget(m_newPlaylistWidget);

    m_renamePlaylistWidget = new QWidget();
    QVBoxLayout *renameLayout = new QVBoxLayout(m_renamePlaylistWidget);
    renameLayout->setContentsMargins(0, 0, 0, 0);
    renameLayout->setSpacing(8);
    renameLayout->addStretch();

    QLabel *renameHint = new QLabel("Имя (не более 10 символов)");
    renameHint->setAlignment(Qt::AlignCenter);
    renameHint->setStyleSheet("color: #a0a0a0; font-size: 12px; font-weight: bold;");
    renameLayout->addWidget(renameHint);

    m_renamePlaylistEdit = new QLineEdit();
    m_renamePlaylistEdit->setMaxLength(10);
    m_renamePlaylistEdit->setAlignment(Qt::AlignCenter);
    m_renamePlaylistEdit->setFixedHeight(30);
    m_renamePlaylistEdit->setStyleSheet(
        "QLineEdit { background-color: #1e1e1e; color: #f1f1f1;"
        " border: 1px solid #3e3e42; border-radius: 8px; padding: 0 8px; font-size: 12px; }"
        "QLineEdit:focus { border: 1px solid #5e5e5e; }"
        );
    connect(m_renamePlaylistEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onRenamePlaylistConfirmed);
    renameLayout->addWidget(m_renamePlaylistEdit);

    QHBoxLayout *renameButtons = new QHBoxLayout();
    renameButtons->setContentsMargins(0, 0, 0, 0);
    renameButtons->setSpacing(8);

    QPushButton *renameSaveBtn = new QPushButton("Сохранить");
    renameSaveBtn->setFixedHeight(40);
    renameSaveBtn->setCursor(Qt::PointingHandCursor);
    renameSaveBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); color: #f1f1f1; border: none;"
        " border-radius: 8px; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); }"
        );
    connect(renameSaveBtn, &QPushButton::clicked,
            this, &MainWindow::onRenamePlaylistConfirmed);
    renameButtons->addWidget(renameSaveBtn, 1);

    QPushButton *renameCancelBtn = new QPushButton("Отмена");
    renameCancelBtn->setFixedHeight(40);
    renameCancelBtn->setCursor(Qt::PointingHandCursor);
    renameCancelBtn->setStyleSheet(
        "QPushButton { background-color: #3e3e42; color: #f1f1f1; border: none;"
        " border-radius: 8px; font-size: 13px; }"
        "QPushButton:hover { background-color: #4d4d4d; }"
        );
    connect(renameCancelBtn, &QPushButton::clicked,
            this, &MainWindow::hideRenamePlaylist);
    renameButtons->addWidget(renameCancelBtn, 1);

    renameLayout->addLayout(renameButtons);

    QShortcut *renameCancelShortcut =
        new QShortcut(QKeySequence(Qt::Key_Escape), m_renamePlaylistWidget);
    renameCancelShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(renameCancelShortcut, &QShortcut::activated,
            this, &MainWindow::hideRenamePlaylist);

    m_playlistStack->addWidget(m_renamePlaylistWidget);

    layout->addWidget(m_playlistStack);

    layout->addSpacing(16);

    QWidget *eqContainer = new QWidget();
    QVBoxLayout *eqBlock = new QVBoxLayout(eqContainer);
    eqBlock->setContentsMargins(0, 0, 0, 0);
    eqBlock->setSpacing(6);

    QLabel *eqTitle = new QLabel("Эквалайзер");
    eqTitle->setAlignment(Qt::AlignCenter);
    eqTitle->setStyleSheet("color: #a0a0a0; font-size: 12px; font-weight: bold;");
    eqBlock->addWidget(eqTitle);

    QWidget *eqRow = new QWidget();
    QHBoxLayout *eqLayout = new QHBoxLayout(eqRow);
    eqLayout->setContentsMargins(0, 0, 0, 0);
    eqLayout->setSpacing(2);

    static const char *const kEqLabels[10] = {
        "31", "62", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
    };

    for (int i = 0; i < 10; ++i) {
        QWidget *bandBox = new QWidget();
        QVBoxLayout *bandLayout = new QVBoxLayout(bandBox);
        bandLayout->setContentsMargins(0, 0, 0, 0);
        bandLayout->setSpacing(2);

        EqSlider *band = new EqSlider(Qt::Vertical);
        band->setRange(-12, 12);
        band->setFixedSize(20, 84);
        band->setCursor(Qt::PointingHandCursor);
        band->setToolTip(QString("%1 Гц").arg(QString::fromLatin1(kEqLabels[i])));
        // Стиль задаёт только геометрию; форма ручки рисуется в EqSlider::paintEvent.
        band->setStyleSheet(
            "QSlider::groove:vertical { width: 6px; }"
            "QSlider::handle:vertical { height: 12px; margin: 0 -3px; background: transparent; }"
            );
        QSettings eqSettings;
        band->setValue(qBound(-12, 12,
                              eqSettings.value(QString("eq/band%1").arg(i), 0).toInt()));
        connect(band, &QSlider::valueChanged, this, [i](int value) {
            QSettings settings;
            settings.setValue(QString("eq/band%1").arg(i), value);
        });
        m_eqSliders.append(band);
        bandLayout->addWidget(band, 0, Qt::AlignHCenter);

        QLabel *bandLabel = new QLabel(QString::fromLatin1(kEqLabels[i]));
        bandLabel->setAlignment(Qt::AlignCenter);
        bandLabel->setStyleSheet("color: #7a7a7a; font-size: 8px;");
        bandLayout->addWidget(bandLabel);

        eqLayout->addWidget(bandBox, 1);
    }

    eqBlock->addWidget(eqRow);
    layout->addWidget(eqContainer);
    layout->addSpacing(16);

    QWidget *volContainer = new QWidget();
    QHBoxLayout *volLayout = new QHBoxLayout(volContainer);
    volLayout->setContentsMargins(0, 0, 0, 0);
    volLayout->setSpacing(10);

    QLabel *volIcon = new QLabel();
    volIcon->setPixmap(makeSpeakerIcon(kIconGray, 18).pixmap(18, 18));
    volIcon->setFixedSize(18, 18);
    volLayout->addWidget(volIcon);

    int startVolume = 80;
#ifdef Q_OS_WIN
    const int deviceVolume = systemVolume().volume();
    if (deviceVolume >= 0)
        startVolume = deviceVolume;
#endif

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setMinimumHeight(20);
    m_volumeSlider->setValue(startVolume);
    m_volumeSlider->setCursor(Qt::PointingHandCursor);
    m_volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 6px; background: #3e3e42; border-radius: 3px; }"
        "QSlider::sub-page:horizontal { background: #c0c0c0; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 16px; margin: -5px 0;"
        " background: #f1f1f1; border-radius: 8px; }"
        );
    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &MainWindow::onVolumeChanged);
    volLayout->addWidget(m_volumeSlider, 1);

    m_volumeLabel = new QLabel(QString("%1%").arg(startVolume));
    m_volumeLabel->setFixedWidth(40);
    m_volumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_volumeLabel->setStyleSheet("color: #a0a0a0; font-size: 12px;");
    volLayout->addWidget(m_volumeLabel);

    layout->addWidget(volContainer);
    layout->addStretch();

    onVolumeChanged(startVolume);

#ifdef Q_OS_WIN
    systemVolume().setChangeCallback([this](int percent) {
        QMetaObject::invokeMethod(this, "onSystemVolumeChanged", Qt::QueuedConnection,
                                  Q_ARG(int, percent));
    });
#endif
}

void MainWindow::toggleSettings()
{
    m_settingsOpen = !m_settingsOpen;

    if (m_settingsOpen) {
        const int w = m_rightPanel->x() - 1;
        m_settingsOverlay->setGeometry(0, 0, w, m_outerWidget->height());
        m_settingsOverlay->show();
        m_settingsOverlay->raise();

        updatePlaylistActionButton();

#ifdef Q_OS_WIN
        const int deviceVolume = systemVolume().volume();
        if (deviceVolume >= 0 && m_volumeSlider) {
            m_volumeSlider->blockSignals(true);
            m_volumeSlider->setValue(deviceVolume);
            m_volumeSlider->blockSignals(false);
            if (m_volumeLabel)
                m_volumeLabel->setText(QString("%1%").arg(deviceVolume));
        }
#endif
    } else {
        m_settingsOverlay->hide();
        if (m_addToPlaylistMode)
            cancelAddToPlaylist();
        m_addToPlaylistMode = false;
        m_addTargetPlaylist.clear();
        m_addModeBackup.clear();
        hideCreatePlaylist();
        hideRenamePlaylist();
        updatePlaylistActionButton();
    }

    refreshPlaylist();
}

void MainWindow::onPlaylistActionClicked()
{
    if (m_addToPlaylistMode) {
        savePlaylist();
        m_addToPlaylistMode = false;
        m_addTargetPlaylist.clear();
        m_addModeBackup.clear();
        updatePlaylistActionButton();
        refreshPlaylist();
    } else if (m_filter == Playlist) {
        m_addToPlaylistMode = true;
        m_addTargetPlaylist = m_activePlaylist;
        m_addModeBackup.clear();
        m_addModeBackup.reserve(m_playlist.size());
        for (const Track &trk : m_playlist)
            m_addModeBackup.append(trk.playlists);
        updatePlaylistActionButton();
        refreshPlaylist();
    } else {
        showCreatePlaylist();
    }
}

void MainWindow::cancelAddToPlaylist()
{
    if (!m_addToPlaylistMode)
        return;

    for (int i = 0; i < m_playlist.size() && i < m_addModeBackup.size(); ++i)
        m_playlist[i].playlists = m_addModeBackup.at(i);

    m_addToPlaylistMode = false;
    m_addTargetPlaylist.clear();
    m_addModeBackup.clear();
    updatePlaylistActionButton();
    refreshPlaylist();
}

void MainWindow::updatePlaylistActionButton()
{
    hideDeleteConfirm();

    if (m_deletePlaylistBtn) {
        const bool canDelete = (m_filter == Playlist
                                && !m_activePlaylist.isEmpty()
                                && !m_addToPlaylistMode);
        m_deletePlaylistBtn->setVisible(canDelete);
        if (m_renamePlaylistBtn)
            m_renamePlaylistBtn->setVisible(canDelete);
    }

    if (m_addTrackButton)
        m_addTrackButton->setVisible(!m_addToPlaylistMode);
    if (m_addToPlaylistLabel)
        m_addToPlaylistLabel->setVisible(m_addToPlaylistMode);

    if (m_cancelEditBtn)
        m_cancelEditBtn->setVisible(m_addToPlaylistMode);

    m_playlistActionBtn->setEnabled(true);

    if (m_addToPlaylistMode) {
        m_playlistActionBtn->setText("  Сохранить");
        m_addToPlaylistLabel->setText("Добавление в: " + m_addTargetPlaylist);
    } else if (m_filter == Playlist) {
        m_playlistActionBtn->setText("  Добавить в плейлист");
        m_addToPlaylistLabel->clear();
    } else {
        const bool limitReached = (m_customPlaylists.size() >= 10);
        m_playlistActionBtn->setText(limitReached ? "  Лимит: 10 плейлистов"
                                                  : "  Добавить плейлист");
        m_playlistActionBtn->setEnabled(!limitReached);
        m_addToPlaylistLabel->clear();
    }
}

void MainWindow::onDeletePlaylistClicked()
{
    if (m_filter != Playlist || m_activePlaylist.isEmpty())
        return;

    m_pendingDeletePlaylist = m_activePlaylist;
    m_deleteConfirmLabel->setText(
        QString("Вы точно хотите удалить плейлист: %1?").arg(m_pendingDeletePlaylist));
    m_playlistStack->setCurrentWidget(m_deleteConfirmWidget);
}

void MainWindow::onConfirmDeletePlaylist()
{
    deleteActivePlaylist();
}

void MainWindow::hideDeleteConfirm()
{
    m_pendingDeletePlaylist.clear();
    if (m_playlistStack && !m_createPlaylist && !m_renamePlaylist)
        m_playlistStack->setCurrentWidget(m_playlistRowWidget);
}

void MainWindow::showCreatePlaylist()
{
    if (m_customPlaylists.size() >= 10)
        return;

    m_createPlaylist = true;
    hideDeleteConfirm();

    if (m_playPauseShortcut)
        m_playPauseShortcut->setEnabled(false);

    m_newPlaylistEdit->clear();
    m_playlistStack->setCurrentWidget(m_newPlaylistWidget);
    m_newPlaylistEdit->setFocus();
}

void MainWindow::hideCreatePlaylist()
{
    if (!m_createPlaylist)
        return;

    m_createPlaylist = false;
    if (m_addToPlaylistLabel)
        m_addToPlaylistLabel->clear();
    if (m_playlistStack)
        m_playlistStack->setCurrentWidget(m_playlistRowWidget);
    if (m_playPauseShortcut)
        m_playPauseShortcut->setEnabled(true);
}

void MainWindow::onCreatePlaylistConfirmed()
{
    QString name = m_newPlaylistEdit->text().trimmed();

    if (name.isEmpty()) {
        m_newPlaylistEdit->setFocus();
        return;
    }
    if (name.length() > 10)
        name = name.left(10);

    hideCreatePlaylist();
    hideRenamePlaylist();

    if (m_customPlaylists.contains(name)) {
        setFilterPlaylist(name);
        return;
    }

    if (m_customPlaylists.size() >= 10) {
        updatePlaylistActionButton();
        refreshPlaylist();
        return;
    }

    m_customPlaylists.append(name);
    savePlaylist();
    setFilterPlaylist(name);
}

void MainWindow::onRenamePlaylistClicked()
{
    showRenamePlaylist();
}

void MainWindow::showRenamePlaylist()
{
    if (m_filter != Playlist || m_activePlaylist.isEmpty())
        return;

    m_renamePlaylist = true;
    m_renamePlaylistSource = m_activePlaylist;
    hideDeleteConfirm();

    if (m_playPauseShortcut)
        m_playPauseShortcut->setEnabled(false);

    m_renamePlaylistEdit->setText(m_activePlaylist);
    m_renamePlaylistEdit->selectAll();
    m_playlistStack->setCurrentWidget(m_renamePlaylistWidget);
    m_renamePlaylistEdit->setFocus();
}

void MainWindow::hideRenamePlaylist()
{
    if (!m_renamePlaylist)
        return;

    m_renamePlaylist = false;
    m_renamePlaylistSource.clear();
    if (m_playlistStack)
        m_playlistStack->setCurrentWidget(m_playlistRowWidget);
    if (m_playPauseShortcut)
        m_playPauseShortcut->setEnabled(true);
}

void MainWindow::onRenamePlaylistConfirmed()
{
    QString newName = m_renamePlaylistEdit->text().trimmed();

    if (newName.isEmpty()) {
        m_renamePlaylistEdit->setFocus();
        return;
    }
    if (newName.length() > 10)
        newName = newName.left(10);

    const QString oldName = m_renamePlaylistSource;
    if (oldName.isEmpty() || newName == oldName) {
        hideRenamePlaylist();
        return;
    }

    if (m_customPlaylists.contains(newName)) {
        m_renamePlaylistEdit->setFocus();
        return;
    }

    const int index = m_customPlaylists.indexOf(oldName);
    if (index >= 0)
        m_customPlaylists[index] = newName;

    for (Track &trk : m_playlist) {
        if (trk.playlists.contains(oldName)) {
            trk.playlists.removeAll(oldName);
            trk.playlists.append(newName);
        }
    }

    if (m_activePlaylist == oldName)
        m_activePlaylist = newName;
    if (m_addTargetPlaylist == oldName)
        m_addTargetPlaylist = newName;

    hideRenamePlaylist();
    savePlaylist();
    updatePlaylistButtons();
    updatePlaylistActionButton();
    refreshPlaylist();
}

void MainWindow::deleteActivePlaylist()
{
    const QString name = m_pendingDeletePlaylist;
    if (name.isEmpty() || !m_customPlaylists.contains(name))
        return;

    m_customPlaylists.removeAll(name);
    for (Track &trk : m_playlist)
        trk.playlists.removeAll(name);

    if (m_addTargetPlaylist == name) {
        m_addToPlaylistMode = false;
        m_addTargetPlaylist.clear();
    }

    const bool wasOpen = (m_filter == Playlist && m_activePlaylist == name);

    hideDeleteConfirm();
    savePlaylist();

    if (wasOpen) {
        setFilterAll();
    } else {
        updatePlaylistButtons();
        updatePlaylistActionButton();
        refreshPlaylist();
    }
}

void MainWindow::updatePlaylistButtons()
{
    QLayoutItem *child;
    while ((child = m_filterButtonsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    const QString baseStyle =
        "QPushButton { "
        "  border: none; "
        "  border-radius: 14px; "
        "  font-size: 12px; "
        "  padding: 0px 12px; "
        "  outline: none; "
        "}";

    const QString activeStyle = baseStyle +
                                "QPushButton { background-color: rgba(255,255,255,0.15); color: #f1f1f1; } "
                                "QPushButton:hover { background-color: rgba(255,255,255,0.20); }";

    const QString idleStyle = baseStyle +
                              "QPushButton { background-color: rgba(255,255,255,0.03); color: #a0a0a0; } "
                              "QPushButton:hover { color: #f1f1f1; background-color: rgba(255,255,255,0.08); }";

    int chipsWidth = 0;
    for (const QString &name : m_customPlaylists) {
        QPushButton *btn = new QPushButton(name);
        btn->setFixedHeight(28);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet((m_filter == Playlist && m_activePlaylist == name)
                               ? activeStyle : idleStyle);
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        btn->ensurePolished();
        const int w = qMax(50, btn->sizeHint().width());
        btn->setFixedWidth(w);
        connect(btn, &QPushButton::clicked, this, [this, name]() {
            setFilterPlaylist(name);
        });
        m_filterButtonsLayout->addWidget(btn);
        chipsWidth += w;
    }
    if (!m_customPlaylists.isEmpty())
        chipsWidth += m_filterButtonsLayout->spacing() * (m_customPlaylists.size() - 1);

    m_filterButtonsLayout->addStretch();
    m_filterButtonsWidget->setFixedWidth(chipsWidth);
    m_filterButtonsWidget->setVisible(!m_customPlaylists.isEmpty());
}

void MainWindow::toggleTrackInPlaylist(int index)
{
    const QString target = m_addToPlaylistMode ? m_addTargetPlaylist
                                               : m_activePlaylist;

    if (index < 0 || index >= m_playlist.size() || target.isEmpty())
        return;

    Track &trk = m_playlist[index];
    if (trk.playlists.contains(target)) {
        trk.playlists.removeAll(target);
    } else {
        trk.playlists.append(target);
    }

    if (m_addToPlaylistMode) {
        refreshPlaylist();
        return;
    }

    savePlaylist();
    refreshPlaylist();
}

void MainWindow::setFilterAll()
{
    hideCreatePlaylist();
    hideRenamePlaylist();

    if (m_filter == All && !m_addToPlaylistMode)
        return;

    m_filter = All;
    m_activePlaylist.clear();

    const QString baseStyle =
        "QPushButton { border: none; border-radius: 14px; font-size: 12px; padding: 0px 12px; outline: none; }";
    const QString activeStyle = baseStyle + "QPushButton { background-color: rgba(255,255,255,0.15); color: #f1f1f1; } QPushButton:hover { background-color: rgba(255,255,255,0.20); }";
    const QString idleStyle = baseStyle + "QPushButton { background-color: rgba(255,255,255,0.03); color: #a0a0a0; } QPushButton:hover { color: #f1f1f1; background-color: rgba(255,255,255,0.08); }";

    m_allFilterBtn->setStyleSheet(activeStyle);
    m_favFilterBtn->setStyleSheet(idleStyle);
    updatePlaylistButtons();
    updatePlaylistActionButton();

    refreshPlaylist();
}

void MainWindow::setFilterFavorites()
{
    hideCreatePlaylist();
    hideRenamePlaylist();

    if (m_filter == Favorites && !m_addToPlaylistMode)
        return;

    m_filter = Favorites;
    m_activePlaylist.clear();

    const QString baseStyle =
        "QPushButton { border: none; border-radius: 14px; font-size: 12px; padding: 0px 12px; outline: none; }";
    const QString activeStyle = baseStyle + "QPushButton { background-color: rgba(255,255,255,0.15); color: #f1f1f1; } QPushButton:hover { background-color: rgba(255,255,255,0.20); }";
    const QString idleStyle = baseStyle + "QPushButton { background-color: rgba(255,255,255,0.03); color: #a0a0a0; } QPushButton:hover { color: #f1f1f1; background-color: rgba(255,255,255,0.08); }";

    m_favFilterBtn->setStyleSheet(activeStyle);
    m_allFilterBtn->setStyleSheet(idleStyle);
    updatePlaylistButtons();
    updatePlaylistActionButton();

    refreshPlaylist();
}

void MainWindow::setFilterPlaylist(const QString &name)
{
    hideCreatePlaylist();
    hideRenamePlaylist();

    m_filter = Playlist;
    m_activePlaylist = name;

    const QString baseStyle =
        "QPushButton { border: none; border-radius: 14px; font-size: 12px; padding: 0px 12px; outline: none; }";
    const QString idleStyle = baseStyle + "QPushButton { background-color: rgba(255,255,255,0.03); color: #a0a0a0; } QPushButton:hover { color: #f1f1f1; background-color: rgba(255,255,255,0.08); }";

    m_allFilterBtn->setStyleSheet(idleStyle);
    m_favFilterBtn->setStyleSheet(idleStyle);
    updatePlaylistButtons();
    updatePlaylistActionButton();

    refreshPlaylist();
}

void MainWindow::refreshPlaylist()
{
    m_listWidget->clear();

    QVector<int> order = playbackOrder();
    m_countLabel->setText(QString("(%1)").arg(order.size()));

    if (order.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        QLabel *msg = new QLabel("Здесь пока пусто");
        msg->setAlignment(Qt::AlignCenter);
        msg->setStyleSheet("color: #a0a0a0; font-size: 13px; background: transparent;");
        item->setSizeHint(QSize(0, 80));
        m_listWidget->setItemWidget(item, msg);
        return;
    }

    for (int origIdx : order) {
        const Track &trk = m_playlist.at(origIdx);

        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setData(Qt::UserRole, origIdx);

        QWidget *row = new QWidget();
        row->setStyleSheet("background: transparent;");

        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(10, 4, 10, 4);
        rowLayout->setSpacing(6);

        const QString displayName = trk.title.isEmpty() ? trk.name : trk.title;
        QLabel *title = new QLabel(displayName);

        if (!trk.artist.isEmpty()) {
            title->setText(QString("%1 — %2").arg(trk.artist, displayName));
        }

        title->setStyleSheet("color: #f1f1f1; font-size: 13px; background: transparent;");
        title->setToolTip(displayName);
        title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        title->setMinimumWidth(0);
        title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        rowLayout->addWidget(title, 1, Qt::AlignVCenter);

        QLabel *dur = new QLabel(formatTime(trk.durationMs));
        dur->setStyleSheet("color: #a0a0a0; font-size: 11px; background: transparent;");
        dur->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        dur->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowLayout->addWidget(dur, 0, Qt::AlignVCenter);

        if (!m_settingsOpen && (m_filter == Favorites || m_filter == Playlist)) {
            QWidget *spacer = new QWidget();
            spacer->setFixedSize(26, 26);
            rowLayout->addWidget(spacer, 0, Qt::AlignVCenter);
        } else {
            QPushButton *actionBtn = new QPushButton();
            actionBtn->setFixedSize(26, 26);
            actionBtn->setIconSize(QSize(16, 16));
            actionBtn->setCursor(Qt::PointingHandCursor);

            actionBtn->setStyleSheet(
                "QPushButton { background: transparent; border: none; outline: none; border-radius: 13px; }"
                "QPushButton:hover { background: rgba(255,255,255,0.08); border-radius: 13px; outline: none; }"
                "QPushButton:pressed { background: rgba(255,255,255,0.15); border-radius: 13px; outline: none; }"
                );

            if (m_settingsOpen) {
                if (m_addToPlaylistMode) {
                    const bool inTarget = trk.playlists.contains(m_addTargetPlaylist);
                    actionBtn->setIcon(makeCircleIcon(inTarget, kWhite, 16));
                    actionBtn->setToolTip(
                        inTarget ? QString("Убрать из плейлиста «%1»").arg(m_addTargetPlaylist)
                                 : QString("Добавить в плейлист «%1»").arg(m_addTargetPlaylist));
                    connect(actionBtn, &QPushButton::clicked, this, [this, origIdx]() {
                        toggleTrackInPlaylist(origIdx);
                    });
                } else {
                    actionBtn->setIcon(makeSmallCloseIcon(kIconGray, 14));
                    if (m_filter == All) {
                        actionBtn->setToolTip("Удалить трек полностью");
                        connect(actionBtn, &QPushButton::clicked, this, [this, origIdx]() {
                            removeTrack(origIdx);
                        });
                    } else if (m_filter == Favorites) {
                        actionBtn->setToolTip("Убрать из избранного");
                        connect(actionBtn, &QPushButton::clicked, this, [this, origIdx]() {
                            toggleFavorite(origIdx);
                        });
                    } else if (m_filter == Playlist) {
                        actionBtn->setToolTip("Удалить из плейлиста");
                        connect(actionBtn, &QPushButton::clicked, this, [this, origIdx]() {
                            toggleTrackInPlaylist(origIdx);
                        });
                    }
                }
            } else {
                actionBtn->setIcon(trk.favorite ? makeHeartIcon(true, kRed, 16)
                                                : makeHeartIcon(false, kIconGray, 16));
                actionBtn->setToolTip(trk.favorite ? "Убрать из избранного" : "В избранное");
                connect(actionBtn, &QPushButton::clicked, this, [this, origIdx]() {
                    toggleFavorite(origIdx);
                });
            }
            rowLayout->addWidget(actionBtn, 0, Qt::AlignVCenter);
        }

        item->setSizeHint(row->sizeHint());
        m_listWidget->setItemWidget(item, row);

        if (origIdx == m_currentIndex)
            item->setSelected(true);
    }

    if (m_currentIndex >= 0) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            QListWidgetItem *it = m_listWidget->item(i);
            if (it && it->data(Qt::UserRole).toInt() == m_currentIndex) {
                m_listWidget->scrollToItem(it, QAbstractItemView::EnsureVisible);
                break;
            }
        }
    }
}

QVector<int> MainWindow::playbackOrder() const
{
    QVector<int> order;
    for (int i = 0; i < m_playlist.size(); ++i) {
        if (m_filter == All) {
            order.append(i);
        } else if (m_filter == Favorites) {
            if (m_playlist.at(i).favorite) order.append(i);
        } else if (m_filter == Playlist) {
            if (m_playlist.at(i).playlists.contains(m_activePlaylist)) order.append(i);
        }
    }
    return order;
}

void MainWindow::addTracks()
{
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Выберите музыкальные файлы",
        musicFolder(),
        "Аудио файлы (*.mp3 *.wav *.ogg *.flac *.m4a);;Все файлы (*)"
        );

    const QString folder = musicFolder();
    QDir().mkpath(folder);

    bool added = false;
    int insertIndex = 0;

    for (const QString &sourcePath : files) {
        QString path = sourcePath;

        const bool insideFolder = path.startsWith(folder + "/", Qt::CaseInsensitive)
                                  || path.startsWith(folder + "\\", Qt::CaseInsensitive);
        if (!insideFolder) {
            const QFileInfo sourceInfo(path);
            QString destination = folder + "/" + sourceInfo.fileName();

            if (QFileInfo::exists(destination)
                && QFileInfo(destination).size() == sourceInfo.size()) {
                path = destination;
            } else {
                int suffix = 1;
                while (QFileInfo::exists(destination)) {
                    destination = folder + "/" + sourceInfo.completeBaseName()
                                  + QString(" (%1).").arg(suffix++) + sourceInfo.suffix();
                }
                if (QFile::copy(path, destination))
                    path = destination;
            }
        }

        const bool exists = std::any_of(
            m_playlist.cbegin(), m_playlist.cend(),
            [&path](const Track &t) { return t.filePath == path; });
        if (exists)
            continue;

        Track trk;
        trk.name = QFileInfo(path).completeBaseName();
        trk.filePath = path;

        m_playlist.insert(insertIndex++, trk);
        added = true;

        if (m_currentIndex >= 0) {
            m_currentIndex++;
        }
    }

    if (!added)
        return;

    savePlaylist();

    if (m_currentIndex == -1 && !m_playlist.isEmpty()) {
        loadTrack(0);
    } else {
        refreshPlaylist();
    }
}

void MainWindow::loadTrack(int index)
{
    if (index < 0 || index >= m_playlist.size())
        return;

    if (m_editOverlay && m_editOverlay->isVisible())
        closeTrackEditor();

    m_player->stop();

    m_currentIndex = index;
    const Track &trk = m_playlist.at(index);

    QSettings settings;
    settings.setValue("lastTrackPath", trk.filePath);

    m_player->setSource(QUrl::fromLocalFile(trk.filePath));

    m_artistLabel->setText(trk.artist.isEmpty() ? "Загрузка..." : trk.artist);

    updateTitleText();
    updateCover();

    m_totalTime->setText(formatTime(trk.durationMs));
    m_currentTime->setText("0:00");

    if (m_progressSlider) {
        m_progressSlider->setRange(0, static_cast<int>(trk.durationMs));
        m_progressSlider->setValue(0);
    }

    updateHeartButton();
    updatePlayButton();
    refreshPlaylist();
}

void MainWindow::playTrack()
{
    if (m_currentIndex == -1 && !m_playlist.isEmpty())
        loadTrack(0);

    if (m_currentIndex == -1)
        return;

    m_player->play();
}

void MainWindow::pauseTrack()
{
    m_player->pause();
}

void MainWindow::stopTrack()
{
    m_player->stop();
    m_currentTime->setText("0:00");
    if (m_progressSlider)
        m_progressSlider->setValue(0);
}

void MainWindow::togglePlay()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
        pauseTrack();
    else
        playTrack();
}

void MainWindow::playNext()
{
    if (m_playlist.isEmpty())
        return;

    const QVector<int> order = playbackOrder();
    if (order.isEmpty())
        return;

    if (m_playMode == Shuffle && order.size() > 1) {
        int pick = m_currentIndex;
        while (pick == m_currentIndex)
            pick = order.at(QRandomGenerator::global()->bounded(order.size()));

        loadTrack(pick);
        playTrack();
        return;
    }

    int pos = order.indexOf(m_currentIndex);
    pos = (pos == -1 || pos == order.size() - 1) ? 0 : pos + 1;

    loadTrack(order.at(pos));
    playTrack();
}

void MainWindow::playPrevious()
{
    if (m_playlist.isEmpty())
        return;

    const QVector<int> order = playbackOrder();
    if (order.isEmpty())
        return;

    if (m_playMode == Shuffle && order.size() > 1) {
        int pick = m_currentIndex;
        while (pick == m_currentIndex)
            pick = order.at(QRandomGenerator::global()->bounded(order.size()));

        loadTrack(pick);
        playTrack();
        return;
    }

    int pos = order.indexOf(m_currentIndex);
    pos = (pos <= 0) ? order.size() - 1 : pos - 1;

    loadTrack(order.at(pos));
    playTrack();
}

void MainWindow::onPositionChanged(qint64 position)
{
    if (m_progressSlider && !m_progressSlider->isSliderDown()) {
        m_progressSlider->blockSignals(true);
        m_progressSlider->setValue(static_cast<int>(position));
        m_progressSlider->blockSignals(false);
    }
    m_currentTime->setText(formatTime(position));
}

void MainWindow::onDurationChanged(qint64 duration)
{
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        const qint64 old = m_playlist[m_currentIndex].durationMs;
        m_playlist[m_currentIndex].durationMs = duration;
        if (qAbs(duration - old) > 1000)
            savePlaylist();
    }
    m_totalTime->setText(formatTime(duration));
    if (m_progressSlider)
        m_progressSlider->setRange(0, static_cast<int>(duration));
}

void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status != QMediaPlayer::EndOfMedia)
        return;

    if (m_playMode == RepeatOne && m_currentIndex >= 0) {
        const int idx = m_currentIndex;
        loadTrack(idx);
        playTrack();
        return;
    }

    playNext();
}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    Q_UNUSED(state);
    updatePlayButton();
    updateHeartButton();
}

void MainWindow::onMetaDataChanged()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        return;

    Track &trk = m_playlist[m_currentIndex];
    const QMediaMetaData md = m_player->metaData();

    const QString metaTitle = md.value(QMediaMetaData::Title).toString();
    const QString metaArtist = md.value(QMediaMetaData::ContributingArtist).toString();

    bool changed = false;

    if (!metaTitle.isEmpty() && trk.title != metaTitle) {
        trk.title = metaTitle;
        changed = true;
    }
    if (!metaArtist.isEmpty() && trk.artist != metaArtist) {
        trk.artist = metaArtist;
        changed = true;
    }

    const QVariant coverData = md.value(QMediaMetaData::CoverArtImage);
    if (coverData.isValid() && !coverData.value<QImage>().isNull()) {
        trk.cover = QPixmap::fromImage(coverData.value<QImage>());
        changed = true;
    } else {
        const QVariant thumbData = md.value(QMediaMetaData::ThumbnailImage);
        if (thumbData.isValid() && !thumbData.value<QImage>().isNull()) {
            trk.cover = QPixmap::fromImage(thumbData.value<QImage>());
            changed = true;
        }
    }

    if (changed) {
        updateTitleText();
        m_artistLabel->setText(trk.artist.isEmpty() ? "Неизвестный исполнитель" : trk.artist);
        updateCover();
        refreshPlaylist();
        savePlaylist();
    }
}

void MainWindow::updateCover()
{
    if (!m_coverWidget) return;

    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        m_currentCover = m_playlist.at(m_currentIndex).cover;
    } else {
        m_currentCover = QPixmap();
    }

    static_cast<CoverWidget*>(m_coverWidget)->cover = m_currentCover;
    m_coverWidget->update();
}

void MainWindow::onSeekClicked(int pos)
{
    m_player->setPosition(static_cast<qint64>(pos));
}

void MainWindow::toggleFavoriteCurrent()
{
    if (m_currentIndex >= 0)
        toggleFavorite(m_currentIndex);
}

void MainWindow::toggleFavorite(int index)
{
    if (index < 0 || index >= m_playlist.size())
        return;

    m_playlist[index].favorite = !m_playlist[index].favorite;
    savePlaylist();
    updateHeartButton();
    refreshPlaylist();
}

void MainWindow::updateHeartButton()
{
    if (!m_heartButton)
        return;

    const bool fav = (m_currentIndex >= 0 &&
                      m_currentIndex < m_playlist.size() &&
                      m_playlist.at(m_currentIndex).favorite);

    m_heartButton->setIcon(fav ? makeHeartIcon(true, kRed, 20)
                               : makeHeartIcon(false, kIconGray, 20));
    m_heartButton->setToolTip(fav ? "Убрать из избранного" : "В избранное");
}

void MainWindow::updatePlayButton()
{
    if (!m_playButton)
        return;

    const bool playing = (m_player->playbackState() == QMediaPlayer::PlayingState);
    m_playButton->setIcon(playing ? makePauseIcon(kWhite, 24)
                                  : makePlayIcon(kWhite, 24));
}

void MainWindow::updateTitleText()
{
    if (!m_titleLabel)
        return;

    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size()) {
        m_titleLabel->setText("Нет трека");
        m_titleLabel->setToolTip(QString());
        return;
    }

    const Track &trk = m_playlist.at(m_currentIndex);
    const QString full = trk.title.isEmpty() ? trk.name : trk.title;

    const QFontMetrics fm(m_titleLabel->font());
    const int avail = qMax(40, m_titleLabel->width() - 8);

    m_titleLabel->setText(fm.elidedText(full, Qt::ElideRight, avail));
    m_titleLabel->setToolTip(full);
}

void MainWindow::removeTrack(int index)
{
    if (index < 0 || index >= m_playlist.size())
        return;

    const bool wasCurrent = (index == m_currentIndex);
    m_playlist.removeAt(index);

    if (wasCurrent) {
        stopTrack();
        if (m_playlist.isEmpty()) {
            m_currentIndex = -1;
            m_titleLabel->setText("Нет трека");
            m_artistLabel->setText("Добавьте музыку");
            m_totalTime->setText("0:00");
            m_currentTime->setText("0:00");
            if (m_progressSlider)
                m_progressSlider->setRange(0, 0);
            updateHeartButton();
            updatePlayButton();
            updateCover();
        } else {
            m_currentIndex = qMin(index, static_cast<int>(m_playlist.size()) - 1);
            loadTrack(m_currentIndex);
        }
    } else if (m_currentIndex > index) {
        --m_currentIndex;
    }

    savePlaylist();
    refreshPlaylist();
}

void MainWindow::onPlaylistItemClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    const int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= m_playlist.size())
        return;

    loadTrack(idx);
    playTrack();
}

void MainWindow::onVolumeChanged(int value)
{
#ifdef Q_OS_WIN
    if (systemVolume().isValid()) {
        systemVolume().setVolume(value);
        if (m_audioOut)
            m_audioOut->setVolume(1.0);
    } else
#endif
    if (m_audioOut) {
        m_audioOut->setVolume(value / 100.0);
    }

    if (m_volumeLabel)
        m_volumeLabel->setText(QString("%1%").arg(value));
}

void MainWindow::onSystemVolumeChanged(int percent)
{
    if (!m_volumeSlider || m_volumeSlider->isSliderDown())
        return;

    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(percent);
    m_volumeSlider->blockSignals(false);

    if (m_volumeLabel)
        m_volumeLabel->setText(QString("%1%").arg(percent));
}

void MainWindow::setupEditOverlay()
{
    m_editOverlay = new QWidget(m_outerWidget);
    m_editOverlay->setObjectName("editOverlay");
    m_editOverlay->setStyleSheet(
        "#editOverlay { background-color: #2d2d30;"
        " border-bottom-left-radius: 12px;"
        " border-bottom-right-radius: 0px; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(m_editOverlay);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    QWidget *headerRow = new QWidget();
    headerRow->setFixedHeight(34);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *closeBtn = new QPushButton();
    closeBtn->setFixedSize(34, 34);
    closeBtn->setIcon(makeCloseIcon(kIconGray, 16));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; border-radius: 17px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); }"
        );
    closeBtn->setToolTip("Закрыть");
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::closeTrackEditor);
    headerLayout->addWidget(closeBtn);
    headerLayout->addStretch();
    layout->addWidget(headerRow);

    m_editCoverPreview = new QLabel();
    m_editCoverPreview->setFixedSize(140, 140);
    m_editCoverPreview->setAlignment(Qt::AlignCenter);
    m_editCoverPreview->setStyleSheet(
        "background-color: #1e1e1e; border-radius: 12px;");
    layout->addWidget(m_editCoverPreview, 0, Qt::AlignHCenter);

    QPushButton *pickCoverBtn = new QPushButton("  Выбрать обложку");
    pickCoverBtn->setFixedHeight(34);
    pickCoverBtn->setCursor(Qt::PointingHandCursor);
    pickCoverBtn->setStyleSheet(
        "QPushButton { background-color: #3e3e42; color: #f1f1f1; border: none;"
        " border-radius: 8px; font-size: 12px; }"
        "QPushButton:hover { background-color: #4d4d4d; }"
        );
    connect(pickCoverBtn, &QPushButton::clicked,
            this, &MainWindow::pickTrackCover);
    layout->addWidget(pickCoverBtn);

    QLabel *titleCaption = new QLabel("Название");
    titleCaption->setStyleSheet("color: #a0a0a0; font-size: 11px; font-weight: bold;");
    layout->addWidget(titleCaption);

    m_trackTitleEdit = new QLineEdit();
    m_trackTitleEdit->setFixedHeight(32);
    m_trackTitleEdit->setStyleSheet(
        "QLineEdit { background-color: #1e1e1e; color: #f1f1f1;"
        " border: 1px solid #3e3e42; border-radius: 8px; padding: 0 8px; font-size: 12px; }"
        "QLineEdit:focus { border: 1px solid #5e5e5e; }"
        );
    layout->addWidget(m_trackTitleEdit);

    QLabel *artistCaption = new QLabel("Исполнитель");
    artistCaption->setStyleSheet("color: #a0a0a0; font-size: 11px; font-weight: bold;");
    layout->addWidget(artistCaption);

    m_trackArtistEdit = new QLineEdit();
    m_trackArtistEdit->setFixedHeight(32);
    m_trackArtistEdit->setStyleSheet(m_trackTitleEdit->styleSheet());
    layout->addWidget(m_trackArtistEdit);

    layout->addStretch();

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(8);

    QPushButton *saveBtn = new QPushButton("Сохранить");
    saveBtn->setFixedHeight(40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #4d4d4d, stop:1 #3e3e42); color: #f1f1f1; border: none;"
        " border-radius: 10px; font-size: 13px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #5e5e5e, stop:1 #4d4d4d); }"
        );
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::applyTrackEditor);
    buttons->addWidget(saveBtn, 1);

    QPushButton *cancelBtn = new QPushButton("Отмена");
    cancelBtn->setFixedHeight(40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #3e3e42; color: #f1f1f1; border: none;"
        " border-radius: 10px; font-size: 13px; }"
        "QPushButton:hover { background-color: #4d4d4d; }"
        );
    connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::closeTrackEditor);
    buttons->addWidget(cancelBtn, 1);

    layout->addLayout(buttons);

    m_editOverlay->hide();
}

void MainWindow::openTrackEditor()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        return;

    m_editTrackIndex = m_currentIndex;
    const Track &trk = m_playlist.at(m_editTrackIndex);

    m_trackTitleEdit->setText(trk.title.isEmpty() ? trk.name : trk.title);
    m_trackArtistEdit->setText(trk.artist);
    m_editCoverData.clear();

    if (trk.cover.isNull()) {
        m_editCoverPreview->clear();
    } else {
        m_editCoverPreview->setPixmap(trk.cover.scaled(
            140, 140, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    const int w = m_rightPanel->x() - 1;
    m_editOverlay->setGeometry(0, 0, w, m_outerWidget->height());
    m_editOverlay->show();
    m_editOverlay->raise();
    m_trackTitleEdit->setFocus();
}

void MainWindow::closeTrackEditor()
{
    if (m_editOverlay)
        m_editOverlay->hide();
    m_editTrackIndex = -1;
    m_editCoverData.clear();
}

void MainWindow::pickTrackCover()
{
    const QString file = QFileDialog::getOpenFileName(
        this, "Обложка", QString(),
        "Изображения (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (file.isEmpty())
        return;

    QImage image(file);
    if (image.isNull())
        return;

    image = image.scaled(800, 800, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_editCoverData.clear();
    QBuffer buffer(&m_editCoverData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG", 90);
    buffer.close();

    m_editCoverPreview->setPixmap(QPixmap::fromImage(image).scaled(
        140, 140, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void MainWindow::applyTrackEditor()
{
    if (m_editTrackIndex < 0 || m_editTrackIndex >= m_playlist.size()) {
        closeTrackEditor();
        return;
    }

    Track &trk = m_playlist[m_editTrackIndex];
    const QString newTitle = m_trackTitleEdit->text().trimmed();
    const QString newArtist = m_trackArtistEdit->text().trimmed();
    const QByteArray newCover = m_editCoverData;
    const bool isMp3 = trk.filePath.endsWith(".mp3", Qt::CaseInsensitive);
    const bool isM4a = trk.filePath.endsWith(".m4a", Qt::CaseInsensitive) ||
                       trk.filePath.endsWith(".mp4", Qt::CaseInsensitive);

    bool fileUpdated = false;
    if (isMp3 || isM4a) {
        const bool wasPlaying = (m_player->playbackState() != QMediaPlayer::StoppedState);
        m_player->stop();
        m_player->setSource(QUrl());
        fileUpdated = isMp3
                          ? writeMp3Tags(trk.filePath, newTitle, newArtist, newCover)
                          : writeM4aTags(trk.filePath, newTitle, newArtist, newCover);
        m_player->setSource(QUrl::fromLocalFile(trk.filePath));
        if (wasPlaying)
            m_player->play();
    } else {
        QMessageBox::information(this, "Редактировать трек",
                                 "Запись тегов поддерживается для MP3 и M4A.\n"
                                 "Изменения сохранены только в библиотеке.");
    }

    trk.title = newTitle;
    trk.artist = newArtist;
    if (!newCover.isEmpty()) {
        QImage image;
        if (image.loadFromData(newCover, "JPEG"))
            trk.cover = QPixmap::fromImage(image);
    }

    closeTrackEditor();

    m_artistLabel->setText(trk.artist.isEmpty() ? "Неизвестный исполнитель" : trk.artist);
    updateTitleText();
    updateCover();
    refreshPlaylist();
    savePlaylist();

    if ((isMp3 || isM4a) && !fileUpdated) {
        QMessageBox::warning(this, "Редактировать трек",
                             "Не удалось записать теги в файл.\n"
                             "Библиотека обновлена, файл не изменён.");
    }
}

void MainWindow::onMinimizeWindow()
{
    showMinimized();
}

void MainWindow::onCloseWindow()
{
    close();
}

void MainWindow::savePlaylist() const
{
    const QString path = storagePath();
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning("savePlaylist: cannot open %s", qPrintable(path));
        return;
    }

    QJsonObject rootObj;
    rootObj["playlists"] = QJsonArray::fromStringList(m_customPlaylists);

    QJsonArray arr;
    for (const Track &trk : m_playlist)
        arr.append(trk.toJson());
    rootObj["tracks"] = arr;

    const QByteArray data = QJsonDocument(rootObj).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size())
        qWarning("savePlaylist: short write to %s", qPrintable(path));
}

void MainWindow::loadPlaylist()
{
    QFile file(storagePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    m_playlist.clear();
    m_customPlaylists.clear();

    if (doc.isArray()) {
        QSet<QString> allPlaylists;
        for (const QJsonValue &value : doc.array()) {
            Track t = Track::fromJson(value.toObject());
            m_playlist.append(t);
            for (const QString &p : t.playlists) {
                allPlaylists.insert(p);
            }
        }
        m_customPlaylists = allPlaylists.values();
    } else if (doc.isObject()) {
        QJsonObject rootObj = doc.object();
        m_customPlaylists = rootObj["playlists"].toVariant().toStringList();
        for (const QJsonValue &value : rootObj["tracks"].toArray()) {
            m_playlist.append(Track::fromJson(value.toObject()));
        }
    }
}

QString MainWindow::storagePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
    + "/playlist.json";
}

QString MainWindow::musicFolder() const
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (base.isEmpty())
        base = QDir::homePath() + "/Music";
    return base + "/Sonora";
}

void MainWindow::scanMusicFolder()
{
    const QString folder = musicFolder();
    QDir dir(folder);
    if (!dir.exists() && !dir.mkpath("."))
        return;

    const QStringList nameFilters = {"*.mp3", "*.wav", "*.ogg", "*.flac", "*.m4a"};
    const QFileInfoList entries = dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    bool added = false;
    for (const QFileInfo &entry : entries) {
        const QString path = entry.absoluteFilePath();
        const bool exists = std::any_of(
            m_playlist.cbegin(), m_playlist.cend(),
            [&path](const Track &t) { return t.filePath == path; });
        if (exists)
            continue;

        Track trk;
        trk.name = entry.completeBaseName();
        trk.filePath = path;
        m_playlist.append(trk);
        added = true;
    }

    if (added) {
        savePlaylist();
        refreshPlaylist();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (isMaximized())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#1e1e1e"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_settingsOverlay && m_settingsOpen) {
        const int w = m_rightPanel->x() - 1;
        m_settingsOverlay->setGeometry(0, 0, w, m_outerWidget->height());
    }

    if (m_editOverlay && m_editOverlay->isVisible()) {
        const int w = m_rightPanel->x() - 1;
        m_editOverlay->setGeometry(0, 0, w, m_outerWidget->height());
    }

    updateTitleText();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->position().y() < 25
        && !isMaximized()) {
        m_dragging = true;
        m_dragOffset = event->pos();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
        move(event->globalPosition().toPoint() - m_dragOffset);
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QWidget *fw = focusWidget();

    if (qobject_cast<QAbstractSlider *>(fw)) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (qobject_cast<QListWidget *>(fw)) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
        m_player->setPosition(qMax<qint64>(0, m_player->position() - 5000));
        break;
    case Qt::Key_Right:
        m_player->setPosition(m_player->position() + 5000);
        break;
    case Qt::Key_Escape:
        if (m_editOverlay && m_editOverlay->isVisible())
            closeTrackEditor();
        else if (m_settingsOpen)
            toggleSettings();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_addToPlaylistMode)
        cancelAddToPlaylist();
    savePlaylist();
    event->accept();
}

QString MainWindow::formatTime(qint64 ms)
{
    if (ms <= 0)
        return "0:00";
    const qint64 totalSec = ms / 1000;
    const qint64 mins = totalSec / 60;
    const qint64 secs = totalSec % 60;
    return QString("%1:%2").arg(mins).arg(secs, 2, 10, QChar('0'));
}

void MainWindow::cyclePlaybackMode()
{
    switch (m_playMode) {
    case Sequential: m_playMode = Shuffle;    break;
    case Shuffle:    m_playMode = RepeatOne;  break;
    case RepeatOne:  m_playMode = Sequential; break;
    }

    updateModeButton();
}

void MainWindow::updateModeButton()
{
    switch (m_playMode) {
    case Sequential:
        m_modeButton->setIcon(makeRepeatIcon(kIconGray, 20));
        m_modeButton->setToolTip("По порядку");
        break;

    case Shuffle:
        m_modeButton->setIcon(makeShuffleIcon(kIconGray, 20));
        m_modeButton->setToolTip("Случайный порядок");
        break;

    case RepeatOne:
        m_modeButton->setIcon(makeRepeatOneIcon(kIconGray, 20));
        m_modeButton->setToolTip("Повтор одного трека");
        break;
    }
}

Track *MainWindow::currentTrack()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        return nullptr;
    return &m_playlist[m_currentIndex];
}