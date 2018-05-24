# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#
# Configuration file for an ATLAS CMake release installation.
# It defines the following variables:
#
#  STAnalysisCode_ROOT_DIR     - Root directory for the release
#  STAnalysisCode_INCLUDE_DIR  - Include directory for the release
#  STAnalysisCode_INCLUDE_DIRS - Include directory for the release
#  STAnalysisCode_LIBRARY_DIR  - Library directory for the release
#  STAnalysisCode_LIBRARY_DIRS - Library directory for the release
#  STAnalysisCode_RUNTIME_DIR  - Runtime directory for the release
#  STAnalysisCode_MODULE_DIR   - Directory holding CMake modules
#  STAnalysisCode_PYTHON_DIR   - Directory holding python code
#
# Note however that most of the time you should not be using any of these
# variables directly, but should rather use the ROOTCORE_PROJECT(...) function
# to set up a work area that compiles against this release.
#

# Get some path names:
get_filename_component( _thisdirSTAnalysisCode
   "${CMAKE_CURRENT_LIST_FILE}" PATH )
get_filename_component( _releasedirSTAnalysisCode
   "${_thisdirSTAnalysisCode}" PATH )

# Set the root directory variable:
get_filename_component( STAnalysisCode_ROOT_DIR
   "${_releasedirSTAnalysisCode}" ABSOLUTE )

# Print what project/release was found just now:
if( NOT STAnalysisCode_FIND_QUIETLY )
   message( STATUS "Found STAnalysisCode: "
      "${STAnalysisCode_ROOT_DIR} (version: 1.0.0)" )
endif()

# Set the include directory variable:
get_filename_component( STAnalysisCode_INCLUDE_DIR
   "${_releasedirSTAnalysisCode}/include" ABSOLUTE )
set( STAnalysisCode_INCLUDE_DIRS ${STAnalysisCode_INCLUDE_DIR} )

# Set the library directory variable:
get_filename_component( STAnalysisCode_LIBRARY_DIR
   "${_releasedirSTAnalysisCode}/lib" ABSOLUTE )
set( STAnalysisCode_LIBRARY_DIRS ${STAnalysisCode_LIBRARY_DIR} )

# Set the runtime directory variable:
get_filename_component( STAnalysisCode_RUNTIME_DIR
   "${_releasedirSTAnalysisCode}/bin" ABSOLUTE )
set( STAnalysisCode_BINARY_PATH ${STAnalysisCode_RUNTIME_DIR} )

# Set the python directory variable:
get_filename_component( STAnalysisCode_PYTHON_DIR
   "${_releasedirSTAnalysisCode}/python" ABSOLUTE )
set( STAnalysisCode_PYTHON_PATH ${STAnalysisCode_PYTHON_DIR} )

# The base projects that this project was built on top of:
set( STAnalysisCode_BASE_PROJECTS AnalysisBase;21.2.30 )

# The platform name used for the build. It is overridden in release builds,
# but can be useful when building vanilla CMake code against the release.
set( ATLAS_PLATFORM x86_64-slc6-gcc62-opt )

# Packages provided by the project:
set( STAnalysisCode_PACKAGE_NAMES SUSYTools;VBFInvAnalysis )

# Include the base projects, in the order in which they were given to
# the atlas_projects call.
set( _baseProjectsSTAnalysisCode
   ${STAnalysisCode_BASE_PROJECTS} )
while( _baseProjectsSTAnalysisCode )
   # Extract the release name and version, and then remove these entries
   # from the list:
   list( GET _baseProjectsSTAnalysisCode 0
      _base_projectSTAnalysisCode )
   list( GET _baseProjectsSTAnalysisCode 1
      _base_versionSTAnalysisCode )
   list( REMOVE_AT _baseProjectsSTAnalysisCode 0 1 )
   # Make sure that the project version is a regular version number:
   if( NOT _base_versionSTAnalysisCode MATCHES "^[0-9]+[0-9.]*" )
      # Let's not specify a version in this case...
      message( STATUS "Using base project "
         "\"${_base_projectSTAnalysisCode}\" without "
         "its \"${_base_versionSTAnalysisCode}\" version name/number" )
      set( _base_versionSTAnalysisCode )
   endif()
   # Find the base release:
   if( STAnalysisCode_FIND_QUIETLY )
      find_package( ${_base_projectSTAnalysisCode}
         ${_base_versionSTAnalysisCode} QUIET )
   else()
      find_package( ${_base_projectSTAnalysisCode}
         ${_base_versionSTAnalysisCode} )
   endif()
