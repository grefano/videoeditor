#pragma once
#include "imgui.h"
#include "timeline.hpp"
#include <functional>
#include <cstring>
#include "log.hpp"
static bool debug = false;
struct TimelineUI{
    Timeline* tl;
    ImVec2 view_time_window;
    ImVec2 view_track_window;
    float px_per_sec;
    float px_per_track;
    
    ImVec2 size;

    Clip* selected_clip = nullptr;
    size_t selected_clip_hover_track = -1;
    float selected_clip_hover_rel_t0 = 0;
    float selected_clip_hover_t0 = 0;
    
    float get_x(float time){
        return (time - view_time_window.x) * px_per_sec;
    }  
    float get_t(float x){
        // (time - view_time_window.x) * px_per_sec = x;
        // time * pxsec - view.x*px_per_sec = x
        // time = (x + view.x*px_per_sec ) / px_sec
        // time = (x/ px_sec + view.x ) 
        return (x / px_per_sec + view_time_window.x);
    }
    struct ClipState{
        Clip* clip;
        ImVec2 pos;
        ImVec2 pos2;
         


    };

    std::list<ClipState> clips; // !!! preciso modificar domain clip a partir da interação com usuario. usuario muda uiclip -> 

    TimelineUI(Timeline* tl) : tl(tl){
        this->view_time_window = ImVec2(0, 100);
        this->view_track_window = ImVec2(0, 2);
        this->px_per_sec = 20;

       
    }


    ImVec2 get_track_pos(int id){
        return ImVec2(0, id*px_per_track);
    }
    ImVec2 get_track_size(int id){
        return ImVec2(this->size.x, (id+1)*px_per_track);
    }
    ImVec2 get_clip_pos(Clip* clip, Track* track, Timeline* tl){
        return ImVec2( get_x(clip->tl_time0), get_track_pos(track->id).y);
    }
    ImVec2 get_clip_size(Clip* clip, Track* track, Timeline* tl){
        return ImVec2( (clip->tl_time1 - clip->tl_time0) * px_per_sec, get_track_size(track->id).y);
    }

    void set_size(float w, float h){
        this->size = ImVec2(w, h);
        this->px_per_sec = w/(view_time_window.y-view_time_window.x);
        this->px_per_track = h / (view_track_window.y+1 - view_track_window.x);
    }

    //interaction
    bool isHoveringClipSide(ImVec2 pos, float y2, ImVec2 cursorpos){

        if (cursorpos.x > pos.x-10 && cursorpos.x < pos.x+10 && cursorpos.y > pos.y+3 && cursorpos.y < y2-3){
            return true;
        }
        return false;
    }

    void drawClipSide(ImVec2 pos, ImVec2 pos2, ImVec2 cursorpos, ImDrawList* drawlist){
        auto col = IM_COL32(255, 0, 0, 255);
        if (isHoveringClipSide(pos, pos2.y, cursorpos)){
            drawlist->AddRectFilled({pos.x-2, pos.y+2}, {pos.x+2, pos2.y-2}, col);
        }
        // if (isHoveringClipSide({pos2.x, pos.y}, pos2.y, cursorpos)){
        //     drawlist->AddRectFilled({pos.x-2, pos.y+2}, {pos2.x+2, pos2.y-2}, col);
        // }

    }


