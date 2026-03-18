#include "timelinewalker.hpp"
static bool debug = false;

void WalkerTimeline::walk(Timeline* tl, std::list<Clip*>* out_clips){
    PROFILE_FUNCTION();
    if (debug)
        printf("walk tl\n");

    for (auto it = tl->tracks_.rbegin(); it != tl->tracks_.rend(); ++it){
        walk(std::addressof(*it), out_clips);
    }
}
void WalkerTimeline::walk(Track* track, std::list<Clip*>* out_clips){
    if (debug)
        printf("walk track. track ptr %p clips size=%zu\n", track, track->clips.size());
    if (!track){
        printf("track undefined\n");

    }
    for (auto& ptrclip : track->clips) {
        if (ptrclip == nullptr){
            printf("clip null\n");
        }
        Clip* c = ptrclip.get();
        walk(c, out_clips);
    }
}
void WalkerTimeline::walk(Clip* clip, std::list<Clip*>* out_clips){
    out_clips->push_back(clip);
}