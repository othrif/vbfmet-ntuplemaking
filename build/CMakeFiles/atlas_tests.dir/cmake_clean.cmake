file(REMOVE_RECURSE
  "x86_64-slc6-gcc62-opt/setup.sh"
  "x86_64-slc6-gcc62-opt/lib/STAnalysisCode.rootmap"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/atlas_tests.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
