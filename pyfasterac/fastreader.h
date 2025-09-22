//
// Created by Mathieu GUIGUE on 13/09/2024.
//

#ifndef FASTREADER_PYFASTERAC_H
#define FASTREADER_PYFASTERAC_H

#include <string>
#include <vector>
#include "fasterac/fasterac.h"
#include "fasterac/adc.h"

struct sub_event_data{
  unsigned int delta_t;
  unsigned int label;
  unsigned int q;
};

struct event_data{
  // main informations
  unsigned int time;
  unsigned int label;
  unsigned int multiplicity;
  std::vector<sub_event_data> sub_events;
};

std::string fast_version();

class fastreader {
public:
  fastreader(const std::string& filename);

  bool get_next_event();
  bool is_group() const;

  event_data get_event(){return m_data;};

  ~fastreader();
private:

  void close_file();

  faster_file_reader_p   reader{};
  faster_data_p          p_data{};
  event_data             m_data;

//   main informations
  unsigned char m_alias{};
};


#endif //FASTREADER_PYFASTERAC_H

