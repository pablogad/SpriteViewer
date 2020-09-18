#ifndef FILEBUFFER_H
#define FILEBUFFER_H

#include <QFile>

#include <memory>

const uint64_t BUFFER_SIZE = 1048576;

class filebuffer
{
public:
    filebuffer(std::unique_ptr<QFile>& file);
    bool readBuffer();

    uint8_t* getBufferPtr() { return buffer; }
    uint64_t getBufferSize() { return buffersize; }

private:
    uint8_t buffer[BUFFER_SIZE];
    uint64_t buffersize;

    std::unique_ptr<QFile>& fPtr;
    uint64_t fileoffset;
};

#endif // FILEBUFFER_H
