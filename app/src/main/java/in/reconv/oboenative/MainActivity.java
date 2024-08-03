package in.reconv.oboenative;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;

import in.reconv.oboenative.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'oboenative' library on application startup.
    static {
        System.loadLibrary("oboenative");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button startRecordingButton = findViewById(R.id.startRecordingButton);
        Button stopRecordingButton = findViewById(R.id.stopRecordingButton);
        Button startPlaybackButton = findViewById(R.id.startPlaybackButton);
        Button stopPlaybackButton = findViewById(R.id.stopPlaybackButton);

        startRecordingButton.setOnClickListener(v -> {
            stopRecordingButton.setEnabled(true);
            startRecordingButton.setEnabled(false);
            startRecordingNative();
        });
        stopRecordingButton.setOnClickListener(v -> {
            stopRecordingButton.setEnabled(false);
            startRecordingButton.setEnabled(true);
            stopRecordingNative();
        });
        startPlaybackButton.setOnClickListener(v -> {
            startPlaybackButton.setEnabled(false);
            stopPlaybackButton.setEnabled(true);
            startPlayback();
        });
        stopPlaybackButton.setOnClickListener(v -> {
            startPlaybackButton.setEnabled(true);
            stopPlaybackButton.setEnabled(false);
            stopPlayback();
        });

        //startWhiteNoise();
    }

    public native void startRecordingNative();
    public native void stopRecordingNative();
    public native void startPlayback();
    public native void stopPlayback();
}