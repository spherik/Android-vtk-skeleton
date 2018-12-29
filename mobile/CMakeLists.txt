# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# find_package(VTK REQUIRED)
find_package(VTK COMPONENTS
  vtkInteractionStyle
  vtkCommonDataModel
  vtkRenderingOpenGL2
  vtkRenderingFreeType
)
include(${VTK_USE_FILE})

include_directories(
  "${VTK_INCLUDE_DIRS}"
)

link_directories(
  "${VTK_LIB_DIR}"
)

set(sources "")

if( ENABLE_JAVA_ACTIVITY )
  set(sources
   src/main/cpp/native-lib.cpp
  )
else()
  include_directories(
    "${ANDROID_NDK}/platforms/android-${ANDROID_NATIVE_API_LEVEL}/arch-${ANDROID_ARCH_NAME}/usr/include"
    "${ANDROID_NDK}/sources/android/native_app_glue"
  )

  set(sources
    src/main/cpp/main.cpp
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
  )
endif()


add_library(NativeVTK SHARED ${sources})
target_link_libraries(NativeVTK
  android
  EGL
  GLESv3
  log
  ${VTK_LIBRARIES}
)
