/**
 * @file text_section.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief text_section.hの実装
 * @version 0.1
 * @date 2021-07-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "text_section.h"

#include <cassert>

namespace pickc::pcir {
  Text::Text(const std::string& value, const WeakTextSection& section) : value(value), section(section) {}
  std::string Text::getValue() const {
    return value;
  }
  uint32_t Text::getIndex() const {
    return section.lock()->getIndex(*this);
  }
  bool operator<(const Text& a, const Text& b) {
    assert(a.section.lock() == b.section.lock());
    return a.value < b.value;
  }

  TextSection _TextSection::create() {
    return TextSection(new _TextSection());
  }
  Text _TextSection::createText(const std::string& value) {
    texts.insert(value);
    return Text(value, weak_from_this());
  }
  uint32_t _TextSection::getIndex(const Text& text) const {
    return static_cast<uint32_t>(std::distance(
      texts.begin(),
      texts.find(text.getValue())
    ));
  }
  BinaryVec _TextSection::toBinaryVec() const {
    BinaryVec bin;
    bin << static_cast<uint32_t>(0);
    bin << static_cast<uint32_t>(texts.size());
    for(const auto& text : texts) {
      bin << static_cast<uint32_t>(text.size());
      bin << text;
    }
    write(bin, 0, static_cast<uint32_t>(bin.size()));
    return bin;
  }
}