
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

#include "display/Display.h"
#include "PathTracer.h"

using namespace std;
using namespace optix;

//-----------------------------------------------------------------------------
//
// main
//
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
	GLUTDisplay::init(argc, argv);
	//Display d;

	// Process command line options
	int sceneType = 2;
	bool outline = false;
	unsigned int sqrt_num_samples = 1u;
	unsigned int width = 960u, height = 540u;
	float timeout = 0.0f;

	try {
		PathTracerScene scene;
		scene.setNumSamples(sqrt_num_samples);
		scene.setDimensions(width, height);
		GLUTDisplay::setProgressiveDrawingTimeout(timeout);
		GLUTDisplay::setUseSRGB(false);
		GLUTDisplay::run(argv[0], &scene, GLUTDisplay::CDProgressive);
	} catch (Exception& e) {
		sutilReportError(e.getErrorString().c_str());
		exit(1);
	}
	return 0;
}
