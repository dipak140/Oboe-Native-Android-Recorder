package in.reconv.oboenativemodule;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

public enum LiveEffectEngine {

    INSTANCE;

    // Load native library
    static {
        System.loadLibrary("oboenativemodule");
    }

    // Native methods
    public static native boolean create();
    public static native boolean isAAudioRecommended();
    public static native boolean setAPI(int apiType);
    public static native boolean setEffectOn(boolean isEffectOn, String fullPathToFile);
    public static native void setRecordingDeviceId(int deviceId);
    public static native void setPlaybackDeviceId(int deviceId);
    public static native void delete();
    static native void native_setDefaultStreamValues(int defaultSampleRate, int defaultFramesPerBurst);
    public static native void setVolume(float volume);

    public static void setDefaultStreamValues(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1){
            AudioManager myAudioMgr = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            String sampleRateStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            int defaultSampleRate = Integer.parseInt(sampleRateStr);
            String framesPerBurstStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            int defaultFramesPerBurst = Integer.parseInt(framesPerBurstStr);

            native_setDefaultStreamValues(defaultSampleRate, defaultFramesPerBurst);
        }
    }
}
