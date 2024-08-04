package in.reconv.oboenative;

import static in.reconv.oboenative.DuplexStreamForegroundService.ACTION_START;
import static in.reconv.oboenative.DuplexStreamForegroundService.ACTION_STOP;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import in.reconv.oboenative.databinding.ActivityMainBinding;

public class MainActivity extends Activity implements
        ActivityCompat.OnRequestPermissionsResultCallback {

    // Used to load the 'oboenative' library on application startup.

    private static final int OBOE_API_AAUDIO = 0;
    private ActivityMainBinding binding;
    private boolean isPlaying = false;
    private int apiSelection = OBOE_API_AAUDIO;
    private boolean mAAudioRecommended;
    private static final int AUDIO_EFFECT_REQUEST = 0;
    private static final String TAG = MainActivity.class.getName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        setContentView(R.layout.activity_main);

        Button startFeedbackButton = findViewById(R.id.startFeedbackButton);
        Button stopFeedbackButton = findViewById(R.id.stopFeedbackButton);

        startFeedbackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LiveEffectEngine.setAPI(apiSelection);
                startEffect();
            }
        });

        LiveEffectEngine.setRecordingDeviceId(2);
        LiveEffectEngine.setPlaybackDeviceId(2);

        // api selection audio/openes SL
        apiSelection = OBOE_API_AAUDIO; // let's assume that it works there
        LiveEffectEngine.setDefaultStreamValues(this);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
        // Start a foreground service
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Intent serviceIntent = new Intent(ACTION_START, null, this,
                    DuplexStreamForegroundService.class);
            startForegroundService(serviceIntent);
        }

        onStartTest();

    }

    private void onStartTest() {
        LiveEffectEngine.create();
        mAAudioRecommended = LiveEffectEngine.isAAudioRecommended();
        LiveEffectEngine.setAPI(apiSelection);
    }

    public void toggleEffect() {
        if (isPlaying) {
            stopEffect();
        } else {
            LiveEffectEngine.setAPI(apiSelection);
            startEffect();
        }
    }

    private void startEffect() {
        Log.d(TAG, "Attempting to start");

        if (!isRecordPermissionGranted()){
            requestRecordPermission();
            return;
        }

        boolean success = LiveEffectEngine.setEffectOn(true);
        if (success) {
            isPlaying = true;
        } else {
            isPlaying = false;
        }
    }

    private void onStopTest() {
        stopEffect();
        LiveEffectEngine.delete();
    }

    private void stopEffect() {
        LiveEffectEngine.setEffectOn(false);
        isPlaying = false;
    }

    private boolean isRecordPermissionGranted() {
        return (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.RECORD_AUDIO) ==
                PackageManager.PERMISSION_GRANTED);
    }

    private void requestRecordPermission(){
        ActivityCompat.requestPermissions(
                this,
                new String[]{Manifest.permission.RECORD_AUDIO},
                AUDIO_EFFECT_REQUEST);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {

        if (AUDIO_EFFECT_REQUEST != requestCode) {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }

        if (grantResults.length != 1 ||
                grantResults[0] != PackageManager.PERMISSION_GRANTED) {

            // User denied the permission, without this we cannot record audio
            // Show a toast and update the status accordingly
            Toast.makeText(getApplicationContext(),
                            "R.string.need_record_audio_permission",
                            Toast.LENGTH_SHORT)
                    .show();
        } else {
            // Permission was granted, start live effect
            toggleEffect();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
    }
    @Override
    protected void onResume() {
        super.onResume();
    }
    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        onStopTest();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Intent serviceIntent = new Intent(ACTION_STOP, null, this,
                    DuplexStreamForegroundService.class);
            startForegroundService(serviceIntent);
        }
        super.onDestroy();
    }


}