#############################################################################
#
# This file is part of the ustk software.
# Copyright (C) 2016 - 2017 by Inria. All rights reserved.
#
# This software is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# ("GPL") version 2 as published by the Free Software Foundation.
# See the file LICENSE.txt at the root directory of this source
# distribution for additional information about the GNU GPL.
#
# For using ustk with software that can not be combined with the GNU
# GPL, please contact Inria about acquiring a ViSP Professional
# Edition License.
#
# This software was developed at:
# Inria Rennes - Bretagne Atlantique
# Campus Universitaire de Beaulieu
# 35042 Rennes Cedex
# France
#
# If you have questions regarding the use of this file, please contact
# Inria at ustk@inria.fr
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# Authors:
# Fabien Spindler
#
#############################################################################

if(USE_VTK)
  include(${VTK_USE_FILE})

  list(APPEND opt_incs ${VTK_INCLUDE_DIRS})

  # Check for vtk dependencies
  foreach(lib_ ${VTK_LIBRARIES})
    if(${lib_} MATCHES "^(vtk.*)")
      get_target_property(imported_dependency_ ${lib_} INTERFACE_LINK_LIBRARIES)
      if(imported_dependency_)
        list(APPEND vtk_opt_libs_deps_ ${imported_dependency_})
      endif()
    endif()
  endforeach()
  vp_list_unique(vtk_opt_libs_deps_)
  list(APPEND VTK_LIBRARIES ${vtk_opt_libs_deps_})

  # list(APPEND opt_libs ${VTK_LIBRARIES})
  # Using VTK_LIBRARIES works to build visp/ustk library, examples, demos and test thanks to the components,
  # but not tutorials when they are build outside ViSP as they are stand alone CMake projects that use
  # ViSP as a 3rd party.
  # To be clear VTK_LIBRARIES contains VTK 3rd party such as vtkalglib and not /usr/local/Cellar/vtk/6.3.0/lib/libvtkalglib-6.3.1.dylib
  # full path as requested to use ViSP as 3rd party. This is the case for all VTK libraries we depend on.
  # The build of works with VTK_LIBRARIES since in that case thanks to vtkalglib properties, CMake
  # is able to find the real name and location of the libraries.
  # But when ViSP is used as a 3rd party where it should import VTK libraries, it doesn't work with
  # VTK_LIBRARIES.
  # The solution here is to get the full location of VTK_LIBRARIES libraries thanks to the properties and link
  # with these names.
  # An other way could be to include vtkConfig.cmake, but in that case, visp-config and visp.pc
  # will not be able to give the names of VTK libraries when used without CMake.

  set(config_ "NONE" "RELEASE" "DEBUG" "RELEASEWITHDEBINFO" "RELWITHDEBINFO")
  foreach(lib_ ${VTK_LIBRARIES})
    if(${lib_} MATCHES "^(vtk.*)")
      foreach(imp_config_ ${config_})
        # next line seems not useful. TODO: check if calling IMPORTED_LOCATION_ is enough
        get_target_property(lib_property_${imp_config_}_ ${lib_} IMPORTED_IMPLIB_${imp_config_})
        if(NOT EXISTS ${lib_property_${imp_config_}_})
          get_target_property(lib_property_${imp_config_}_ ${lib_} IMPORTED_LOCATION_${imp_config_})
        endif()
        # Under Unix, there is no specific suffix for PCL/VTK libraries.         # Under Windows, we add the "optimized", "debug" specific keywords
        if(WIN32 AND EXISTS "${lib_property_${imp_config_}_}" AND "${imp_config_}" MATCHES "RELEASE") # also valid for RELEASEWITHDEBINFO
          list(APPEND opt_libs optimized "${lib_property_${imp_config_}_}")
        elseif(WIN32 AND EXISTS "${lib_property_${imp_config_}_}" AND "${imp_config_}" MATCHES "DEBUG")
          list(APPEND opt_libs debug     "${lib_property_${imp_config_}_}")
        elseif(EXISTS "${lib_property_${imp_config_}_}")
          list(APPEND opt_libs "${lib_property_${imp_config_}_}")
        endif()
      endforeach()
    else() # not a vtk library
      list(APPEND opt_libs "${lib_}")
    endif()
  endforeach()

  # Instruct CMake to run moc automatically when needed.
  set(CMAKE_AUTOMOC ON)

  # Find Qt version corresponding to the Qt version used to compile VTK (4 or 5)
  if(VTK_QT_VERSION VERSION_EQUAL "5")
    find_package(Qt5 COMPONENTS Widgets QuickWidgets REQUIRED QUIET)
    qt5_wrap_ui(UI_Srcs ${UIs})

    qt5_add_resources(UI_RESOURCES resources/resources.qrc)
    set_property(SOURCE ${UI_RESOURCES} PROPERTY SKIP_AUTOMOC ON) # To avoid CMP0071 warning

    list(APPEND opt_incs "${Qt5Widgets_INCLUDE_DIRS}")
    list(APPEND opt_libs Qt5::Widgets Qt5::Gui Qt5::QuickWidgets)

  elseif(VTK_QT_VERSION VERSION_EQUAL "4")
    find_package(Qt4 REQUIRED QtGui QtCore OPTIONAL_COMPONENTS QtNetwork)
    # Use what VTK built with
    set(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
    set(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
    set(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")

    list(APPEND opt_incs "${QT_INCLUDE_DIR}")
    list(APPEND opt_incs "${QT_QTGUI_INCLUDE_DIR}")
    list(APPEND opt_incs "${QT_QTCORE_INCLUDE_DIR}")
    list(APPEND opt_incs "${QT_QTNETWORK_INCLUDE_DIR}")
    list(APPEND opt_libs ${QT_QTGUI_LIBRARIES})
    list(APPEND opt_libs ${QT_QTCORE_LIBRARIES})
    list(APPEND opt_libs ${QT_QTNETWORK_LIBRARIES})
  endif()

  # Manage resources file (for qml components)
  set(AUTORCC_ON)

  # Create ustk_gui module, only if vtk found
  vp_add_module(ustk_gui visp_ustk_core visp_ustk_elastography visp_ustk_grabber visp_robot visp_ustk_confidence_map visp_ustk_template_tracking)
  vp_glob_module_sources(${UI_RESOURCES})
  vp_module_include_directories(${opt_incs})
  vp_create_module(${opt_libs})

  vp_add_tests()

  # Add specific build flag to turn off warnings coming from VTK 3rd party
  vp_set_source_file_compile_flag(src/us2DSceneWidget.cpp -Wno-inconsistent-missing-override)
  vp_set_source_file_compile_flag(src/us3DSceneSlicing.cpp -Wno-inconsistent-missing-override)
  vp_set_source_file_compile_flag(src/us3DSceneWidget.cpp -Wno-inconsistent-missing-override)
  vp_set_source_file_compile_flag(src/usMedicalImageViewer.cpp -Wno-inconsistent-missing-override)
  vp_set_source_file_compile_flag(src/usResliceMatrixViewer.cpp -Wno-inconsistent-missing-override)
  vp_set_source_file_compile_flag(src/usViewerWidget.cpp -Wno-inconsistent-missing-override)
  vp_set_source_file_compile_flag(src/usVTKConverter.cpp -Wno-inconsistent-missing-override)

  vp_set_source_file_compile_flag(test/testUsVTKconverter.cpp -Wno-inconsistent-missing-override)

  if(APPLE)
    vp_set_source_file_compile_flag(src/usElastographyQtWrapper.cpp -Wno-unused-parameter)
    vp_set_source_file_compile_flag(src/usImageElastographyCreationWrapper.cpp -Wno-unused-parameter)
  endif()

endif()
