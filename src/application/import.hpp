#pragma once
#include <stdio.h>
#include "mediapool.h"
#include <filesystem>
class Import{
  private:
  Mediapool* mediapool;
  public:
  Import(Mediapool* mediapool) : mediapool(mediapool){};
  void import_filepath(const char* filepath){
    if (!std::filesystem::exists(filepath)){
      throw std::runtime_error("diretório não existe");
      return;
    }
    if (!std::filesystem::is_regular_file(filepath)){
      throw std::runtime_error("não é um arquivo");
      return;
    }
    // validar filepath
    // ¿quem valida arquivo? Import ou mediapool
    MediaSource* source = mediapool->add_file(filepath);
  }
};


const std::vector<std::unique_ptr<MediaSource>>& Mediapool::get_pool(){
  return this->pool_;
}
