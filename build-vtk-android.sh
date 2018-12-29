
export BUILD_DIR=${PWD}/vtk-android-buid
export VTK_SRC_DIR=${BUILD_DIR}/VTK
export VTK_BUILD_DIR=${BUILD_DIR}/vtk-build

# export ANDROID_NDK=${BUILD_DIR}/android-ndk
# export ANDROID_HOME=${BUILD_DIR}/android-sdk
# export ANDROID_NDK_ZIP=android-ndk-r18b-darwin-x86_64.zip
#
# export ANDROID_SDK_ZIP=tools_r21-macosx.zip
# export ANDROID_NDK_URL_ZIP=https://dl.google.com/android/repository/${ANDROID_NDK_ZIP}
# export ANDROID_SDK_URL_ZIP=http://dl-ssl.google.com/android/repository/${ANDROID_SDK_ZIP}

# rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
# Download and install SDK and NDK
# if [ ! -f ${BUILD_DIR}/${ANDROID_NDK_ZIP} ]; then
#   curl ${ANDROID_NDK_URL_ZIP} --output ${BUILD_DIR}/${ANDROID_NDK_ZIP}
#   unzip ${BUILD_DIR}/${ANDROID_NDK_ZIP} -d ${ANDROID_NDK}
# fi
#
# if [ ! -f ${BUILD_DIR}/${ANDROID_SDK_ZIP} ]; then
#   curl ${ANDROID_SDK_URL_ZIP} --output ${BUILD_DIR}/${ANDROID_SDK_ZIP}
#   unzip ${BUILD_DIR}/${ANDROID_SDK_ZIP} -d ${ANDROID_HOME}
# fi
# #
# #
# export ANDROID_NDK=${ANDROID_NDK}/android-ndk-r18b
# export PATH=${ANDROID_HOME}/tools:${PATH}

export ANDROID_NDK=/Users/spherik/Library/Android/sdk/ndk-bundle/

# #
# # #cd ${ANDROID_HOME}/tools
# echo y | android update sdk -u
# echo y | android update sdk -u -t 5
# echo y | android update sdk -u -t 2
# echo y | android update sdk -u -t 2 # build tools

cd ${BUILD_DIR}
git clone https://github.com/Kitware/VTK.git
cd ${BUILD_DIR}
mkdir -p $VTK_BUILD_DIR
cd $VTK_BUILD_DIR
rm -rf *
#cmake -DCMAKE_VERBOSE_MAKE=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DOPENGL_ES_VERSION=3.0 -DVTK_ANDROID_BUILD=ON -DANDROID_ARCH_ABI='armeabi-v7a' -DANDROID_NATIVE_API_LEVEL=19 ../VTK
cmake -DCMAKE_VERBOSE_MAKE=ON -DCMAKE_BUILD_TYPE=Debug -DOPENGL_ES_VERSION=3.0 -DVTK_ANDROID_BUILD=ON -DANDROID_ARCH_ABI='armeabi-v7a' -DANDROID_NATIVE_API_LEVEL=28 ../VTK
make -j8
