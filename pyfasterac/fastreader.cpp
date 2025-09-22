//
// Created by Mathieu GUIGUE on 13/09/2024.
//

#include "fastreader.h"
#include <fasterac/group.h>

#include <iostream>

std::string fast_version(){
  char info [256];
  fasterac_version_number (info);
  return {info};
}

fastreader::fastreader(const std::string& filename){
  reader = faster_file_reader_open (filename.c_str());
  if (reader == nullptr) {
    std::cout << "Error opening file " << filename << "; exiting!" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "File " << filename << " opened" << std::endl;
}

bool fastreader::get_next_event(){
  if ((p_data = faster_file_reader_next (reader)) == nullptr){
    return false;
  }
  m_data.sub_events.clear();
//  m_data.alias
  m_alias = faster_data_type_alias (p_data);
  m_data.label = faster_data_label      (p_data);
  m_data.time = faster_data_clock_ns      (p_data);
  if (m_alias == GROUP_TYPE_ALIAS) {
    // Read through events
    faster_data_p group_data;
    auto lsize = faster_data_load_size  (p_data);
    auto group_buffer = faster_data_load_p (p_data);                                  //  get group data
    auto group_reader = faster_buffer_reader_open (group_buffer, lsize);            //  open group reader
    while ((group_data = faster_buffer_reader_next (group_reader)) != nullptr) {  //  read data inside the group
      crrc4_spectro sub_event;                                               //  for that file : label1 => qdc_t_x1 ch1
      faster_data_load (group_data, &sub_event);
      sub_event_data s_event;
      s_event.label = faster_data_label (group_data);
      s_event.delta_t = sub_event.delta_t;
      s_event.q = sub_event.measure;
      m_data.sub_events.push_back(s_event);
    }
    faster_buffer_reader_close (group_reader);
    m_data.multiplicity = m_data.sub_events.size();
  }
  else if (m_alias == GROUP_COUNTER_TYPE_ALIAS) {
  }
  else {
    sub_event_data s_event;
    crrc4_spectro sub_event;                                               //  for that file : label1 => qdc_t_x1 ch1
    faster_data_load (p_data, &sub_event);
    s_event.label = faster_data_label      (p_data);
    s_event.delta_t = sub_event.delta_t;
    s_event.q = sub_event.measure;
    m_data.sub_events.push_back(s_event);
    m_data.multiplicity = 1;
  }
  return true;
}

bool fastreader::is_group() const {
  if (m_alias == GROUP_TYPE_ALIAS){
    return true;
  }
  return false;
}

//event_data fastreader::get_event() {
//  event_data data;
//
//  data.time = faster_data_clock_ns   (p_data);
//  data.label = m_label;
//
//  auto lsize = faster_data_load_size  (p_data);
//  if ( is_group()){
//    auto group_buffer = faster_data_load_p (p_data);
//    auto group_reader = faster_buffer_reader_open (group_buffer, lsize);
//    faster_data_p group_data;
////    while ((group_data = faster_buffer_reader_next (group_reader)) != NULL){
////      crrc4_spectro qdc;
////      faster_data_load (group_data, &qdc);
////
////    }
//  }
//  else {
//    data.multiplicity = 1;
//  }
//  return data;
//}



fastreader::~fastreader() {
  close_file();
}

void fastreader::close_file() {
  faster_file_reader_close (reader);
}
