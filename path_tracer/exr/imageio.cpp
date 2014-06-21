/*
Author: Andrew Chalmers
Date:   23/04/2014

Feel free to do whatever you want.
*/

// Standard

// My includes
#include "imageio.h"

// EXR
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfArray.h>
#include <half.h>

// Namespaces
using namespace Imath;
using namespace Imf;
using namespace std;

// http://www.openexr.com/TechnicalIntroduction.pdf
// http://www.openexr.com/ReadingAndWritingImageFiles.pdf

imageio::imageio(void) {

}

imageio::~imageio(void) {

}

/*
 * Writing EXR Image
 */
void imageio::writeRgba1(const char fileName[], const Rgba *pixels, int width, int height){
  RgbaOutputFile file (fileName, width, height, WRITE_RGBA);
  file.setFrameBuffer (pixels, 1, width);
  file.writePixels (height);
}


/*
 * Reading EXR Image
 */
void imageio::readRgba1(const char fileName[], Array2D<Rgba> &pixels, int &width, int &height){
 RgbaInputFile file (fileName);
 Box2i dw = file.dataWindow();
 width = dw.max.x - dw.min.x + 1;
 height = dw.max.y - dw.min.y + 1;
 pixels.resizeErase (height, width);
 file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
 file.readPixels (dw.min.y, dw.max.y);
}

// Read in chunks, good for large images
void readRgba2 (const char fileName[]){
 RgbaInputFile file (fileName);
 Box2i dw = file.dataWindow();
 int width = dw.max.x - dw.min.x + 1;
 int height = dw.max.y - dw.min.y + 1;
 Array2D<Rgba> pixels (10, width);
 while (dw.min.y <= dw.max.y){
	 file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width,
	 1, width);
	 file.readPixels (dw.min.y, min (dw.min.y + 9, dw.max.y));
	 // processPixels (pixels)
	 dw.min.y += 10;
 }
}


bool imageio::ReadEXR(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha){
    try {
    InputFile file(name);
    Box2i dw = file.header().dataWindow();
    xRes = dw.max.x - dw.min.x + 1;
    yRes = dw.max.y - dw.min.y + 1;

    half *hrgba = new half[4 * xRes * yRes];

    hasAlpha = true;
    int nChannels = 4;

    hrgba -= 4 * (dw.min.x + dw.min.y * xRes);
    FrameBuffer frameBuffer;
    frameBuffer.insert("R", Slice(HALF, (char *)hrgba,
                  4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
    frameBuffer.insert("G", Slice(HALF, (char *)hrgba+sizeof(half),
                  4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
    frameBuffer.insert("B", Slice(HALF, (char *)hrgba+2*sizeof(half),
                  4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 0.0));
    frameBuffer.insert("A", Slice(HALF, (char *)hrgba+3*sizeof(half),
                  4*sizeof(half), xRes * 4 * sizeof(half), 1, 1, 1.0));

    file.setFrameBuffer(frameBuffer);
    file.readPixels(dw.min.y, dw.max.y);

    hrgba += 4 * (dw.min.x + dw.min.y * xRes);
    rgba = new float[nChannels * xRes * yRes];
    for (int i = 0; i < nChannels * xRes * yRes; ++i)
    rgba[i] = hrgba[i];
    delete[] hrgba;
    } catch (const std::exception &e) {
        fprintf(stderr, "Unable to read image file \"%s\": %s", name, e.what());
        return NULL;
    }

    return rgba;
}

