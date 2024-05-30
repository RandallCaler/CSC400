#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <iostream>
#include <queue>
#include <map>

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
        int id; // each event should have a different ID so that the event manager will not repeat the sound play

        Event(const char *sp, ma_engine *en);
        Event();
        // void initSound();
        void startSound();
        void stopSound();

        // // redefine the comparison attribute so events with higher priority play first
        // bool operator<(const Event& other) const {
        //     return weight > other.weight;
        // }
};

class EventManager {
    // 1. have a buffer of two histories 2 index array 
    // each event has an event buffer, where did things true 1 
    // check if things change between render calls

    // take in when things should play
    // lambda function that check conidtions, pass boolean to event manager to determine whihch event to call

    public:
        EventManager();
        void updateSound(string id);
        void stopSoundM(string id);
        void addEvent(Event e);
    // void addEvent();

        map<string, bool> *eventHistory = new map<string, bool>;
        map<string, Event*> events;

};

#endif