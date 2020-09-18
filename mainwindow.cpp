#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mypixmapitem.h"
#include "conversiontools.h"

const QStringList MainWindow::listImgFormats = { "Mono", "256 col", "24bpp", "32bpp" };
// Bits per pixel in the same order as listImgFormats
static const int8_t listImgFmtsBpp[] = {1, 8, 24, 32};
// Image formats in the same order as listImgFormats
static const QImage::Format listQImageFmts[] = {
    QImage::Format::Format_Mono, QImage::Format::Format_Indexed8,
    QImage::Format::Format_RGB888, QImage::Format::Format_ARGB32};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Populate formats combo box
    ui->comboBoxFmt->addItems(listImgFormats);

    // Create graphics view for sprites
    std::unique_ptr<MyPixmapItem> temp = std::unique_ptr<MyPixmapItem>(new MyPixmapItem());
    pmapitem = std::move(temp);
    // Initial format
    ui->comboBoxFmt->setCurrentIndex(0);
    pmapitem->setBitsPerPixel(listImgFmtsBpp[ui->comboBoxFmt->currentIndex()]);

    ui->graphicsView->setScene(new QGraphicsScene());
    ui->graphicsView->scene()->addItem(pmapitem.get());

    // Set font to monospace for hexdump zone
    ui->textEditHexDmp->setFontFamily("Monospace");

    // Check command line params (a file to load)
    if(QCoreApplication::arguments().size() > 1) {
        const QString filePath = QCoreApplication::arguments().at(1);
        doOpenFile(filePath);
    }

    sceneUpdate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Open a file requested
void MainWindow::on_actionOpen_triggered()
{
    const QString filePath = QFileDialog::getOpenFileName(this, ("Open binary file"),
                                                      "/home/pablo",
                                                      ("Binary file (*.*)"));
    doOpenFile(filePath);
}

void MainWindow::doOpenFile(const QString& filePath) {

    bool res = false;

    std::unique_ptr<QFile> fPtr(new QFile(filePath));

    if(fPtr->exists())
        res = fPtr->open(QFile::ReadOnly);

    if(res) {
        file = std::move(fPtr);
        fb = std::unique_ptr<filebuffer>(new filebuffer(file));
        char fbSize[32];
        snprintf(fbSize, sizeof(fbSize), "%lu", fb->getBufferSize());
        ui->labelFileName->setText(file.get()->fileName().append(" [").append(fbSize).append(" bytes]"));

        fileName=filePath;

        // Pass filebuffer to pixmap item, width, offset
        pmapitem->setBuffer(fb->getBufferPtr(), fb->getBufferSize());

        // Default values
        ui->spinBoxWidth->setValue(16);

        // Byte boundary if more than 1 pixels per byte
        ui->spinBoxWidth->setSingleStep(8);
        ui->spinBoxWidth->setMinimum(8);

        ui->lineEditOffset->setText("0x0000");

        pmapitem->setBufferOffset(0LU);

        // Initial color format: MONO, 1 bpp
        ui->comboBoxFmt->setCurrentIndex(0);
        int w = ui->spinBoxWidth->value();
        pmapitem->setWidth(w);
        // WARN: after setW so that height is calculated based on width
        pmapitem->setBitsPerPixel(listImgFmtsBpp[ui->comboBoxFmt->currentIndex()]);
        pmapitem->setImageFormat(listQImageFmts[ui->comboBoxFmt->currentIndex()]);
        pmapitem->setGeo(ui->graphicsView->geometry());

        hexdump();

        sceneUpdate();
     }
    else {
        QMessageBox mb;
        mb.critical(0, "Error opening file", QString("Error opening file ").append(filePath));
    }
}

// Offset in buffer changed
void MainWindow::onOffsetChange()
{
    if (!ui->lineEditOffset->isModified())
        return; // Ignore second signal.
    ui->lineEditOffset->setModified(false);  // Ignore second signal when user presses ENTER
    // (one for ENTER and the second for lost focus)

    const QString offset = ui->lineEditOffset->text();

    uint64_t iOffset = ConversionTools::numberStringToInteger(offset);

    if(iOffset != -1LU && fb->getBufferPtr() == nullptr) {
        iOffset = -1LU;
    }
    if(iOffset != -1LU) {
       pmapitem->setBufferOffset(iOffset);
       qDebug() << "NEW OFFSET:" << offset;
       // Hex dump
       hexdump();
    }
    else {
        // Wrong number: restore current offset
        ui->lineEditOffset->setText(
              ConversionTools::int64ToHexString(pmapitem->getBufferOffset()));
        qDebug() << "INVALID OFFSET " << offset;
    }

    sceneUpdate();
}

