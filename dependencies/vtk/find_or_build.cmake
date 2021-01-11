# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
hemelb_dependency(vtk find)

if (VTK_FOUND)
  message("VTK >= 9.0 already installed, no need to download")
else()
  message("VTK not installed, will build from source")
  find_file(VTK_TARBALL VTK-9.0.1.tar.gz
    DOC "Path to download VTK (can be url http://)"
    PATHS ${HEMELB_DEPENDENCIES_PATH}/distributions
    )
  if(NOT VTK_TARBALL)
    message("No VTK source found, will download.")
    set(VTK_TARBALL https://www.vtk.org/files/release/9.0/VTK-9.0.1.tar.gz
      CACHE STRING "Path to download VTK (can be local file://)" FORCE)
  endif()

  # Only compile the VTK modules being used by HemeLB at the moment, may need expanding in the future
  ExternalProject_Add(
    vtk
    INSTALL_DIR ${HEMELB_DEPENDENCIES_INSTALL_PATH}
    URL ${VTK_TARBALL}
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DVTK_BUILD_ALL_MODULES:BOOL=OFF
    -DVTK_Group_StandAlone:BOOL=OFF
    -DVTK_Group_Rendering:BOOL=OFF
    -DModule_vtkFiltersCore:BOOL=ON
    -DModule_vtkCommonDataModel:BOOL=ON
    -DModule_vtkIOXML:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${HEMELB_DEPENDENCIES_INSTALL_PATH}
    )
endif()
