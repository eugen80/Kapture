#pragma once

#include <QPixmap>
#include <QWidget>

class CaptureOverlay : public QWidget
{
public:
    explicit CaptureOverlay(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_screenPixmap;
};
