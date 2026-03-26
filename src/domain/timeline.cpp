#include "timeline.hpp"
#include "log.hpp"
/*
quem store framebuffer?
- não é o clip pq é objeto de dominio, não deve ser acoplado com o visual
- nao é o videoref pq o framebuffer muda pra cada clipe

clip, videoref
get_clip_frame(timestamp, clip, videoref): uint8_t
get_clip_tex(timestamp, clip, videoref): 
*/
static bool debug = false;

Clip* Timeline::add_clip(size_t track, float time0, float time1){
    int _id = 0;
    for(std::list<Track>::iterator it = tracks_.begin(); it != tracks_.end(); ++it){
        if (_id == (int)track){
            (*it).clips.push_back(std::make_unique<Clip>(time0, time1));
            return (*it).clips.back().get();
        }
        _id++;
    }
    Track* t = &this->tracks_.emplace_back();
    t->id = tracks_.size()-1;
    t->clips.push_back(std::make_unique<Clip>(time0, time1));
    return t->clips.back().get();

}


void Timeline::update(double dt){
        log("update\n");
    this->playhead_time += (float)dt;
    //log("update %f\n", playhead_time);
    // track de baixo pra cima
    // pra cada track, criar uma lista de clipes na ordem
    // o resultado da iteração vai ser oq? (preciso implementar em Clip)



}
Timeline::Timeline(){
    this->init_shader();
}
void Timeline::init_shader(){
  glGenTextures(1, &this->playhead_tex);

}

Timeline::~Timeline(){
    glDeleteTextures(1, &this->playhead_tex);
}

void Timeline::key_callback(int key, int action){
    bool wasPlaying = this->isPlaying;
    if (key == GLFW_KEY_RIGHT){
        if (action == GLFW_PRESS){
            this->playhead_time += 3.0;
            ////log("key press right\n");
        } else if (action == GLFW_REPEAT){
            this->isPlaying = false;
            this->playhead_time += 1;
        }
        // this->pts = (int64_t)(playhead_time / this->selected_video->get_time_base());
        // this->selected_video->seek_frame(this->pts);
    }
    if (key == GLFW_KEY_LEFT){
        if (action == GLFW_PRESS){
            this->playhead_time -= 3.0;
            
        } else if (action == GLFW_REPEAT){
            this->isPlaying = false;
            this->playhead_time -= 1;
        }
        
    }

    if (key == GLFW_KEY_R){
        this->playhead_time = 0;
        
    }

    this->isPlaying = wasPlaying;
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        ////log("key space\n");
        this->isPlaying = !this->isPlaying;
    }
}

void VideoClip::get_tex(std::function<void(Clip*, MasterClip*, float)> get_tex, Clip* clip, float rel_ts){
    get_tex(clip, this, rel_ts);
}


bool VideoClip::can_tl_move(ImVec2 disp, ImVec2 time){
    return disp.y + time.y > disp.x + time.x; // !!!
    double dur = static_cast<double>(this->reader.state.av_format_context->duration) / AV_TIME_BASE;
    float durwant = (time.y+disp.y -time.x-disp.x);
    log("avdur %"PRIu64" dur %f want %f base %d\n", this->reader.state.av_format_context->duration, dur, durwant, this->reader.get_time_base());
    if (durwant < dur){
        return true;
    }
    return false;
}