package com.android.babylonnative.playground;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import androidx.activity.result.ActivityResultLauncher;
//import androidx.activity.;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
//import android.support.v4.app.ActivityCompat;
//import android.support.v4.content.ContextCompat;
import android.view.View;
import android.widget.Toast;

import BabylonNative.BabylonView;

public class PlaygroundActivity extends Activity implements BabylonView.ViewDelegate {
    BabylonView mView;

    final private int REQUEST_CODE_ASK_PERMISSIONS_GEO = 2556;
    private boolean hasGeoPermission = false;

    // Activity life
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new BabylonView(getApplication(), this);

        if (ContextCompat.checkSelfPermission(
                this, Manifest.permission.ACCESS_FINE_LOCATION) ==
                PackageManager.PERMISSION_GRANTED) {
            hasGeoPermission = true;
            // You can use the API that requires the permission.
            setContentView(mView);
        } else {
            if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.ACCESS_FINE_LOCATION) !=
                    PackageManager.PERMISSION_GRANTED ||
                    ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) !=
                            PackageManager.PERMISSION_GRANTED){
                ActivityCompat.requestPermissions(this, new String[]{
                                android.Manifest.permission.ACCESS_FINE_LOCATION, android.Manifest.permission.ACCESS_COARSE_LOCATION},
                        REQUEST_CODE_ASK_PERMISSIONS_GEO);
            }
        }
            /*
            // You can directly ask for the permission.
            // The registered ActivityResultCallback gets the result of this request.
            ActivityResultLauncher<String[]> locationPermissionRequest =
                    registerForActivityResult(new ActivityResultContracts
                                    .RequestMultiplePermissions(), result -> {
                                Boolean fineLocationGranted = result.getOrDefault(
                                        Manifest.permission.ACCESS_FINE_LOCATION, false);
                                Boolean coarseLocationGranted = result.getOrDefault(
                                        Manifest.permission.ACCESS_COARSE_LOCATION,false);
                                if (fineLocationGranted != null && fineLocationGranted) {
                                    // Precise location access granted.
                                } else if (coarseLocationGranted != null && coarseLocationGranted) {
                                    // Only approximate location access granted.
                                } else {
                                    // No location access granted.
                                }
                            }
                    );
            locationPermissionRequest.launch(new String[] {
                    Manifest.permission.ACCESS_FINE_LOCATION,
                    Manifest.permission.ACCESS_COARSE_LOCATION
            });
            */
        //setContentView(mView);
    }

    @Override
    protected void onPause() {
        mView.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        switch (requestCode) {
            case REQUEST_CODE_ASK_PERMISSIONS_GEO:
                if (results[0] == PackageManager.PERMISSION_GRANTED || results[1] == PackageManager.PERMISSION_GRANTED ) {
                    hasGeoPermission = true;
                    setContentView(mView);
                } else {
                    // Permission Denied
                    Toast.makeText( this,"Location access is required to use this application." , Toast.LENGTH_SHORT)
                            .show();
                }
                break;
            default:
                if (hasGeoPermission) {
                    mView.onRequestPermissionsResult(requestCode, permissions, results);
                }
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus && mView.getVisibility() == View.GONE) {
            mView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onViewReady() {
        mView.loadScript("app:///Scripts/index.js");
    }
}
