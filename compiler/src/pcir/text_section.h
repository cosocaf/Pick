/**
 * @file text_section.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief テキストセクション
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
  /**
   * @brief _TextSectionのshared_ptrラッパー。
   * 
   */
  using TextSection = std::shared_ptr<_TextSection>;
  /**
   * @brief _TextSectionのweak_ptrラッパー。
   * 
   */
  using WeakTextSection = std::weak_ptr<_TextSection>;

  /**
   * @brief テキスト。
   * 
   * @note ほかのセクションやセクション要素と違い、
   * このクラスはshared_ptrやweak_ptrでラップせずに実装している。
   * ほかのクラスと違い、このクラスはイミュータブルに実装しているため、
   * コピーが発生しても何ら問題ないからである。
   * もし統一性が欲しかったり、コピーが発生して問題があるようならば、
   * ほかのクラスと同じような実装に変更する。
   */
  class Text {
    std::string value;
    WeakTextSection section;
    friend bool operator<(const Text&, const Text&);
  public:
    /**
     * @brief Textを構築する。
     * 
     * @param value 文字列
     * @param section セクション
     */
    Text(const std::string& value, const WeakTextSection& section);
    /**
     * @brief このテキストの文字列を取得する。
     * 
     * @return std::string 文字列
     */
    std::string getValue() const;
    /**
     * @brief このテキストのインデックスを取得する。
     * 
     * @return uint32_t インデックス
     */
    uint32_t getIndex() const;
  };
  /**
   * @brief Text同士の大小比較をする。
   * 実装ではText::valueの大小比較をする。
   * 
   * @param a 左オペランド
   * @param b 右オペランド
   * @return true a.value < b.value == true
   * @return false a.value < b.value == false
   */
  bool operator<(const Text& a, const Text& b);
  
  /**
   * @brief テキストセクション。
   * 基本的にこのクラスを直接扱わず、TextSectionやWeakTextSectionを使用する。
   */
  class _TextSection : public std::enable_shared_from_this<_TextSection> {
    std::set<std::string> texts;
  private:
    _TextSection() = default;
  public:
    /**
     * @brief TextSectionを作成する。
     * 
     * @return TextSection 作成したTextSection
     */
    static TextSection create();
  public:
    /**
     * @brief Textを作成する。
     * 
     * @param value 作成するテキストの内容
     * @return Text 作成したテキスト
     */
    Text createText(const std::string& value);
    /**
     * @brief 指定のテキストのインデックスを取得する。
     * 
     * @param text インデックスを取得するテキスト
     * @return uint32_t テキストのインデックス
     */
    uint32_t getIndex(const Text& text) const;
    /**
     * @brief このセクションのPCIRバイトコード表現を出力する。
     * 
     * @return BinaryVec 出力結果
     */
    BinaryVec toBinaryVec() const;
  };
}

#endif // PICKC_PCIR_TEXT_SECTION_H_