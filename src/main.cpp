
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
PathTracer pt;

void useGLUT(int argc, char** argv) {
	try {

		GLUTDisplay::setProgressiveDrawingTimeout(0.0f);
		GLUTDisplay::setUseSRGB(false);
		GLUTDisplay::run(argv[0], &pt, GLUTDisplay::CDProgressive);
	} catch (Exception& e) {
		sutilReportError(e.getErrorString().c_str());
		exit(1);
	}
}

int main(int argc, char** argv) {
	Display d(960, 540);
	GLUTDisplay::init(argc, argv);

	// Process command line options
	shared_ptr<Scene> scene = make_shared<Scene>();
	scene->loadConfig("config.txt");

	pt.initScene(scene, false);

	useGLUT(argc, argv);


	// pass output buffer to display
	d.run( pt );
	return 0;
}
