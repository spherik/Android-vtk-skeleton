package com.example.spherik.myapplication;

/**
 * Created by hcwiley on 6/27/17.
 */

public class NativeLib {
  static
  {
    System.loadLibrary("NativeVTK");
  }
  
  /**
   * @param width the current view width
   * @param height the current view height
   */
  public static native long init(int width, int height);
  public static native void render(long udp);
  public static native void onKeyEvent(long udp, boolean down, int keyCode,
                                       int metaState,
                                       int repeatCount);
  public static native void onMotionEvent(long udp,
                                          int action,
                                          int eventPointer,
                                          int numPtrs,
                                          float [] xPos, float [] yPos, int [] ids,
                                          int metaState);
  
  public static native void deinit();
}
