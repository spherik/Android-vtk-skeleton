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

#include <vtk_glew.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "NativeVTK", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "NativeVTK", __VA_ARGS__))

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state)
{
  // Make sure glue isn't stripped.
  app_dummy();

  vtkNew<vtkRenderWindow> renWin;
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkAndroidRenderWindowInteractor> iren;

  // this line is key, it provides the android
  // state to VTK
  iren->SetAndroidApplication(state);

  renWin->AddRenderer(renderer.Get());
  iren->SetRenderWindow(renWin.Get());

  renderer->SetBackground(0.4,0.7,0.6);

  vtkSmartPointer<vtkOpenGLTexture> m_gl_texture = vtkSmartPointer<vtkOpenGLTexture>::New();
  vtkSmartPointer<vtkTextureObject> m_to;

  // Make sure we've got a OpenGL render window
  vtkOpenGLRenderWindow* renWinGL= vtkOpenGLRenderWindow::SafeDownCast(renderer->GetRenderWindow());
  renWin->Render();
  renWinGL->GetTextureUnitManager();

  renderer->DrawOff();

  int *size = renWin->GetSize();
  int width = size[0];
  int height = size[1];

  // Get a texture object
  m_to = vtkSmartPointer<vtkTextureObject>::New();
  m_to->SetContext(renWinGL);

  // by default the textures have depth comparison on
  // but for simple display we need to turn it off
  m_to->SetDepthTextureCompare(false);
  m_to->SetInternalFormat(GL_RGBA);
  m_to->SetFormat(GL_RGBA);
  m_to->SetDataType(GL_UNSIGNED_BYTE);
  // to->DebugOn();

  int bytesPerPixel = 4;
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
    allocated = m_to->Allocate2D(width, height, bytesPerPixel, VTK_UNSIGNED_CHAR);
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
  texturedPlane->SetTexture(m_gl_texture);

  // Visualize the textured plane
  renderer->AddActor(texturedPlane);

  renderer->ResetCamera();

  renderer->DrawOn();

  renWin->Render();
  iren->Start();
}
