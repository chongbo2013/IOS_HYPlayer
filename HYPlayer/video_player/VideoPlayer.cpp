//
//  VideoPlayer.cpp
//  HYPlayer
//
//  Created by templechen on 2020/2/21.
//  Copyright © 2020 templechen. All rights reserved.
//

#include "VideoPlayer.hpp"

VideoPlayer::VideoPlayer(const char *path, bool usingMediaCodec) {
    packetQueue = new circle_av_packet_queue();
    frameQueue = new circle_av_frame_queue();
    demuxLooper = new DemuxLooper(packetQueue, false);
    decodeLooper = new DecodeLooper(frameQueue, packetQueue, false, usingMediaCodec);
    demuxLooper->sendMessage(demuxLooper->kMsgDemuxCreated, (void *) path);
    decodeLooper->sendMessage(decodeLooper->kMsgDecodeCreated);
    glVideoLooper = new GLVideoLooper(frameQueue);
    glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceCreated);
    enable = true;
}

VideoPlayer::~VideoPlayer() {

}

void VideoPlayer::start() {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceStart);
    }
}

void VideoPlayer::pause() {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfacePause);
    }
}

void VideoPlayer::seek(long pos) {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceSeek);
    }
    if (demuxLooper != nullptr) {
        demuxLooper->demux->isDemuxing = false;
        demuxLooper->sendMessage(demuxLooper->kMsgDemuxSeek, reinterpret_cast<void *>(pos));
    }
}

void VideoPlayer::release() {
    enable = false;
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceDestroyed);
    }
    if (demuxLooper != nullptr) {
        demuxLooper->demux->isDemuxing = false;
        demuxLooper->demux->isOver = true;
        demuxLooper->sendMessage(demuxLooper->kMsgDemuxRelease);
    }
    if (decodeLooper != nullptr) {
        decodeLooper->sendMessage(decodeLooper->kMsgDecodeRelease);
    }
    delete this;
}

// thread unsafe
long VideoPlayer::getTotalDuration() {
    if (enable && demuxLooper != nullptr) {
        return demuxLooper->getTotalDuration();
    }
    return 0;
}

//thread unsafe
long VideoPlayer::getCurrentDuration() {
    if (enable && glVideoLooper != nullptr) {
        return glVideoLooper->getCurrentPos();
    }
    return 0;
}

void VideoPlayer::setNativeWindowCreated(void *layer) {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceCreated, layer);
    }
}

void VideoPlayer::setNativeWindowDestroyed() {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceDestroyed);
        glVideoLooper->quit();
    }
}

void VideoPlayer::setNativeWindowChanged(int width, int height) {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceChanged, width, height);
    }
}

void VideoPlayer::doFrame() {
    if (glVideoLooper != nullptr) {
        glVideoLooper->sendMessage(glVideoLooper->kMsgSurfaceDoFrame);
    }
}
