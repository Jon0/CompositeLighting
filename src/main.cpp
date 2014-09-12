#include "display/GLFWDisplay.h"
#include "scene/SceneParser.h"
#include "PathTracer.h"

using namespace std;
using namespace optix;

// Initialise glew / opengl display first
GLFWDisplay d;

//-----------------------------------------------------------------------------
//
// main
//
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "argument must specify config file" << endl;
		exit(1);
	}
	srand(time(NULL));

	// Construct the scene
	// This specifies scene geometry, light map and camera angle

	SceneParser sp(argv[1]);
	shared_ptr<Scene> scene = sp.readScene();

	// uses scene to produce rendering

	PathTracer pt;
	pt.setScene(scene, true);

	// pass output buffer to the display

	d.setBuffer( pt.getOutputBuffer() );
	d.run( pt );

	return 0;
}
