#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <iostream>
#include <queue>

#include "../ext/miniaudio/miniaudio.h"

using namespace std;

/*

useful mini audio advice 

- Use ma_sound_seek_to_pcm_frame(&sound, 0) to seek back to the start of a sound

- ma_engine_listener_set_position(&engine, listenerIndex, worldPosX, worldPosY, worldPosZ);
- ma_engine_listener_set_direction(&engine, listenerIndex, forwardX, forwardY, forwardZ);
- ma_engine_listener_set_world_up(&engine, listenerIndex, 0, 1, 0);
- ma_engine_listener_set_cone(&engine, listenerIndex, innerAngleInRadians, outerAngleInRadians, outerGain);

*/

class Event {
    // is it continuous (holding down key) or is it press-release
    public: 
        bool continuous;
        bool activated;
        const char *soundPath;
        int weight;
        ma_engine *engine;
        ma_sound sound;

        Event(const char *sp, ma_engine *en);
        Event();
        // void initSound();
        void startSound();

        // redefine the comparison attribute so events with higher priority play first
        bool operator<(const Event& other) const {
            return weight > other.weight;
        }
};

class EventManager {
    priority_queue<Event> pq;
    // void addEvent();
};

#endif