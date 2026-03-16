#pragma once
#include <stdio.h>
#include "mediapool.h"


class Import{
  private:
  Mediapool* mediapool;
  public:
  Import(Mediapool* mediapool) : mediapool(mediapool){};
  void import_filepath(const char* filepath){
    // validar filepath
    // ¿quem valida arquivo? Import ou mediapool
    
    MediaSource* source = mediapool->add_file(filepath);
    printf("filepath %s\n", filepath);
  }
};


const std::vector<std::unique_ptr<MediaSource>>& Mediapool::get_pool(){
  return this->pool_;
}
