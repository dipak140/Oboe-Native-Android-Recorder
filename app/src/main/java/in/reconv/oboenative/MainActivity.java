package in.reconv.oboenative;

import static android.os.Environment.getExternalStorageDirectory;
import static in.reconv.oboenativemodule.DuplexStreamForegroundService.ACTION_START;
import static in.reconv.oboenativemodule.DuplexStreamForegroundService.ACTION_STOP;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

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
import android.media.MediaPlayer;

import in.reconv.oboenative.databinding.ActivityMainBinding;
import in.reconv.oboenativemodule.NativeLib;

import in.reconv.oboenativemodule.LiveEffectEngine;
import in.reconv.oboenativemodule.DuplexStreamForegroundService;

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
    private static final int MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 100;
    private String filePath;
    private MediaPlayer mediaPlayer;



    // Full path that is going to be sent to C++ through JNI ("/storage/emulated/0/Recorders/record.wav")

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE);
        }

        setContentView(R.layout.activity_main);
        NativeLib.letsdosomting();
        Button startFeedbackButton = findViewById(R.id.startFeedbackButton);
        Button stopFeedbackButton = findViewById(R.id.stopFeedbackButton);
        Button playRecordingButton = findViewById(R.id.playRecordingButton);

        startFeedbackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LiveEffectEngine.setAPI(apiSelection);
                startEffect();
            }
        });

        stopFeedbackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                stopEffect();
            }
        });

        playRecordingButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                playRecording();
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

    private void playRecording() {
        if (filePath != null) {
            mediaPlayer = new MediaPlayer();
            try {
                mediaPlayer.setDataSource(filePath);
                mediaPlayer.prepare();
                mediaPlayer.start();
                mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                    @Override
                    public void onCompletion(MediaPlayer mp) {
                        mp.release();
                        mediaPlayer = null;
                    }
                });
            } catch (Exception e) {
                e.printStackTrace();
                Toast.makeText(this, "Error playing recording", Toast.LENGTH_SHORT).show();
            }
        } else {
            Toast.makeText(this, "No recording found", Toast.LENGTH_SHORT).show();
        }
    }
    private void startEffect() {
        Log.d(TAG, "Attempting to start");

        if (!isRecordPermissionGranted()){
            requestRecordPermission();
            return;
        }

        String timestamp = String.valueOf(System.currentTimeMillis());
        filePath = getExternalFilesDir(null) + "/" + timestamp + "_audio_recording.wav";
        boolean success = LiveEffectEngine.setEffectOn(true, filePath);
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
        LiveEffectEngine.setEffectOn(false, filePath);
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