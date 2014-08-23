//#include <optixu/optixpp_namespace.h>
//#include <sutil.h>
//#include <PPMLoader.h>

#include <ImageDisplay.h>
#include <PPMLoader.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

static RTresult SavePPM(const unsigned char *Pix, const char *fname, int wid, int hgt, int chan)
{
  if(Pix==NULL || chan < 1 || wid < 1 || hgt < 1) {
    fprintf(stderr, "Image is not defined. Not saving.\n");
    return RT_ERROR_UNKNOWN;
  }

  if(chan < 1 || chan > 4) {
    fprintf(stderr, "Can't save a X channel image as a PPM.\n");
    return RT_ERROR_UNKNOWN;
  }

  ofstream OutFile(fname, ios::out | ios::binary);
  if(!OutFile.is_open()) {
    fprintf(stderr, "Could not open file for SavePPM\n");
    return RT_ERROR_UNKNOWN;
  }

  bool is_float = false;
  OutFile << 'P';
  OutFile << ((chan==1 ? (is_float?'Z':'5') : (chan==3 ? (is_float?'7':'6') : '8'))) << endl;
  OutFile << wid << " " << hgt << endl << 255 << endl;

  OutFile.write(reinterpret_cast<char*>(const_cast<unsigned char*>( Pix )), wid * hgt * chan * (is_float ? 4 : 1));

  OutFile.close();

  return RT_SUCCESS;
}

char sub(unsigned char a, unsigned char b) {
	char i = a;
	char j = b;
	return (j - i);
}

/*
 * Differential Rendering
 */
class Differential {
public:
	Differential() {
		PPMLoader ppm1("outputs/scene.ppm", false);
		PPMLoader ppm2("outputs/local.ppm", false);
		PPMLoader ppm3("outputs/geom.ppm", false);
		PPMLoader ppm4("outputs/geomOut.ppm", false);
		PPMLoader ppm5("outputs/localOut.ppm", false);

		unsigned char *scene = ppm1.raster();
		unsigned char *local = ppm2.raster();
		unsigned char *geom = ppm3.raster();
		unsigned char *geomOutline = ppm4.raster();
		unsigned char *localOutline = ppm5.raster();

		int chan = 3;
		int size = ppm1.width()*ppm1.height();
		unsigned char output[ size*chan ];


		for (int i = 0; i < size; ++i) {

			int base_addr = chan*i;
			float geomWeight = geomOutline[base_addr] / 256.0f;
			float localWeight = localOutline[base_addr] / 256.0f;
			float nongeomWeight = 1.0f - geomWeight;
			float nonlocalWeight = 1.0f - localWeight;

			for (int addr = base_addr; addr < base_addr + chan; ++addr) {
				unsigned int out = 0;
				out += geomWeight * geom[addr];
				out += nonlocalWeight * nongeomWeight * scene[addr];

				// local * m = geom
				float m0 = (float)geom[addr] / (float)local[addr];
				out += localWeight * nongeomWeight * scene[addr] * m0;

				if (out > 255) out = 255;
				output[addr] = out;
			}
		}


		SavePPM((const unsigned char *)&output, "outputs/differential.ppm", ppm1.width(), ppm1.height(), chan);

	}

};

int main( int argc, char** argv ) {
	Differential test;
	return 0;
}
