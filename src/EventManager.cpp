#include "EventManager.h"

Event::Event(const char* sp, ma_engine *en){
    soundPath = sp;
    engine = en;
    ma_result result = ma_sound_init_from_file(engine, soundPath, 0, NULL, NULL, &sound);
    cout << "INIT SOUND : " << (result == MA_SUCCESS) << endl;
    
}

Event::Event(){}

void Event::startSound(){
    ma_result result = ma_sound_start(&sound);
    cout << "START SOUND : " << (result == MA_SUCCESS) << endl;
}

void Event::stopSound(){
    ma_sound_stop(&sound);
}

EventManager::EventManager(){
    eventHistory = new map<string, bool>;
    eventHistory->insert_or_assign("walking", false);
}

void EventManager::updateSound(string id){
    if (eventHistory->at(id) == false){
        cout << "in the update sound area" << endl;
        eventHistory->insert_or_assign(id, true);
        events.at(id)->startSound();
    }
}

void EventManager::addEvent(Event e){
}

void EventManager::stopSoundM(string id){
    cout << "stopping sound" << endl;
    // eventHistory->insert_or_assign(id, false);
    // events.at(id)->stopSound();

    //ma_sound_stop();
}