package com.example.spherik.myapplication;

import android.content.Context;
import android.graphics.PixelFormat;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;


/**
 * TODO: document your custom view class.
 */
public class VtkView extends GLSurfaceView {
  private static String TAG = "VtkView";
  private Renderer myRenderer;

  public VtkView(Context context) {
    super(context);
    init(null, 0);
  }

  public VtkView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init(attrs, 0);
  }

  @Override
  public void onDetachedFromWindow(){
    super.onDetachedFromWindow();
    NativeLib.deinit();
  }


  private void init(AttributeSet attrs, int defStyle) {
    setFocusable(true);
    setFocusableInTouchMode(true);

    this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
    setEGLContextFactory(new ContextFactory());
    setEGLConfigChooser(new ConfigChooser(8, 8, 8, 0, 8, 0));

        /* Set the renderer responsible for frame rendering */
    this.myRenderer = new Renderer();
    this.setRenderer(myRenderer);
    this.setRenderMode(RENDERMODE_WHEN_DIRTY);
//    this.setRenderMode(RENDERMODE_CONTINUOUSLY);
  }
  private static class ContextFactory implements EGLContextFactory
  {
    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig)
    {
      Log.w(TAG, "creating OpenGL ES 3.0 context");
      checkEglError("Before eglCreateContext", egl);
      int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL10.EGL_NONE };
      EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
      checkEglError("After eglCreateContext", egl);
      return context;
    }

    public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context)
    {
      egl.eglDestroyContext(display, context);
    }
  }

  private static void checkEglError(String prompt, EGL10 egl)
  {
    int error;
    while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS)
    {
      Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
    }
  }

  private static class ConfigChooser implements EGLConfigChooser
  {
    public ConfigChooser(int r, int g, int b, int a, int depth, int stencil)
    {
      mRedSize = r;
      mGreenSize = g;
      mBlueSize = b;
      mAlphaSize = a;
      mDepthSize = depth;
      mStencilSize = stencil;
    }

    /* This EGL config specification is used to specify 2.0 rendering.
     * We use a minimum size of 4 bits for red/green/blue, but will
     * perform actual matching in chooseConfig() below.
     */
    private static int EGL_OPENGL_ES2_BIT = 4;
    private static int[] s_configAttribs2 =
        {
            EGL10.EGL_RED_SIZE, 4,
            EGL10.EGL_GREEN_SIZE, 4,
            EGL10.EGL_BLUE_SIZE, 4,
            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL10.EGL_NONE
        };

    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display)
    {
      // Get the number of minimally matching EGL configurations
      int[] num_config = new int[1];
      egl.eglChooseConfig(display, s_configAttribs2, null, 0, num_config);

      int numConfigs = num_config[0];

      if (numConfigs <= 0)
      {
        throw new IllegalArgumentException("No configs match configSpec");
      }

      // Allocate then read the array of minimally matching EGL configs
      EGLConfig[] configs = new EGLConfig[numConfigs];
      egl.eglChooseConfig(display, s_configAttribs2, configs, numConfigs, num_config);

      // Now return the "best" one
      return chooseConfig(egl, display, configs);
    }

    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
                                  EGLConfig[] configs)
    {
      for(EGLConfig config : configs)
      {
        int d = findConfigAttrib(egl, display, config,
            EGL10.EGL_DEPTH_SIZE, 0);
        int s = findConfigAttrib(egl, display, config,
            EGL10.EGL_STENCIL_SIZE, 0);

        // We need at least mDepthSize and mStencilSize bits
        if (d < mDepthSize || s < mStencilSize)
          continue;

        // We want an *exact* match for red/green/blue/alpha
        int r = findConfigAttrib(egl, display, config,
            EGL10.EGL_RED_SIZE, 0);
        int g = findConfigAttrib(egl, display, config,
            EGL10.EGL_GREEN_SIZE, 0);
        int b = findConfigAttrib(egl, display, config,
            EGL10.EGL_BLUE_SIZE, 0);
        int a = findConfigAttrib(egl, display, config,
            EGL10.EGL_ALPHA_SIZE, 0);

        if (r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize)
          return config;
      }
      return null;
    }

    private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                                 EGLConfig config, int attribute, int defaultValue)
    {
      if (egl.eglGetConfigAttrib(display, config, attribute, mValue))
      {
        return mValue[0];
      }
      return defaultValue;
    }

    // Subclasses can adjust these values:
    protected int mRedSize;
    protected int mGreenSize;
    protected int mBlueSize;
    protected int mAlphaSize;
    protected int mDepthSize;
    protected int mStencilSize;
    private int[] mValue = new int[1];
  }

  private static class Renderer implements GLSurfaceView.Renderer
  {
    private long vtkContext;

    public void onDrawFrame(GL10 gl)
    {
      NativeLib.render(0);
    }

    // forward events to VTK for it to handle
    public void onKeyEvent(boolean down, KeyEvent ke)
    {
      NativeLib.onKeyEvent(vtkContext, down, ke.getKeyCode(),
          ke.getMetaState(),
          ke.getRepeatCount());
    }

    // forward events to VTK for it to handle
    public void onMotionEvent(final MotionEvent me)
    {
      try {
        int numPtrs = me.getPointerCount();
        float [] xPos = new float[numPtrs];
        float [] yPos = new float[numPtrs];
        int [] ids = new int[numPtrs];
        for (int i = 0; i < numPtrs; ++i)
        {
          ids[i] = me.getPointerId(i);
          xPos[i] = me.getX(i);
          yPos[i] = me.getY(i);
        }

        int actionIndex = me.getActionIndex();
        int actionMasked = me.getActionMasked();
        int actionId = me.getPointerId(actionIndex);

        if (actionMasked != 2)
        {
          Log.e(TAG, "Got action " + actionMasked + " on index " + actionIndex + " has id " + actionId);
        }
        NativeLib.onMotionEvent(vtkContext,
            actionMasked,
            actionId,
            numPtrs, xPos, yPos, ids,
            me.getMetaState());
      } catch (IllegalArgumentException e) {
        Log.e(TAG, "Bogus motion event");
      }
    }

    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
      vtkContext = NativeLib.init(width, height);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
      // Do nothing.
    }
  }

  // forward events to rendering thread for it to handle
  public boolean onKeyUp(int keyCode, KeyEvent event)
  {
    final KeyEvent keyEvent = event;
    queueEvent(new Runnable()
    {
      public void run()
      {
        myRenderer.onKeyEvent(false, keyEvent);
      }
    });
    return true;
  }

  // forward events to rendering thread for it to handle
  public boolean onKeyDown(int keyCode, KeyEvent event)
  {
    final KeyEvent keyEvent = event;
    queueEvent(new Runnable()
    {
      public void run()
      {
        myRenderer.onKeyEvent(true, keyEvent);
      }
    });
    return true;
  }

  // forward events to rendering thread for it to handle
  public boolean onTouchEvent(MotionEvent event)
  {
    final MotionEvent motionEvent = event;
    queueEvent(new Runnable()
    {
      public void run()
      {
        myRenderer.onMotionEvent(motionEvent);
      }
    });
    return true;
  }
}
