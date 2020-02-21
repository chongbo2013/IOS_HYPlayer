//
//  circle_av_frame_queue.cpp
//  HYPlayer
//
//  Created by templechen on 2020/2/21.
//  Copyright © 2020 templechen. All rights reserved.
//

#include "circle_av_frame_queue.hpp"
#include <pthread.h>

circle_av_frame_queue::circle_av_frame_queue() {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);

    tail = new AVFrameStruct();
    AVFrameStruct *nextCursor = tail;
    AVFrameStruct *currCursor = nullptr;
    int i = DEFAULT_AV_FRAME_SIZE - 1;
    while (i > 0) {
        currCursor = new AVFrameStruct();
        currCursor->next = nextCursor;
        nextCursor = currCursor;
        i--;
    }
    head = currCursor;
    tail->next = head;
    pushCursor = head;
    pullCursor = head;
}

circle_av_frame_queue::~circle_av_frame_queue() {
    pthread_mutex_lock(&mutex);
    AVFrameStruct *temp = nullptr;
    while (head != tail) {
        if (head->data != nullptr) {
            head->data->clear();
            temp = head;
            head = head->next;
            delete temp;
        }
    }
    if (tail->next != nullptr) {
        tail->data->clear();
    }
    delete tail;
    head = nullptr;
    tail = nullptr;
    pullCursor = nullptr;
    pushCursor = nullptr;
    pthread_mutex_unlock(&mutex);
}

void circle_av_frame_queue::push(AVFrameData *frame) {
    pthread_mutex_lock(&mutex);
    if (pushCursor->next == pullCursor) {
        pthread_cond_wait(&cond, &mutex);
    }
    pushCursor->data = frame;
    pushCursor = pushCursor->next;
    currSize++;
    decodeStarted = true;
    if (currSize > DEFAULT_AV_FRAME_SIZE / 2) {
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
}

AVFrameData *circle_av_frame_queue::pull() {
    pthread_mutex_lock(&mutex);
    if (!decodeStarted) {
        pthread_cond_wait(&cond, &mutex);
    }
    if (currSize < DEFAULT_AV_FRAME_SIZE / 3) {
        pthread_cond_signal(&cond);
    }
    if (pullCursor->next != pushCursor) {
        AVFrameData *data = pullCursor->data;
        pullCursor = pullCursor->next;
        currSize--;
        pthread_mutex_unlock(&mutex);
        return data;
    } else {
//        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
        return new AVFrameData();
    }
}

int64_t circle_av_frame_queue::pullHeadFramePts() {
    if (pullCursor != nullptr && pullCursor->data != nullptr) {
        return pullCursor->data->pts;
    }
    return -1;
}