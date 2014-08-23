FILE(REMOVE_RECURSE
  "../lib/ptx/path_tracer_generated_parallelogram.cu.ptx"
  "../lib/ptx/path_tracer_generated_path_tracer.cu.ptx"
  "../lib/ptx/path_tracer_generated_triangle_mesh_iterative.cu.ptx"
  "CMakeFiles/path_tracer.dir/main.cpp.o"
  "CMakeFiles/path_tracer.dir/Args.cpp.o"
  "CMakeFiles/path_tracer.dir/PathTracer.cpp.o"
  "CMakeFiles/path_tracer.dir/Renderer.cpp.o"
  "CMakeFiles/path_tracer.dir/scene/Scene.cpp.o"
  "CMakeFiles/path_tracer.dir/exr/imageio.cpp.o"
  "../bin/path_tracer.pdb"
  "../bin/path_tracer"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang CXX)
  INCLUDE(CMakeFiles/path_tracer.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
