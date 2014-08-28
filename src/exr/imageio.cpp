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
using namespace optix;

optix::TextureSampler loadExrTexture(const char fileName[],
		optix::Context context, const float3& default_color) {
	std::cout << "Reading " << fileName << std::endl;
	Imf::Array2D < Imf::Rgba > pixels;
	int width;
	int height;
	Imf::imageio::readRgba1(fileName, pixels, width, height);

	float *rgba;
	bool hasAlpha;
	Imf::imageio::ReadEXR(fileName, rgba, width, height, hasAlpha);
	std::cout << "image dimensions " << width << "x" << height << std::endl;

	// Create tex sampler and populate with default values
	optix::TextureSampler sampler = context->createTextureSampler();
	sampler->setWrapMode(0, RT_WRAP_REPEAT);
	sampler->setWrapMode(1, RT_WRAP_REPEAT);
	sampler->setWrapMode(2, RT_WRAP_REPEAT);
	sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
	sampler->setReadMode(RT_TEXTURE_READ_ELEMENT_TYPE);
	sampler->setMaxAnisotropy(1.0f);
	sampler->setMipLevelCount(1u);
	sampler->setArraySize(1u);

	const unsigned int nx = width;
	const unsigned int ny = height;

	// Create buffer and populate with PPM data
	optix::Buffer buffer = context->createBuffer(RT_BUFFER_INPUT,
			RT_FORMAT_FLOAT4, nx, ny);
	float* buffer_data = static_cast<float *>(buffer->map());

	for (unsigned int i = 0; i < nx; ++i) {
		for (unsigned int j = 0; j < ny; ++j) {

			unsigned int ppm_index = ((ny - j - 1) * nx + (nx - i - 1));
			unsigned int buf_index = (j * nx + i) * 4;

			Imf::Rgba pix = pixels[ny - j - 1][nx - i - 1];
			buffer_data[buf_index + 0] = pix.r;
			buffer_data[buf_index + 1] = pix.g;
			buffer_data[buf_index + 2] = pix.b;
			buffer_data[buf_index + 3] = 1.0f;

//			buffer_data[buf_index + 0] = rgba[ppm_index * 4 + 0];
//			buffer_data[buf_index + 1] = rgba[ppm_index * 4 + 1];
//			buffer_data[buf_index + 2] = rgba[ppm_index * 4 + 2];
//			buffer_data[buf_index + 3] = rgba[ppm_index * 4 + 3];
		}
	}
	buffer->unmap();

	sampler->setBuffer(0u, 0u, buffer);
	sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR,
			RT_FILTER_NONE);

	return sampler;
}

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

