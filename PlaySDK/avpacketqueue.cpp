#include "avpacketqueue.h"

AvPacketQueue::AvPacketQueue()
{
    mutex   = SDL_CreateMutex();
    cond    = SDL_CreateCond();
}

void AvPacketQueue::enqueue(AVPacket *packet)
{
    SDL_LockMutex(mutex);

    m_queue.push(*packet);

    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);
}

bool AvPacketQueue::dequeue(AVPacket& packet, bool isBlock)
{
	bool bRet = false;

    SDL_LockMutex(mutex);
    while (true)
	{
        if (!m_queue.empty())
		{
           packet = m_queue.front();
            m_queue.pop();

			bRet = true;

			break;
        }
	
		if (!isBlock)
		{
            break;
        }
		
		SDL_CondWait(cond, mutex);
    }
    SDL_UnlockMutex(mutex);

	return bRet;
}

void AvPacketQueue::empty()
{
    SDL_LockMutex(mutex);
    while (!m_queue.empty())
	{
        AVPacket& packet = m_queue.front();
		av_packet_unref(&packet);
        m_queue.pop();
    }
	SDL_UnlockMutex(mutex);
}

bool AvPacketQueue::isEmpty()
{
    return m_queue.empty();
}

size_t AvPacketQueue::queueSize()
{
    return m_queue.size();
}
