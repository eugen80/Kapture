#include "CaptureOverlay.h"

#include <QApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <cmath>
#include <QEvent>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QPolygonF>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QScreen>
#include <Qt>
#include <QImage>
#include <QPainter>
#include <QClipboard>
#include <QFileDialog>
#include <QDebug>

CaptureOverlay::CaptureOverlay(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    m_scene = new QGraphicsScene(this);
    m_scene->installEventFilter(this);
    m_view = new QGraphicsView(m_scene, this);

    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QPixmap screenPixmap = screen->grabWindow(0);
        m_scene->addPixmap(screenPixmap);
        m_scene->setSceneRect(screenPixmap.rect());
    }
}

void CaptureOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        m_scene->clearFocus();

        QRectF sceneRect = m_scene->sceneRect();
        QImage image(sceneRect.size().toSize(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        m_scene->render(&painter);
        painter.end();

        QGuiApplication::clipboard()->setImage(image);

        QString fileName = QFileDialog::getSaveFileName(this, "Bild speichern", "screenshot.png", "Images (*.png)");
        if (!fileName.isEmpty()) {
            if (image.save(fileName)) {
                qDebug() << "Bild erfolgreich gespeichert unter:" << fileName;
            }
        }

        QApplication::quit();
    } else if (event->key() == Qt::Key_R) {
        m_currentTool = Tool::Rectangle;
    } else if (event->key() == Qt::Key_A) {
        m_currentTool = Tool::Arrow;
    } else if (event->key() == Qt::Key_T) {
        m_currentTool = Tool::Text;
    } else {
        QWidget::keyPressEvent(event);
    }
}

void CaptureOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QApplication::quit();
    } else {
        QWidget::mousePressEvent(event);
    }
}

bool CaptureOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_scene) {
        if (event->type() == QEvent::GraphicsSceneMousePress) {
            auto *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QGraphicsItem *itemUnderMouse = m_scene->itemAt(mouseEvent->scenePos(), QTransform());
                if (itemUnderMouse && itemUnderMouse->type() == QGraphicsTextItem::Type) {
                    return false;
                }

                m_scene->clearFocus();

                m_startPoint = mouseEvent->scenePos();
                if (m_currentTool == Tool::Rectangle) {
                    m_currentRect = new QGraphicsRectItem();
                    m_currentRect->setPen(QPen(Qt::red, 3));
                    m_scene->addItem(m_currentRect);
                } else if (m_currentTool == Tool::Arrow) {
                    m_currentArrowLine = new QGraphicsLineItem();
                    m_currentArrowLine->setPen(QPen(Qt::red, 3));
                    m_scene->addItem(m_currentArrowLine);

                    m_currentArrowHead = new QGraphicsPolygonItem();
                    m_currentArrowHead->setBrush(Qt::red);
                    m_currentArrowHead->setPen(Qt::NoPen);
                    m_scene->addItem(m_currentArrowHead);
                } else if (m_currentTool == Tool::Text) {
                    auto *textItem = new QGraphicsTextItem();
                    textItem->setDefaultTextColor(Qt::red);
                    QFont font = textItem->font();
                    font.setPointSize(14);
                    textItem->setFont(font);
                    textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
                    textItem->setPos(m_startPoint);
                    m_scene->addItem(textItem);
                    textItem->setFocus();
                }
                return true;
            }
        } else if (event->type() == QEvent::GraphicsSceneMouseMove) {
            auto *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            if (m_currentTool == Tool::Rectangle && m_currentRect) {
                QRectF rect(m_startPoint, mouseEvent->scenePos());
                m_currentRect->setRect(rect.normalized());
                return true;
            } else if (m_currentTool == Tool::Arrow && m_currentArrowLine && m_currentArrowHead) {
                QPointF endPoint = mouseEvent->scenePos();
                QLineF line(m_startPoint, endPoint);
                m_currentArrowLine->setLine(line);

                double angle = std::atan2(line.dy(), line.dx());
                qreal arrowSize = 15;
                QPointF arrowP1 = endPoint - QPointF(cos(angle + M_PI / 6) * arrowSize,
                                                     sin(angle + M_PI / 6) * arrowSize);
                QPointF arrowP2 = endPoint - QPointF(cos(angle - M_PI / 6) * arrowSize,
                                                     sin(angle - M_PI / 6) * arrowSize);

                QPolygonF arrowHead;
                arrowHead << endPoint << arrowP1 << arrowP2;
                m_currentArrowHead->setPolygon(arrowHead);
                return true;
            }
        } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
            auto *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                if (m_currentTool == Tool::Rectangle && m_currentRect) {
                    m_currentRect = nullptr;
                    return true;
                } else if (m_currentTool == Tool::Arrow && m_currentArrowLine) {
                    m_currentArrowLine = nullptr;
                    m_currentArrowHead = nullptr;
                    return true;
                }
                return false;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