// Number of pixels per line shown changed
void MainWindow::onWidthChange(int pixelsPerLine)
{
    if(pmapitem->getBitsPerPixel() < 8 && pixelsPerLine % 8 != 0)
        qDebug() << "IGNORED " << pixelsPerLine << " PX PER LINE: NOT BYTE BOUNDARY";
    else {
        qDebug() << "NEW PIXELS PER LINE:" << pixelsPerLine;
        pmapitem->setWidth(pixelsPerLine);
        hexdump();
        sceneUpdate();
    }
}

void MainWindow::onImgFormatChange(int formatIndex)
{
    int bpp = listImgFmtsBpp[formatIndex];

    qDebug() << "NEW IMG FMT " << listImgFormats[formatIndex];

    pmapitem->setBitsPerPixel(bpp);
    pmapitem->setImageFormat(listQImageFmts[formatIndex]);

    ui->spinBoxWidth->setValue(ui->spinBoxWidth->value()%bpp);
    if (bpp<8) {
       ui->spinBoxWidth->setSingleStep(8);  // Byte boundary if more than 1 pixels per byte
       ui->spinBoxWidth->setMinimum(8);
    }
    else
       ui->spinBoxWidth->setSingleStep(1);

    hexdump();

    sceneUpdate();
}

void MainWindow::onScaleChange(int scale)
{
    // Pixmap scale modified so that we can see more data reducing width
    pmapitem->setPixmapScale(scale);
    hexdump();

    sceneUpdate();
}

void MainWindow::keyReleaseEvent(QKeyEvent* ev)
{
    //QWidget* focusWidget = QApplication::focusWidget();
    bool updated = false;
    if(ev->key() == Qt::Key_PageDown) {
        // Increase offset one page
        uint64_t bytes = pmapitem->getVisibleBufferSize();
        updated = pmapitem->incBufferOffset(bytes);
    }
    else if(ev->key() == Qt::Key_PageUp) {
        // Decrease offset one page
        uint64_t bytes = pmapitem->getVisibleBufferSize();
        updated = pmapitem->decBufferOffset(bytes);
    }
    else if(ev->key() == Qt::Key_Down) {
        // Increase offset by one line
        uint64_t bytes = pmapitem->getBytesPerLine();
        updated = pmapitem->incBufferOffset(bytes);
    }
    else if(ev->key() == Qt::Key_Up) {
        // Decrease offset one line
        uint64_t bytes = pmapitem->getBytesPerLine();
        updated = pmapitem->decBufferOffset(bytes);
    }

    if(updated) {
        // Update edit
        ui->lineEditOffset->setText(
              ConversionTools::int64ToHexString(pmapitem->getBufferOffset()));
        hexdump();
        sceneUpdate();
    }
    else
        QMainWindow::keyReleaseEvent(ev);
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

void MainWindow::hexdump()
{
    // Do a dump of current buffer
    if(fb->getBufferSize() > 0) {
        uint64_t offset = pmapitem->getBufferOffset();
        uint8_t* ptr = fb->getBufferPtr() + offset;
        ui->textEditHexDmp->clear();
        QString line;
        char byteStr[3];
        uint64_t cnt=0;
        uint64_t endOffset = offset + pmapitem->getVisibleBufferSize();
        while(offset < endOffset) {
            sprintf(byteStr, "%02X", *ptr);
            if(cnt%16 == 0) {
                char addrStr[32] = {0};
                sprintf(addrStr, "%08" PRIx64 ": ", offset); //"%p: ", ptr);
                line.append(addrStr);
            }
            line.append(byteStr);
            cnt++;
            offset++;
            ptr++;
            if(cnt%16==0 || offset == endOffset) {
               line.append("\n");
               ui->textEditHexDmp->insertPlainText(line);
               line.clear();
            }
        }
    }
}

// Refresh Scene
void MainWindow::sceneUpdate()
{
    ui->graphicsView->scene()->update();
}
