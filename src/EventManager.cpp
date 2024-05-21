#include "EventManager.h"

Event::Event(const char* sp, ma_engine *en){
    soundPath = sp;
    engine = en;
    ma_sound_init_from_file(engine, soundPath, 0, NULL, NULL, &sound);

}

Event::Event(){}

// void Event::initSound(){
//     ma_sound_init_from_file(engine, soundPath, 0, NULL, NULL, &sound);
// }

void Event::startSound(){
    ma_sound_start(&sound);
}