endwhile()
unset( _baseProjectsSTAnalysisCode )
unset( _base_projectSTAnalysisCode )
unset( _base_versionSTAnalysisCode )

# Make CMake find the release's installed modules. Append the module
# library instead of prepending it. To allow the user to override the
# modules packaged with the release.
get_filename_component( STAnalysisCode_MODULE_DIR
   "${_thisdirSTAnalysisCode}/modules" ABSOLUTE )
if( ATLAS_DONT_PREPEND_PROJECT_MODULES )
   list( APPEND CMAKE_MODULE_PATH ${STAnalysisCode_MODULE_DIR} )
else()
   list( INSERT CMAKE_MODULE_PATH 0 ${STAnalysisCode_MODULE_DIR} )
endif()
list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )

# Pull in the ATLAS code:
include( AtlasFunctions )

# Include the project-specific pre-include file, if it exists:
if( EXISTS ${_thisdirSTAnalysisCode}/PreConfig.cmake )
   include( ${_thisdirSTAnalysisCode}/PreConfig.cmake )
endif()

# Include the file listing all the imported targets and options:
if( EXISTS
   ${_thisdirSTAnalysisCode}/STAnalysisCodeConfig-targets.cmake )
   include(
      ${_thisdirSTAnalysisCode}/STAnalysisCodeConfig-targets.cmake )
endif()

# Check what build mode the release was built with. And set CMAKE_BUILD_TYPE
# to that value by default. While there should only be one build mode in
# a given install area, provide an explicit preference order to the different
# build modes:
foreach( _type Debug RelWithDebInfo Release MinSizeRel Default )
   string( TOLOWER "${_type}" _typeLower )
   set( _fileName "${_thisdirSTAnalysisCode}/" )
   set( _fileName "${_fileName}STAnalysisCodeConfig-targets" )
   set( _fileName "${_fileName}-${_typeLower}.cmake" )
   if( EXISTS "${_fileName}" AND NOT CMAKE_BUILD_TYPE )
      set( CMAKE_BUILD_TYPE "${_type}"
         CACHE STRING "Build mode for the release" FORCE )
   endif()
   unset( _fileName )
   unset( _typeLower )
endforeach()

# Tell the user how many packages were found:
if( NOT STAnalysisCode_FIND_QUIETLY )
   list( LENGTH STAnalysisCode_PACKAGE_NAMES _nPackages )
   message( STATUS "Package(s) in STAnalysisCode: ${_nPackages}" )
   unset( _nPackages )
endif()

# Set the CMake code into "release mode" for the next bit:
set( ATLAS_RELEASE_MODE ON )
set( ATLAS_RELEASE_NAME STAnalysisCode )

