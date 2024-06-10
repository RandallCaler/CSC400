#include "EventManager.h"

Event::Event(const char* sp, ma_engine *en, bool loop, string type){
    soundPath = sp;
    engine = en;
    ma_result result = ma_sound_init_from_file(engine, soundPath, 0, NULL, NULL, &sound);
    // cout << "INIT SOUND : " << (result == MA_SUCCESS) << endl;
    looping = loop;
    id = type;
    ma_sound_get_length_in_pcm_frames(&sound, &soundDuration);  
    startTime = 0;
}

Event::Event(){}

void Event::startSound(){
    if (!isPlaying()){
        ma_result result;
        if (looping == true){
            ma_sound_set_looping(&sound, looping);
            ma_sound_start(&sound);
        }
        else {
            result = ma_sound_start(&sound);
        } 
        ma_sound_get_cursor_in_pcm_frames(&sound, &startTime);
        //cout << "START SOUND : " << (result == MA_SUCCESS) << endl;
    }
}

void Event::rewindSound(){
    ma_sound_seek_to_pcm_frame(&sound, 0);
}

bool Event::isPlaying(){
    return (MA_TRUE == ma_sound_is_playing(&sound));
}

bool Event::stopSound(){
   if (isPlaying()) {
        if ((id != "walking") && (id != "background")) {
            ma_uint64 currentFrame;
            ma_sound_get_cursor_in_pcm_frames(&sound, &currentFrame);

            if (currentFrame - startTime >= soundDuration) {
                cout << "premature stop"<< endl;
                ma_sound_stop(&sound);
                return false;
            }

        } else {  
            ma_sound_stop(&sound);
            return false;
        }
        return true;
    }
    return false;
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
        if(id != "walking"){
            events.at(id)->rewindSound();
        }
        events.at(id)->startSound();
    }
}

void EventManager::addEvent(Event e){
}

void EventManager::stoppingSound(string id){
   // cout << "stopping sound" << endl;
    bool check = events.at(id)->stopSound();
    eventHistory->insert_or_assign(id, check);   

}