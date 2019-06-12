package com.yau.videoplayer.player;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * author: yau
 * time: 2019/6/12 14:29
 * desc:
 */
public class VideoPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("player-lib");
    }

    private SurfaceHolder mSurfaceHolder;
    private String mDataSource;

    public void setSurfaceView(SurfaceView surfaceView) {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.removeCallback(this);
        }
        mSurfaceHolder = surfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        nSetSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void prepare() {
        nPrepare(mDataSource);
    }

    public void setDataSource(String dataSource) {
        mDataSource = dataSource;
    }

    private native void nPrepare(String dataSource);
    private native void nSetSurface(Surface surface);
    private native void nStart();
}
