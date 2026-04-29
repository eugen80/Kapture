#include "CaptureOverlay.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <Qt>

CaptureOverlay::CaptureOverlay(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        m_screenPixmap = screen->grabWindow(0);
    }
}

void CaptureOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawPixmap(rect(), m_screenPixmap);
}
