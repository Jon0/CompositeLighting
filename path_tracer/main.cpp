
/*
 * Copyright (c) 2008 - 2010 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */


#include <cuda.h>
#include <cuda_runtime.h>

#include "PathTracer.h"

using namespace std;
using namespace optix;

//-----------------------------------------------------------------------------
//
// main
//
//-----------------------------------------------------------------------------

void printUsageAndExit( const std::string& argv0, bool doExit = true )
{
  std::cerr
    << "Usage  : " << argv0 << " [options]\n"
    << "App options:\n"
    << "  -h  | --help                               Print this usage message\n"
    << " -rrd | --rr-begin-depth <d>                 Start Russian Roulette killing of rays at depth <d>\n"
    << "  -md | --max-depth <d>                      Maximum ray tree depth\n"
    << "  -n  | --sqrt_num_samples <ns>              Number of samples to perform for each frame\n"
    << "  -t  | --timeout <sec>                      Seconds before stopping rendering. Set to 0 for no stopping.\n"
    << std::endl;
  GLUTDisplay::printUsage();

  if ( doExit ) exit(1);
}


unsigned int getUnsignedArg(int& arg_index, int argc, char** argv)
{
  int result = -1;
  if (arg_index+1 < argc) {
    result = atoi(argv[arg_index+1]);
  } else {
    std::cerr << "Missing argument to "<<argv[arg_index]<<"\n";
    printUsageAndExit(argv[0]);
  }
  if (result < 0) {
    std::cerr << "Argument to "<<argv[arg_index]<<" must be positive.\n";
    printUsageAndExit(argv[0]);
  }
  ++arg_index;
  return static_cast<unsigned int>(result);
}

int main( int argc, char** argv )
{
  GLUTDisplay::init( argc, argv );

  // Process command line options
  int sceneType = 2;
  bool outline = false;
  unsigned int sqrt_num_samples = 1u;

  unsigned int width = 960u, height = 540u;
  float timeout = 0.0f;

  for ( int i = 1; i < argc; ++i ) {
    std::string arg( argv[i] );
    if ( arg == "--sqrt_num_samples" || arg == "-n" ) {
      if ( i == argc-1 ) printUsageAndExit( argv[0] );
      sqrt_num_samples = atoi( argv[++i] );
    } else if ( arg == "-g" ) {
      if ( i == argc-1 ) printUsageAndExit( argv[0] );
      sceneType = atoi( argv[++i] );
      std::cout << "scene type = " << sceneType << std::endl;
    }
    else if ( arg == "-o" ) {
    	outline = true;
    }
    else if ( arg == "--timeout" || arg == "-t" ) {
      if(++i < argc) {
        timeout = static_cast<float>(atof(argv[i]));
      } else {
        std::cerr << "Missing argument to "<<arg<<"\n";
        printUsageAndExit(argv[0]);
      }
    } else if ( arg == "--help" || arg == "-h" ) {
      printUsageAndExit( argv[0] );
    } else {
      std::cerr << "Unknown option: '" << arg << "'\n";
      printUsageAndExit( argv[0] );
    }
  }

  string desc = "";
  if (sceneType == 0) {
	  desc = "scene";
  }
  else if (sceneType == 1) {
	  desc = "local";
  }
  else if (sceneType >= 2) {
	  desc = "geom";
  }

  if (outline) {
	  desc += "Out";
  }

  try {
    PathTracerScene scene(sceneType, outline);
    scene.setNumSamples( sqrt_num_samples );
    scene.setDimensions( width, height );
    GLUTDisplay::setProgressiveDrawingTimeout(timeout);
    GLUTDisplay::setUseSRGB(false);
    GLUTDisplay::run( desc, &scene, GLUTDisplay::CDProgressive );
  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(1);
  }


  return 0;
}
