// Generated by view binder compiler. Do not edit!
package in.reconv.oboenative.databinding;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewbinding.ViewBinding;
import androidx.viewbinding.ViewBindings;
import in.reconv.oboenative.R;
import java.lang.NullPointerException;
import java.lang.Override;
import java.lang.String;

public final class ActivityMainBinding implements ViewBinding {
  @NonNull
  private final LinearLayout rootView;

  @NonNull
  public final TextView sampleText;

  @NonNull
  public final Button startPlaybackButton;

  @NonNull
  public final Button startRecordingButton;

  @NonNull
  public final Button stopPlaybackButton;

  @NonNull
  public final Button stopRecordingButton;

  private ActivityMainBinding(@NonNull LinearLayout rootView, @NonNull TextView sampleText,
      @NonNull Button startPlaybackButton, @NonNull Button startRecordingButton,
      @NonNull Button stopPlaybackButton, @NonNull Button stopRecordingButton) {
    this.rootView = rootView;
    this.sampleText = sampleText;
    this.startPlaybackButton = startPlaybackButton;
    this.startRecordingButton = startRecordingButton;
    this.stopPlaybackButton = stopPlaybackButton;
    this.stopRecordingButton = stopRecordingButton;
  }

  @Override
  @NonNull
  public LinearLayout getRoot() {
    return rootView;
  }

  @NonNull
  public static ActivityMainBinding inflate(@NonNull LayoutInflater inflater) {
    return inflate(inflater, null, false);
  }

  @NonNull
  public static ActivityMainBinding inflate(@NonNull LayoutInflater inflater,
      @Nullable ViewGroup parent, boolean attachToParent) {
    View root = inflater.inflate(R.layout.activity_main, parent, false);
    if (attachToParent) {
      parent.addView(root);
    }
    return bind(root);
  }

  @NonNull
  public static ActivityMainBinding bind(@NonNull View rootView) {
    // The body of this method is generated in a way you would not otherwise write.
    // This is done to optimize the compiled bytecode for size and performance.
    int id;
    missingId: {
      id = R.id.sample_text;
      TextView sampleText = ViewBindings.findChildViewById(rootView, id);
      if (sampleText == null) {
        break missingId;
      }

      id = R.id.startPlaybackButton;
      Button startPlaybackButton = ViewBindings.findChildViewById(rootView, id);
      if (startPlaybackButton == null) {
        break missingId;
      }

      id = R.id.startRecordingButton;
      Button startRecordingButton = ViewBindings.findChildViewById(rootView, id);
      if (startRecordingButton == null) {
        break missingId;
      }

      id = R.id.stopPlaybackButton;
      Button stopPlaybackButton = ViewBindings.findChildViewById(rootView, id);
      if (stopPlaybackButton == null) {
        break missingId;
      }

      id = R.id.stopRecordingButton;
      Button stopRecordingButton = ViewBindings.findChildViewById(rootView, id);
      if (stopRecordingButton == null) {
        break missingId;
      }

      return new ActivityMainBinding((LinearLayout) rootView, sampleText, startPlaybackButton,
          startRecordingButton, stopPlaybackButton, stopRecordingButton);
    }
    String missingId = rootView.getResources().getResourceName(id);
    throw new NullPointerException("Missing required view with ID: ".concat(missingId));
  }
}
