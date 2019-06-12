package com.yau.videoplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private SurfaceView mSvVideoPlayer;
    private SeekBar mSeekBar;
    private Button mBtnPlayPause;

    private boolean mIsPlaying = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();
        initEvent();
    }

    private void initView() {
        mSvVideoPlayer = findViewById(R.id.sv_video_player);
        mSeekBar = findViewById(R.id.seek_bar);
        mBtnPlayPause = findViewById(R.id.btn_play);
    }

    private void initEvent() {
        mBtnPlayPause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mIsPlaying) {
                    mIsPlaying = false;
                    mBtnPlayPause.setText("播放");
                    play();
                } else {
                    mIsPlaying = true;
                    mBtnPlayPause.setText("暂停");
                    pause();
                }
            }
        });
    }

    private void play() {

    }

    private void pause() {

    }
}
