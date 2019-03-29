#ifndef AVPACKETQUEUE_H
#define AVPACKETQUEUE_H

#include "inc.h"

#include <queue>
using namespace std;

class AvPacketQueue
{
public:
    explicit AvPacketQueue();

    void enqueue(AVPacket *packet);

    bool dequeue(AVPacket& packet, bool isBlock);

    bool isEmpty();

    void empty();

    size_t queueSize();

private:
    SDL_mutex *mutex = NULL;
    SDL_cond *cond = NULL;

    queue<AVPacket> m_queue;
};

#endif // AVPACKETQUEUE_H
