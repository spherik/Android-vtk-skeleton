package com.example.spherik.myapplication;

import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity {

  VtkView mView;

  @Override protected void onCreate(Bundle icicle)
  {
    super.onCreate(icicle);
    mView = new VtkView(getApplication());
    this.setContentView(mView);
  }

  @Override protected void onPause()
  {
    super.onPause();
    this.mView.onPause();
  }

  @Override protected void onResume()
  {
    super.onResume();
    this.mView.onResume();
  }
}
