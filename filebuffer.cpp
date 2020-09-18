#include "filebuffer.h"

filebuffer::filebuffer(std::unique_ptr<QFile>& f) : fPtr(f), fileoffset(0LU) {
    readBuffer();
}

// Read a portion of the input file of max BUFFER_SIZE
bool filebuffer::readBuffer() {
    bool res=false;
    // Create buffer reading f from the beggining
    if(fPtr->isReadable() && fPtr->isOpen()) {
        fPtr->seek(fileoffset);
        buffersize = fPtr->read(reinterpret_cast<char*>(&buffer), BUFFER_SIZE);
        if(buffersize != -1LU) {
            res=true;
            fileoffset += buffersize;
        }
        else {
            buffersize = 0LU;
        }
    }
    return res;
}
