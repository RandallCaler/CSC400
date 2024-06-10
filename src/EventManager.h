#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <iostream>
#include <queue>
#include <map>

#include "../ext/miniaudio/miniaudio.h"

using namespace std;

/*

useful mini audio advice 
ma_sound_set_volume()

- Use ma_sound_seek_to_pcm_frame(&sound, 0) to seek back to the start of a sound
- ma_engine_listener_set_position(&engine, listenerIndex, worldPosX, worldPosY, worldPosZ);
- ma_engine_listener_set_direction(&engine, listenerIndex, forwardX, forwardY, forwardZ);
- ma_engine_listener_set_world_up(&engine, listenerIndex, 0, 1, 0);
- ma_engine_listener_set_cone(&engine, listenerIndex, innerAngleInRadians, outerAngleInRadians, outerGain);

*/

class Event {
    public: 
        bool looping;
        const char *soundPath;
        ma_uint64 soundDuration;
        ma_uint64 startTime;
        ma_engine *engine;
        ma_sound sound;
        string id; // each event should have a different ID so that the event manager will not repeat the sound play

        Event(const char *sp, ma_engine *en, bool looping, string id);
        Event();
        void startSound();
        bool stopSound();
        void rewindSound();
        bool isPlaying();
};

class EventManager {
    // 1. have a buffer of two histories 2 index array 
    // each event has an event buffer, where did things true 1 
    // check if things change between render calls

    public:
        EventManager();
        void triggerSound(string id);
        void stoppingSound(string id);
        void addEvent(Event e);
    // void addEvent();

        map<string, bool> *eventHistory;
        map<string, Event*> events;

};

#endif