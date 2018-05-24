file(REMOVE_RECURSE
  "../x86_64-slc6-gcc62-opt/include/VBFInvAnalysis"
  "../x86_64-slc6-gcc62-opt/data/VBFInvAnalysis/SUSYTools_Default.conf"
  "../x86_64-slc6-gcc62-opt/data/VBFInvAnalysis/SUSYTools_VBF.conf"
  "../x86_64-slc6-gcc62-opt/data/VBFInvAnalysis/prwfiles"
  "../x86_64-slc6-gcc62-opt/data/VBFInvAnalysis/samples"
  "../x86_64-slc6-gcc62-opt/data/VBFInvAnalysis/vbfinv.conf"
  "../x86_64-slc6-gcc62-opt/bin/runVBF.py"
  "../x86_64-slc6-gcc62-opt/bin/setupRelease.sh"
  "../x86_64-slc6-gcc62-opt/include/VBFInvAnalysis"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/VBFInvAnalysisHeaderInstall.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