# If recompilation is requested:
if( STAnalysisCode_FIND_COMPONENTS )

   # Put the packages either into recompilation or simple inclusion mode:
   if( ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "RECOMPILE" OR
         ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "RECOMPILE_DRYRUN" OR
         ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "RECOMPILE_SETUP"  )
      set( ATLAS_RELEASE_RECOMPILE ON )
   elseif( ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "INCLUDE" )
      set( ATLAS_RELEASE_RECOMPILE OFF )
   else()
      message( WARNING "Please use either 'RECOMPILE' or 'INCLUDE' as\n"
         "the COMPONENTS argument of the find_package(...)\n"
         "call. Assuming 'INCLUDE' mode for now." )
      set( ATLAS_RELEASE_RECOMPILE OFF )
   endif()

   # Include the packages. The user must not call find_package
   # multiple times with COMPONENTS options. So no protection is added
   # here...

   # Find all the packages that need to be recompiled. This includes
   # packages that only indirectly depend on a package in the work area.
   # (Through another package in the middle.) To find all the dependencies,
   # the CMakeLists.txt files of the release are scanned multiple times.
   # Until no new package is found that should be recompiled.
   if( ATLAS_RELEASE_RECOMPILE )

      # Find the packages in the source directory:
      set( STAnalysisCode_packageDirs )
      file( GLOB_RECURSE cmakelist_files RELATIVE ${CMAKE_SOURCE_DIR}
         "${_releasedirSTAnalysisCode}/src/*/CMakeLists.txt" )
      foreach( file ${cmakelist_files} )
         get_filename_component( package ${file} DIRECTORY )
         list( APPEND STAnalysisCode_packageDirs ${package} )
      endforeach()
      unset( cmakelist_files )
      unset( package )

      # Only do the scanning if we've not finished with it, starting from
      # a higher level project, already:
      if( NOT ${STAnalysisCode_FIND_COMPONENTS} STREQUAL
            "RECOMPILE_SETUP" )

         # Decide which packages need to be recompiled:
         message( STATUS "Scanning project STAnalysisCode for "
            "packages to recompile..." )
         set( ATLAS_RELEASE_RECOMPILE_DRYRUN ON )
         set( _keepScanningSTAnalysisCode ON )
         if( NOT DEFINED _iteration )
            set( _iteration 0 )
         endif()
         while( _keepScanningSTAnalysisCode )
            # Because of the recursiveness of the next bit, make sure that the
            # project name is still set correctly:
            set( ATLAS_RELEASE_NAME STAnalysisCode )
            # A helper variable:
            math( EXPR _iteration "${_iteration} + 1" )
            # Check how many packages are in the recompilation list right now:
            get_property( _recompilePackagesSTAnalysisCode GLOBAL PROPERTY
               ATLAS_PACKAGES_TO_RECOMPILE )
            list( LENGTH _recompilePackagesSTAnalysisCode
               _nBeforeSTAnalysisCode )
            # Dryrun all the packages in this project:
            foreach( pkgDir ${STAnalysisCode_packageDirs} )
               get_filename_component( pkgName ${pkgDir} NAME )
               # This variable is only defined in this weird way to be able to
               # stay within an 80 column width with the source.
               set( _buildDir "${CMAKE_BINARY_DIR}/STAnalysisCode" )
               set( _buildDir "${_buildDir}/dryrun${_iteration}/${pkgName}" )
               add_subdirectory( ${pkgDir} ${_buildDir} )
            endforeach()
            # Now dryrun all the packages in the base project(s):
            set( _baseProjectsSTAnalysisCode
               ${STAnalysisCode_BASE_PROJECTS} )
            while( _baseProjectsSTAnalysisCode )
               list( GET _baseProjectsSTAnalysisCode 0
                  _base_projectSTAnalysisCode )
               list( GET _baseProjectsSTAnalysisCode 1
                  _base_versionSTAnalysisCode )
               list( REMOVE_AT _baseProjectsSTAnalysisCode 0 1 )
               if( STAnalysisCode_FIND_QUIETLY )
                  find_package( ${_base_projectSTAnalysisCode}
                     QUIET
                     COMPONENTS RECOMPILE_DRYRUN )
               else()
                  find_package( ${_base_projectSTAnalysisCode}
                     COMPONENTS RECOMPILE_DRYRUN )
               endif()
            endwhile()
            unset( _baseProjectsSTAnalysisCode )
            unset( _base_projectSTAnalysisCode )
            unset( _base_versionSTAnalysisCode )
            # Check if the dry running needs to continue or not:
            get_property( _recompilePackagesSTAnalysisCode GLOBAL PROPERTY
               ATLAS_PACKAGES_TO_RECOMPILE )
            list( LENGTH _recompilePackagesSTAnalysisCode
               _nAfterSTAnalysisCode )
            if( _nBeforeSTAnalysisCode EQUAL _nAfterSTAnalysisCode )
               set( _keepScanningSTAnalysisCode OFF )
            endif()
         endwhile()
         # This should be shown even in quiet mode:
         message( STATUS "Found ${_nAfterSTAnalysisCode} packages "
            "needing recompilation, in ${_iteration} iteration(s) "
            "(STAnalysisCode)")
         # Clean up:
         unset( _keepScanningSTAnalysisCode )
         unset( _nBeforeSTAnalysisCode )
         unset( _nAfterSTAnalysisCode )
         unset( _recompilePackagesSTAnalysisCode )
      endif()

      # If we only had to do a dryrun, then exit now:
      if( ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "RECOMPILE_DRYRUN" )
         return()
      endif()

      # Now we're out of the dryrun phase:
      set( ATLAS_RELEASE_RECOMPILE_DRYRUN OFF )

      # Some further cleanup:
      unset( _iteration )

      # Now include all the packages. They should know which one of them
      # need recompilation.
      message( STATUS "Setting up the packages from project "
         "STAnalysisCode - 1.0.0..." )
      set( ATLAS_RELEASE_NAME STAnalysisCode )
      foreach( pkgDir ${STAnalysisCode_packageDirs} )
         get_filename_component( pkgName ${pkgDir} NAME )
         add_subdirectory( ${pkgDir}
            ${CMAKE_BINARY_DIR}/STAnalysisCode/${pkgName} )
      endforeach()

   else()

      # Tell the user what's happening:
      message( STATUS "Including the packages from project "
         "STAnalysisCode - 1.0.0..." )

      # Targets to pull into the current project:
      set( STAnalysisCode_TARGET_NAMES SUSYToolsPkg;SUSYToolsPkgPrivate;SUSYToolsLib;SUSYToolsTester;SUSYTruthTester;SUSYTools_check_xsections;SUSYTools_check_syst;VBFInvAnalysisPkg;VBFInvAnalysisPkgPrivate;VBFInvAnalysisLib )

      # Loop over the targets that this project has:
      foreach( _target ${STAnalysisCode_TARGET_NAMES} )
         # If the target exists already, then don't do aything else:
         if( TARGET ${_target} )
            continue()
         endif()
         # Check whether the target in question is known in this release:
         if( NOT TARGET STAnalysisCode::${_target} )
            message( SEND_ERROR
               "Target with name STAnalysisCode::${_target} not found" )
            continue()
         endif()
         # Get the type of this target:
         get_property( _type TARGET STAnalysisCode::${_target}
            PROPERTY TYPE )
         # And now create a copy of this target in the right way:
         if( "${_type}" STREQUAL "SHARED_LIBRARY" OR
               "${_type}" STREQUAL "MODULE_LIBRARY" OR
               "${_type}" STREQUAL "INTERFACE_LIBRARY" )
            # First, create the right type of library:
            if( "${_type}" STREQUAL "SHARED_LIBRARY" )
               # Create a shared library:
               add_library( ${_target} SHARED IMPORTED GLOBAL )
            elseif( "${_type}" STREQUAL "MODULE_LIBRARY" )
               # Create a module library:
               add_library( ${_target} MODULE IMPORTED GLOBAL )
            elseif( "${_type}" STREQUAL "INTERFACE_LIBRARY" )
               # Create an interface library:
               add_library( ${_target} INTERFACE IMPORTED GLOBAL )
            endif()
            # Copy its dependencies:
            get_property( _libSet TARGET STAnalysisCode::${_target}
               PROPERTY INTERFACE_LINK_LIBRARIES SET )
            if( _libSet )
               get_target_property( _libraries STAnalysisCode::${_target}
                  INTERFACE_LINK_LIBRARIES )
               string( REPLACE "STAnalysisCode::" "" _libraries
                  "${_libraries}" )
               set_target_properties( ${_target} PROPERTIES
                  INTERFACE_LINK_LIBRARIES "${_libraries}" )
               unset( _libraries )
            endif()
            unset( _libSet )
            get_property( _incSet TARGET STAnalysisCode::${_target}
               PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES SET )
            if( _incSet )
               get_target_property( _includes STAnalysisCode::${_target}
                  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES )
               string( REPLACE "STAnalysisCode::" "" _includes
                  "${_includes}" )
               set_target_properties( ${_target} PROPERTIES
                  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_includes}" )
               unset( _includes )
            endif()
            get_property( _incSet TARGET STAnalysisCode::${_target}
               PROPERTY INTERFACE_INCLUDE_DIRECTORIES SET )
            if( _incSet )
               get_target_property( _includes STAnalysisCode::${_target}
                  INTERFACE_INCLUDE_DIRECTORIES )
               string( REPLACE "STAnalysisCode::" "" _includes
                  "${_includes}" )
               set_target_properties( ${_target} PROPERTIES
                  INTERFACE_INCLUDE_DIRECTORIES "${_includes}" )
               unset( _includes )
            endif()
            unset( _incSet )
            get_property( _defSet TARGET STAnalysisCode::${_target}
               PROPERTY INTERFACE_COMPILE_DEFINITIONS SET )
            if( _defSet )
               get_target_property( _definitions
                  STAnalysisCode::${_target}
                  INTERFACE_COMPILE_DEFINITIONS )
               set_target_properties( ${_target} PROPERTIES
                  INTERFACE_COMPILE_DEFINITIONS "${_definitions}" )
               unset( _definitions )
            endif()
            unset( _defSet )
            # For physical libraries, copy their location as well:
            if( "${_type}" STREQUAL "SHARED_LIBRARY" OR
                  "${_type}" STREQUAL "MODULE_LIBRARY" )
               # Get the configurations that it is available for:
               get_target_property( _builds STAnalysisCode::${_target}
                  IMPORTED_CONFIGURATIONS )
               set_target_properties( ${_target} PROPERTIES
                  IMPORTED_CONFIGURATIONS ${_builds} )
               foreach( _build ${_builds} )
                  # Copy the properties:
                  get_target_property( _location
                     STAnalysisCode::${_target}
                     IMPORTED_LOCATION_${_build} )
                  get_target_property( _soname
                     STAnalysisCode::${_target}
                     IMPORTED_SONAME_${_build} )
                  set_target_properties( ${_target} PROPERTIES
                     IMPORTED_LOCATION_${_build} "${_location}"
                     IMPORTED_SONAME_${_build} "${_soname}" )
                  unset( _location )
                  unset( _soname )
                  get_property( _depLibSet
                     TARGET STAnalysisCode::${_target}
                     PROPERTY IMPORTED_LINK_DEPENDENT_LIBRARIES_${_build} SET )
                  if( _depLibSet )
                     get_target_property( _depLib
                        STAnalysisCode::${_target}
                        IMPORTED_LINK_DEPENDENT_LIBRARIES_${_build} )
                     string( REPLACE "STAnalysisCode::" ""
                        _depLib "${_depLib}" )
                     set_target_properties( ${_target} PROPERTIES
                        IMPORTED_LINK_DEPENDENT_LIBRARIES_${_build}
                        "${_depLib}" )
                     unset( _depLib )
                  endif()
                  unset( _depLibSet )
               endforeach()
               unset( _builds )
            endif()
         elseif( "${_type}" STREQUAL "EXECUTABLE" )
            # Set up the imported executable:
            add_executable( ${_target} IMPORTED GLOBAL )
            # Copy its location for all available configurations:
            get_target_property( _builds STAnalysisCode::${_target}
               IMPORTED_CONFIGURATIONS )
            set_target_properties( ${_target} PROPERTIES
               IMPORTED_CONFIGURATIONS "${_builds}" )
            foreach( _build ${_builds} )
               # Copy the properties:
               get_target_property( _location STAnalysisCode::${_target}
                  IMPORTED_LOCATION_${_build} )
               set_target_properties( ${_target} PROPERTIES
                  IMPORTED_LOCATION_${_build} "${_location}" )
               unset( _location )
            endforeach()
            unset( _builds )
         else()
            message( SEND_ERROR "Unknown target type (${_type}) encountered" )
         endif()
         unset( _type )
      endforeach()

   endif()

   # Include the base projects, in reverse order. So that the components from
   # the end of the list would get precedence over the components from the
   # front.
   set( _baseProjectsSTAnalysisCode
      ${STAnalysisCode_BASE_PROJECTS} )
   while( _baseProjectsSTAnalysisCode )
      # Get the last project from the list:
      list( LENGTH _baseProjectsSTAnalysisCode
         _lengthSTAnalysisCode )
      math( EXPR _projNameIdxSTAnalysisCode
         "${_lengthSTAnalysisCode} - 2" )
      math( EXPR _projVersIdxSTAnalysisCode
         "${_lengthSTAnalysisCode} - 1" )
      list( GET _baseProjectsSTAnalysisCode
         ${_projNameIdxSTAnalysisCode}
         _base_projectSTAnalysisCode )
      list( GET _baseProjectsSTAnalysisCode
         ${_projVersIdxSTAnalysisCode}
         _base_versionSTAnalysisCode )
      list( REMOVE_AT _baseProjectsSTAnalysisCode
         ${_projNameIdxSTAnalysisCode}
         ${_projVersIdxSTAnalysisCode} )
      # Find the base release:
      if( ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "INCLUDE" )
         find_package( ${_base_projectSTAnalysisCode}
            COMPONENTS INCLUDE QUIET )
      elseif( ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "RECOMPILE" OR
            ${STAnalysisCode_FIND_COMPONENTS} STREQUAL "RECOMPILE_SETUP" )
         # By this time all the dry running has been done. So let's just
         # set up the packages for recompilation already.
         find_package( ${_base_projectSTAnalysisCode}
            COMPONENTS RECOMPILE_SETUP QUIET )
      endif()
   endwhile()
   unset( _baseProjectsSTAnalysisCode )
   unset( _projNameIdxSTAnalysisCode )
   unset( _projVersIdxSTAnalysisCode )
   unset( _base_projectSTAnalysisCode )
   unset( _base_versionSTAnalysisCode )
   unset( _lengthSTAnalysisCode )

endif()

# Take CMake out of "release mode":
set( ATLAS_RELEASE_MODE OFF )

# Only do this if necessary:
if( NOT ATLAS_DONT_PREPEND_PROJECT_MODULES )
   # Make sure that after all of this, we still have this release's module
   # directory at the front of the list:
   list( INSERT CMAKE_MODULE_PATH 0 ${STAnalysisCode_MODULE_DIR} )
   list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )
endif()

# Include the project-specific post-include file, if it exists:
if( EXISTS ${_thisdirSTAnalysisCode}/PostConfig.cmake )
   include( ${_thisdirSTAnalysisCode}/PostConfig.cmake )
endif()

# Remove the unwanted variables:
unset( _thisdirSTAnalysisCode )
unset( _releasedirSTAnalysisCode )
