/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>
#include <errno.h>

#include <android/log.h>

#include <vtkAndroidRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderer.h>

#include <vtkOpenGLTexture.h>
#include <vtkTextureObject.h>

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlaneSource.h>

#include <vtkSmartPointer.h>
#include <vtkNew.h>

#include <vtk_glew.h>

#include <sys/stat.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "NativeVTK", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "NativeVTK", __VA_ARGS__))

extern "C" {
JNIEXPORT jlong JNICALL Java_co_scandy_nativevtk_NativeLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_deinit(JNIEnv * env, jobject obj);
JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_render(JNIEnv * env, jobject obj, jlong renWinP);
JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_onKeyEvent(JNIEnv * env, jobject obj, jlong udp,
                                                                      jboolean down, jint keyCode, jint metaState, jint repeatCount
);
JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_onMotionEvent(JNIEnv * env, jobject obj, jlong udp,
                                                                         jint action,
                                                                         jint eventPointer,
                                                                         jint numPtrs,
                                                                         jfloatArray xPos, jfloatArray yPos,
                                                                         jintArray ids, jint metaState);
};

class VtkView {
public:
  vtkSmartPointer<vtkRenderer> m_renderer;
  vtkSmartPointer<vtkRenderWindow> m_render_window;
  vtkSmartPointer<vtkAndroidRenderWindowInteractor> m_interactor;
  vtkSmartPointer<vtkOpenGLTexture> m_gl_texture;
  vtkSmartPointer<vtkTextureObject> m_to;
  VtkView(int width, int height);
  ~VtkView();
  void initializeTexture();
};

VtkView::VtkView(int width, int height)
 :m_interactor(vtkSmartPointer<vtkAndroidRenderWindowInteractor>::New())
{
  m_render_window = vtkSmartPointer<vtkRenderWindow>::New();
  m_renderer = vtkRenderer::New();

  m_render_window->AddRenderer(m_renderer);
  m_render_window->SetSize(width, height);
  m_renderer->SetBackground(0.6,0.4,0.2);

  char jniS[4] = {'j','n','i',0};
  m_render_window->SetWindowInfo(jniS); // tell the system that jni owns the window not us

  m_interactor->SetRenderWindow(m_render_window);

  m_interactor->Initialize();

  initializeTexture();
}

VtkView::~VtkView() {
}

void VtkView::initializeTexture(){
  vtkOpenGLRenderWindow* renWinGL= vtkOpenGLRenderWindow::SafeDownCast(m_renderer->GetRenderWindow());
  m_render_window->Render();
  renWinGL->GetTextureUnitManager();

  m_renderer->DrawOff();

  m_gl_texture = vtkSmartPointer<vtkOpenGLTexture>::New();
  m_to = vtkSmartPointer<vtkTextureObject>::New();

  m_to->SetContext(renWinGL);

  m_to->SetDepthTextureCompare(false);
  m_to->SetInternalFormat(GL_RGBA);
  m_to->SetFormat(GL_RGBA);
  m_to->SetDataType(GL_UNSIGNED_BYTE);

  int bytesPerPixel = 4;
  int width = 256;
  int height = 256;
  bool allocated;
  if( true ) {
    // Lets just fill some data to start
    size_t bytes = width * height * bytesPerPixel * sizeof(unsigned char);
    unsigned char *resultData = (unsigned char *) malloc(bytes);
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        unsigned char *pix = (unsigned char *) &resultData[(x + y * width) * 4];
        pix[0] = x;
        pix[1] = ((float) x / (float) width) * 255;
        pix[2] = y;
        pix[3] = (float) (y + x) / (float) (width + height) * 255;
      }
    }
    allocated = m_to->Create2DFromRaw(width, height, bytesPerPixel, VTK_UNSIGNED_CHAR, resultData);
    m_to->SetWrapS(vtkTextureObject::ClampToEdge);
    m_to->SetWrapT(vtkTextureObject::ClampToEdge);
    m_to->SetWrapR(vtkTextureObject::ClampToEdge);
    m_to->SetMagnificationFilter(vtkTextureObject::Linear);
    m_to->SetMinificationFilter(vtkTextureObject::Linear);
    free(resultData);
  } else {
    // allocated = m_to->Allocate2D(width, height, bytesPerPixel, VTK_UNSIGNED_CHAR);
  }

  m_gl_texture->SetTextureObject(m_to);

  // Create a plane
  vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
  plane->SetCenter(0.0, 0.0, -1.0);
  plane->SetNormal(0.0, 0.0, 1.0);

  vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  planeMapper->SetInputConnection(plane->GetOutputPort());

  vtkSmartPointer<vtkActor> texturedPlane = vtkSmartPointer<vtkActor>::New();
  texturedPlane->SetMapper(planeMapper);
  // Visualize the textured plane
  texturedPlane->SetTexture(m_gl_texture);

  m_renderer->AddActor(texturedPlane);

  m_renderer->DrawOn();
  m_renderer->ResetCamera();
}

VtkView *vtkView = nullptr;
JNIEXPORT jlong JNICALL Java_co_scandy_nativevtk_NativeLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
  vtkView = new VtkView(width, height);
  return (jlong) vtkView;
}

JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_render(JNIEnv * env, jobject obj, jlong renWinP){
  if( vtkView != nullptr ){
    vtkView->m_render_window->SwapBuffersOff();
    vtkView->m_render_window->Render();
    vtkView->m_render_window->SwapBuffersOn();
  }
}

JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_onKeyEvent(JNIEnv * env, jobject obj, jlong udp,
                                                                     jboolean down, jint keyCode, jint metaState, jint repeatCount
){

}

JNIEXPORT void JNICALL Java_co_scandy_nativevtk_NativeLib_onMotionEvent(JNIEnv * env, jobject obj, jlong udp,
                                                                        jint action,
                                                                        jint eventPointer,
                                                                        jint numPtrs,
                                                                        jfloatArray xPos, jfloatArray yPos,
                                                                        jintArray ids, jint metaState){
  int xPtr[VTKI_MAX_POINTERS];
  int yPtr[VTKI_MAX_POINTERS];
  int idPtr[VTKI_MAX_POINTERS];

  // only allow VTKI_MAX_POINTERS touches right now
  if (numPtrs > VTKI_MAX_POINTERS)
  {
  numPtrs = VTKI_MAX_POINTERS;
  }

  // fill in the arrays
  jfloat *xJPtr = env->GetFloatArrayElements(xPos, 0);
  jfloat *yJPtr = env->GetFloatArrayElements(yPos, 0);
  jint *idJPtr = env->GetIntArrayElements(ids, 0);
  for (int i = 0; i < numPtrs; ++i)
  {
  xPtr[i] = (int)xJPtr[i];
  yPtr[i] = (int)yJPtr[i];
  idPtr[i] = idJPtr[i];
  }
  env->ReleaseIntArrayElements(ids, idJPtr, 0);
  env->ReleaseFloatArrayElements(xPos, xJPtr, 0);
  env->ReleaseFloatArrayElements(yPos, yJPtr, 0);

  if( vtkView != nullptr ){
    vtkView->m_interactor->HandleMotionEvent(action, eventPointer, numPtrs, xPtr, yPtr, idPtr, metaState);
  }
}
