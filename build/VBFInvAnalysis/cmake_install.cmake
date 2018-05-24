# Install script for directory: /nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/InstallArea/x86_64-slc6-gcc62-opt")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/src/VBFInvAnalysis" TYPE DIRECTORY FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE REGEX "/\\.git$" EXCLUDE REGEX "/[^/]*\\~$" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE OPTIONAL FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/build/x86_64-slc6-gcc62-opt/lib/libVBFInvAnalysisLib_rdict.pcm")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process( COMMAND ${CMAKE_COMMAND}
      -E make_directory
      $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process( COMMAND ${CMAKE_COMMAND}
         -E create_symlink ../src/VBFInvAnalysis/VBFInvAnalysis
         $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include/VBFInvAnalysis )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDebugx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE OPTIONAL FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/build/x86_64-slc6-gcc62-opt/lib/libVBFInvAnalysisLib.so.dbg")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY OPTIONAL FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/build/x86_64-slc6-gcc62-opt/lib/libVBFInvAnalysisLib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libVBFInvAnalysisLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libVBFInvAnalysisLib.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/atlas.cern.ch/repo/sw/software/21.2/sw/lcg/releases/binutils/2.28/x86_64-slc6/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libVBFInvAnalysisLib.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/VBFInvAnalysis" TYPE FILE RENAME "SUSYTools_Default.conf" FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/share/SUSYTools_Default.conf")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/VBFInvAnalysis" TYPE FILE RENAME "SUSYTools_VBF.conf" FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/share/SUSYTools_VBF.conf")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/VBFInvAnalysis" TYPE DIRECTORY FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/share/prwfiles" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/VBFInvAnalysis" TYPE DIRECTORY FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/share/samples" USE_SOURCE_PERMISSIONS REGEX "/\\.svn$" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/data/VBFInvAnalysis" TYPE FILE RENAME "vbfinv.conf" FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/share/vbfinv.conf")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "runVBF.py" FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/scripts/runVBF.py")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "setupRelease.sh" FILES "/nfs/dust/atlas/user/othrif/scratch/myNewNtupleMaker/STAnalysisCode/VBFInvAnalysis/scripts/setupRelease.sh")
endif()

