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

			int addr = chan*i;

			float geomWeight = geomOutline[addr] / 255.0f;
			float localWeight = localOutline[addr] / 255.0f;


			if (geomWeight > 0) {
				output[addr+0] = geomWeight * geom[addr+0];
				output[addr+1] = geomWeight * geom[addr+1];
				output[addr+2] = geomWeight * geom[addr+2];
			}
			else if (localWeight == 0) {
				output[addr+0] = scene[addr+0];
				output[addr+1] = scene[addr+1];
				output[addr+2] = scene[addr+2];
			}
			else {

				// local * m = geom
				float m0 = (float)geom[addr+0] / (float)local[addr+0];
				float m1 = (float)geom[addr+1] / (float)local[addr+1];
				float m2 = (float)geom[addr+2] / (float)local[addr+2];


				// local + d = geom
				int d0 = geom[addr+0] - local[addr+0];
				int d1 = geom[addr+1] - local[addr+1];
				int d2 = geom[addr+2] - local[addr+2];

				output[addr+0] = scene[addr+0] * m0; //256-(unsigned char)(d0); // scene[addr+0] +
				output[addr+1] = scene[addr+1] * m1; //256-(unsigned char)(d1);
				output[addr+2] = scene[addr+2] * m2; //256-(unsigned char)(d2);
			}


		}


		SavePPM((const unsigned char *)&output, "test.ppm", ppm1.width(), ppm1.height(), chan);

	}

};

int main( int argc, char** argv ) {
	Differential test;
	return 0;
}
