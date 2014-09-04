#include "display/GLFWDisplay.h"
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

	// Construct the scene
	// This specifies scene geometry, light map and camera angle
	shared_ptr<Scene> scene = make_shared<Scene>();
	scene->loadConfig(argv[1]);

	// uses scene to produce rendering
	PathTracer pt;
	pt.setScene(scene, true);

	// pass output buffer to display
	d.setBuffer( pt.getOutputBuffer() );
	d.run( pt );

	return 0;
}