    struct Application{
        enum DRAG_CLIP{ NONE, LEFT, RIGHT, MIDDLE };
        DRAG_CLIP drag_clip_mode = NONE;
        virtual void soltar_source(Timeline* tl, size_t track, MediaSource* source, ImVec2 t){
            // MediaSource* file1 = (*mediapool).add_file(filepath);
            VideoClip* masterclip;
            try {
                masterclip = new VideoClip(source);
            } catch (const std::exception& e){
                printf("video inválido\n");
                return;
            }

            Clip* clip = tl->add_clip(track, t.x, t.y);
            clip->masterclip = masterclip;
        }
        void drag(Clip* clip, float disp){
            if (drag_clip_mode == NONE) { return; }
            if (drag_clip_mode == LEFT){
                clip->tl_time0 += disp;

            } else if (drag_clip_mode == RIGHT){
                clip->tl_time1 += disp;

            } else if (drag_clip_mode == MIDDLE){
                clip->tl_time0 += disp;
                clip->tl_time1 += disp;
            }
        }
        
    };
    Application app;
    void draw(){
        log("draw timeline\n");
        ImGui::Begin("tl");                          
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        ImVec2 screenpos = ImGui::GetCursorScreenPos();
        ImVec2 cursorpos = ImGui::GetMousePos();
        this->set_size(ImGui::GetWindowWidth(), 300);
        drawlist->AddRectFilled(screenpos, ImVec2(screenpos.x+this->size.x, screenpos.y+this->size.y), IM_COL32(40, 40, 40, 255));
        int i = 0;
        for(auto& track : (*tl).tracks_){
            log("track id %d", track.id);
            ImVec2 track_pos = this->get_track_pos(track.id);
            ImVec2 track_size = this->get_track_size(track.id);
            ImVec2 pos =ImVec2(screenpos.x+track_pos.x, screenpos.y+track_pos.y);
            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton(("soltar " + std::to_string(i)).c_str(), track_size, 0);
            i++;
            drawlist->AddRectFilled(pos,
                ImVec2(pos.x+track_size.x, pos.y+track_size.y), IM_COL32(80, 80, 100, 255));
            
            if (ImGui::BeginDragDropTarget()){
                log("dragged mediasource");
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MEDIASOURCE")){
                    MediaSource* source = static_cast<MediaSource*>(payload->Data);
                    log("dragged mediasource %s\n", source->filepath);
                    
                    float rel_x = cursorpos.x - pos.x;

                    ImVec2 t = {get_t(rel_x), 0};
                    t.y = t.x + 10;
                    app.soltar_source(tl, track.id, source, t);
                }
                ImGui::EndDragDropTarget();
            }
            for(auto& clip : track.clips){
                Clip* ptr_clip = clip.get();
                log("track pos %f", track_pos.y);
                log("clip t0 %f t1 %f\n", (*clip).tl_time0, (*clip).tl_time1);
                ImVec2 pos = this->get_clip_pos(ptr_clip, &track, tl);
                ImVec2 size = this->get_clip_size(ptr_clip, &track, tl);
                ImVec2 realpos = ImVec2(screenpos.x+pos.x, screenpos.y+pos.y);
                ImVec2 realpos2 = ImVec2(realpos.x+size.x,realpos.y+size.y);
                auto col = IM_COL32(0, 0, 255, 255);
                if (app.drag_clip_mode == app.NONE){
                    if(ImGui::IsMouseHoveringRect(realpos, realpos2)){
                        col = IM_COL32(100,0,255,255);
                        int growstroke = 5;
                        realpos.x -= growstroke;
                        realpos.y -= growstroke;
                        realpos2.x += growstroke;
                        realpos2.y += growstroke;
                        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                            selected_clip = ptr_clip;
                            selected_clip_hover_rel_t0 = selected_clip->tl_time0;
                        }
                    }
                    
                    if (selected_clip == ptr_clip){
                        col = IM_COL32(0,100,100,255);
                    }

                }
                drawlist->AddRectFilled(realpos, realpos2, col);
                drawClipSide(realpos, realpos2, cursorpos, drawlist);
                drawClipSide({realpos2.x, realpos.y}, realpos2, cursorpos, drawlist);
                
                ImGui::PushID(ptr_clip);
                ImGui::SetCursorScreenPos(realpos);
                ImGui::InvisibleButton("##clip", size);
                ImGui::PopID();



                if (selected_clip == ptr_clip){
                    float disp = ImGui::GetIO().MouseDelta.x / px_per_sec;
                    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
                        if (app.drag_clip_mode == app.NONE){
                            
                            if (isHoveringClipSide(realpos, realpos2.y, cursorpos)){
                                app.drag_clip_mode = app.LEFT;
                            } else if (isHoveringClipSide({realpos2.x, realpos.y}, realpos2.y,cursorpos)){
                                app.drag_clip_mode = app.RIGHT;
                            } else {
                                app.drag_clip_mode = app.MIDDLE;
                            }
                        }
                    } else {
                        app.drag_clip_mode = app.NONE;
                    }
                    app.drag(ptr_clip, disp);
                }
                // drawClipSide({realpos2.x, realpos.y}, realpos2, cursorpos, drawlist);
                
            }
        }
        float headx = screenpos.x + get_x(tl->playhead_time);
        drawlist->AddRectFilled(ImVec2(headx, screenpos.y), ImVec2(headx+5, screenpos.y+this->size.y), IM_COL32(0,255,0,255));
        ImGui::End();
        log("end draw timeline\n");
    }

};


