#include "mypixmapitem.h"
#include "filebuffer.h"

#include <QPen>
#include <QPainter>
#include <QDebug>

MyPixmapItem::MyPixmapItem() :
    buffer(nullptr),bufferSize(0LU), bufferOffset(0LU),
    w(0),h(0),
    bitsPerPixel(1),format(QImage::Format::Format_Mono),
    pixmapScale(1)
{
}

void MyPixmapItem::setBuffer(const uint8_t *ptr, const uint64_t size)
{
    buffer = const_cast<uint8_t*>(ptr);
    bufferSize = size;
}

void MyPixmapItem::setBufferOffset(const uint64_t offset) {
    bufferOffset = offset;
}

bool MyPixmapItem::incBufferOffset(const uint64_t offsetToAdd)
{
    if(bufferOffset + offsetToAdd < bufferSize) {
        bufferOffset += offsetToAdd;
        return true;
    }
    return false;  // Did't change
}

bool MyPixmapItem::decBufferOffset(const uint64_t offsetToSub)
{
    if(bufferSize == 0LU)
        return false;
    if(bufferOffset > offsetToSub)
        bufferOffset -= offsetToSub;
    else
        bufferOffset = 0;
    return true;
}

uint64_t MyPixmapItem::getVisibleBufferSize()
{
    uint64_t size = 0LU;
    if(bufferSize > 0LU) {
        int wbytes = w * bitsPerPixel / 8;
        if(geo.isValid()) {
            // Calc number of pixels on screen (height)
            int npixh = (geo.width() * h)/(pixmapScale * w);
            int actualh = h;  // Height in pixels of pixmap
            if(npixh >= geo.height()) {
                npixh = geo.height();
                actualh = (npixh * w * pixmapScale) / geo.width();
            }
            size = actualh * wbytes;

            // The buffer is smaller than the view
            if (size > bufferSize) {
                size = bufferSize;
            }

//            qDebug() << __FUNCTION__ <<  ":WIDTH PX:" << w << ",HEIGHT PX[calc]:" << actualh <<
//                        ",WIDTH BYTES:" << wbytes << ",SIZE:" << size;
        }
    }
    return size;
}

// Set number of pixels of a line
void MyPixmapItem::setWidth(const int width) {
    w = width;
    // Recalculate height if there is an img loaded
    recalculateHeight();
}

// Set total max height in pixels
void MyPixmapItem::setHeight(const int height) {
    h = height;
}

void MyPixmapItem::setBitsPerPixel(const uint8_t bpp)
{
    bitsPerPixel = bpp;
    // Recalculate height if there is an img loaded
    recalculateHeight();
}

void MyPixmapItem::recalculateHeight() {
    if(buffer != nullptr && w != 0)
       h = bufferSize / getBytesPerLine();
}

void MyPixmapItem::setImageFormat(const QImage::Format fmt)
{
    format = fmt; //TODO: change bitsPerPixel if not correct
}

void MyPixmapItem::setPixmapScale(const int scale)
{
    pixmapScale = scale;
}

void MyPixmapItem::setGeo(const QRect &geometryView)
{
    this->geo = geometryView;
}

uint64_t MyPixmapItem::getBufferOffset() const
{
    return bufferOffset;
}

uint8_t MyPixmapItem::getBitsPerPixel() const
{
    return bitsPerPixel;
}

int MyPixmapItem::getBytesPerLine() const
{
    return w/8 * bitsPerPixel;
}

QRectF MyPixmapItem::boundingRect() const
{
    return QRectF(0.0,0.0,100.0,100.0);
}

// Se ejecuta al hacer update() del pixmapItem
void MyPixmapItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget) {
/*    QRectF rect = boundingRect();
    QPen pen(Qt::red, 3);
    painter->setPen(pen);
    painter->drawEllipse(rect);*/
    if (buffer != nullptr) {
        int step = w/8 * bitsPerPixel;  // bytes per sprite line
        QImage img = QImage(w,h, format);
        //img.fill(0);

        uint8_t* ptr = buffer;
        for(int sl = 0; sl<h; sl++) {
            memcpy(img.scanLine(sl), ptr+bufferOffset, step);
            ptr += step;
        }

        QPixmap pixmap = QPixmap::fromImage(img).scaledToWidth(geo.width()/pixmapScale);
        painter->drawPixmap(QPointF(0,0), pixmap);
    }
}
