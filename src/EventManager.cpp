#include "EventManager.h"

Event::Event(const char* sp, ma_engine *en, bool loop){
    soundPath = sp;
    engine = en;
    ma_result result = ma_sound_init_from_file(engine, soundPath, 0, NULL, NULL, &sound);
    // cout << "INIT SOUND : " << (result == MA_SUCCESS) << endl;
    looping = loop;
    
}

Event::Event(){}

void Event::startSound(){
    ma_result result;
    if (looping == true){
        ma_sound_set_looping(&sound, looping);
        ma_sound_start(&sound);
    }
    else {
        result = ma_sound_start(&sound);
    } 
    //cout << "START SOUND : " << (result == MA_SUCCESS) << endl;
}

void Event::stopSound(){
    ma_sound_stop(&sound);
}

EventManager::EventManager(){
    eventHistory = new map<string, bool>;
    eventHistory->insert_or_assign("walking", false);
    eventHistory->insert_or_assign("collection", false);
}

void EventManager::triggerSound(string id){
    if (eventHistory->at(id) == false){
       // cout << "confirmed that walking is false, setting to true" << endl;
        eventHistory->insert_or_assign(id, true);
        events.at(id)->startSound();
    }
}

void EventManager::addEvent(Event e){
}

void EventManager::stopSound(string id){
   // cout << "stopping sound" << endl;
    eventHistory->insert_or_assign(id, false);
    events.at(id)->stopSound();
    //ma_sound_stop();
}