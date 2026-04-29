#pragma once

#include <QPointF>
#include <QWidget>

class QGraphicsLineItem;
class QGraphicsPolygonItem;
class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsView;

class CaptureOverlay : public QWidget
{
public:
    enum class Tool {
        Rectangle,
        Arrow,
        Text
    };

    explicit CaptureOverlay(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QPointF m_startPoint;
    Tool m_currentTool = Tool::Rectangle;
    QGraphicsRectItem *m_currentRect = nullptr;
    QGraphicsLineItem *m_currentArrowLine = nullptr;
    QGraphicsPolygonItem *m_currentArrowHead = nullptr;

    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
};
