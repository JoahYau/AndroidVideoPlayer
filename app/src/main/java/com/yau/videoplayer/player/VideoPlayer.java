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

    public void start() {
        nStart();
    }

    public int getDuration() {
        return nGetDuration();
    }

    private native void nPrepare(String dataSource);
    private native void nSetSurface(Surface surface);
    private native void nStart();
    private native int nGetDuration();

    public void onPrepare() {
        if (mOnPrepareListener != null) {
            mOnPrepareListener.onPrepare();
        }
    }

    public void onProgress(int progress) {
        if (mOnProgressListener != null) {
            mOnProgressListener.onProgress(progress);
        }
    }

    public void onError(int errorCode) {
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(errorCode);
        }
    }

    // region 接口
    private OnPrepareListener mOnPrepareListener;
    private OnProgressListener mOnProgressListener;
    private OnErrorListener mOnErrorListener;

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        mOnPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        mOnProgressListener = onProgressListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        mOnErrorListener = onErrorListener;
    }

    public interface OnPrepareListener {
        void onPrepare();
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public interface OnErrorListener {
        void onError(int errorCode);
    }
    // endregion
}
