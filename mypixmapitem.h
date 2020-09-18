#ifndef MYPIXMAPITEM_H
#define MYPIXMAPITEM_H

#include <QRect>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsItem>

class MyPixmapItem : public QGraphicsItem
{
public:
    MyPixmapItem();

    void setBuffer(const uint8_t* ptr, const uint64_t size);
    void setBufferOffset(const uint64_t offset);
    bool incBufferOffset(const uint64_t offsetToAdd);
    bool decBufferOffset(const uint64_t offsetToSub);
    uint64_t getVisibleBufferSize();

    void setWidth(const int w);
    void setHeight(const int h);
    void setBitsPerPixel(const uint8_t bpp);
    void setImageFormat(const QImage::Format fmt);
    void setPixmapScale(const int scale);
    void setGeo(const QRect& geometryView);

    uint64_t getBufferOffset() const;
    uint8_t getBitsPerPixel() const;
    int getBytesPerLine() const;

    // Override virtual method for drawing
    QRectF boundingRect() const;
    void paint(QPainter* p ,const QStyleOptionGraphicsItem* o, QWidget* w);

private:
    // Buffer data
    uint8_t* buffer;
    uint64_t bufferSize;
    uint64_t bufferOffset;
    QRect    geo;

    // Image data
    int w,h;
    uint8_t bitsPerPixel;  // 1,8,16,24,32
    QImage::Format format;
    int pixmapScale;

    void recalculateHeight();
};

#endif // MYPIXMAPITEM_H
