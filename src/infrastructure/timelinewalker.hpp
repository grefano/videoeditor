#pragma once
#include "timeline.hpp"
#include "profile.hpp"

struct Timeline;

struct WalkerTimeline{
    static void walk(Timeline* tl, std::list<Clip*>* out_clips);
    static void walk(Track* track, std::list<Clip*>* out_clips);
    static void walk(Clip* clip, std::list<Clip*>* out_clips);
};