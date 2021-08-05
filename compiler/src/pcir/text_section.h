/**
 * @file text_section.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_TEXT_SECTION_H_
#define PICKC_PCIR_TEXT_SECTION_H_

#include <string>
#include <set>
#include <memory>

#include "utils/binary_vec.h"

namespace pickc::pcir {
  class _TextSection;
  using TextSection = std::shared_ptr<_TextSection>;
  using WeakTextSection = std::weak_ptr<_TextSection>;

  class Text {
    std::string value;
    WeakTextSection section;
    friend bool operator<(const Text&, const Text&);
  public:
    Text(const std::string& value, const WeakTextSection& section);
    std::string getValue() const;
    uint32_t getIndex() const;
  };
  bool operator<(const Text& a, const Text& b);
  
  class _TextSection : public std::enable_shared_from_this<_TextSection> {
    std::set<std::string> texts;
  private:
    _TextSection() = default;
  public:
    static TextSection create();
  public:
    Text createText(const std::string& value);
    uint32_t getIndex(const Text& text) const;
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_TEXT_SECTION_H_