struct PreviewUI{
    const char* name;
    PreviewUI(const char* name = "preview"){
        this->name = name; 
    }
    void draw(Timeline* tl, GLuint tex, ImVec2 dim){
        log("draw preview %d\n", tex);
        assert(tex != 0);
        static double lasttime = 0;
        double now = glfwGetTime();
        double dt = now - lasttime;        
        lasttime = now;
        ImGui::Begin(this->name);                          
        auto drawlist = ImGui::GetWindowDrawList();
        ImVec2 screenpos = ImGui::GetCursorScreenPos();
        ImVec2 cursorpos = ImGui::GetMousePos();
        ImGui::BeginDisabled();
        float _dtfloat = dt;
        ImGui::SliderFloat("dt", &_dtfloat, 0, 1);
        ImGui::EndDisabled();
        ImGui::SliderFloat("playhead", &(*tl).playhead_time, 0, 100);
        ImGui::Image(tex, dim);

        ImGui::End();

        log("end draw preview\n");

    }
};


struct ImportUI{
    char filepath[30] = "";
    struct Application{
        std::function<void(char*)> import;

    };
    Application app;
    void draw(){

        ImGui::Begin("import");                          
        ImGui::InputText("filepath", filepath, 30, ImGuiInputTextFlags_CharsNoBlank);
        if (ImGui::Button("import file", {50, 50})){
            assert(app.import);
            try{
                app.import(filepath);
            } catch (const std::exception& e){
                std::cout << e.what() << "\0";
            }
        }
        ImGui::End();
    }
};

struct MediapoolUI{
    void draw(Mediapool* pool){
        assert(pool != nullptr);
        int source_size = 200;
        int source_sep = 20;
        
        ImGui::Begin("mediapool", NULL, NULL);  
        ImVec2 screenpos = ImGui::GetCursorScreenPos();
        ImVec2 regionavail = ImGui::GetContentRegionAvail();
        int qtd_sources =  (regionavail.x-source_sep) / (source_sep+source_size);
        if (qtd_sources < 1) qtd_sources = 1;

        auto drawlist = ImGui::GetWindowDrawList();

        int i = 0;
        for(auto& source : pool->get_pool()){
            const char* path = source.get()->filepath;
            log("path %p %p\n", path, path + std::strlen(path));
            
            ImVec2 ipos = {i % qtd_sources, (int)(i / qtd_sources)};
            ImVec2 pos = {screenpos.x + ipos.x * (source_sep + source_size), screenpos.y + ipos.y * (source_sep + source_size)};
            ImVec2 pos2 = {pos.x + source_size, pos.y + source_size};
            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton(("drag" + std::to_string(i)).c_str(), {source_size, source_size});
            drawlist->AddRectFilled(pos, pos2, IM_COL32(200, 230, 220, 255));
            if (ImGui::BeginDragDropSource()){
                ImGui::SetDragDropPayload("MEDIASOURCE", source.get(), sizeof(MediaSource), ImGuiCond_Once);
                ImGui::Text("teste");
                ImGui::BeginTooltip();
                ImGui::Text("dragging %d", i);
                ImGui::EndTooltip();
                ImGui::EndDragDropSource();
            }
            ImVec2 textsize = ImGui::CalcTextSize(path);
            // float textident = (regionavail.x + textsize.x) / 2;
            ImVec2 ident = {-(textsize.x)/2, -(textsize.y)/2};
            drawlist->AddText({(pos.x + pos2.x) / 2 + ident.x, (pos.y + pos2.y)/2 + ident.y} , IM_COL32(0,0,0,255), path);
            i++;
        }
        
        ImGui::End();
    }
};
/* import domain */
