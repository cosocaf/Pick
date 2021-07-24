/**
 * @file output_buffer.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief コンパイラがコンソールへ出力する文字列のバッファを定義する。
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_OUTPUT_BUFFER_H_
#define PICKC_OUTPUT_BUFFER_H_

#include <string>
#include <vector>
#include <memory>

namespace pickc {
  /**
   * @brief 出力するメッセージの種別の列挙。
   * 
   */
  struct OutputMessageKind {
    enum Kind : unsigned {
      /**
       * @brief 通常の出力。
       * 
       */
      Message = 1,
      /**
       * @brief 何らかのエラーではない情報を出力。
       * 
       */
      Infomation = 2,
      /**
       * @brief デバッグ用メッセージを出力。
       * 
       */
      Debug = 4,
      /**
       * @brief コンパイルを中断するほどではない警告。
       * 
       */
      Warning = 8,
      /**
       * @brief コンパイルが失敗したことを報告。
       * 
       */
      Error = 16,
      /**
       * @brief コンパイラが致命的なエラーによりクラッシュすることを報告。
       * 
       */
      Fatal = 32,
      All = (unsigned)-1,
    } _kind;
    OutputMessageKind(unsigned val);
    operator Kind();
  };
  
  /**
   * @brief OutputBufferにためるメッセージの共通クラス。
   * 継承しなくてもこのまま使えるが、場合により継承したクラスと使い分けるべき。
   */
  struct OutputMessage {
    /**
     * @brief メッセージの種別。
     * 
     */
    OutputMessageKind kind;
    /**
     * @brief メッセージの内容。
     * 
     */
    std::string message;
    /**
     * @brief OutputMessageを構築する。
     * 
     * @param kind メッセージの種類。
     * @param message メッセージ本文。
     */
    OutputMessage(OutputMessageKind kind, const std::string& message);
    /**
     * @brief デストラクタ。
     * 
     */
    virtual ~OutputMessage();
    /**
     * @brief メッセージをコンソールに出力する形式にフォーマットして出力する。
     * メッセージを特殊な形にフォーマットする場合にはこの関数をオーバーライドする。
     * 
     * @return std::string 出力するメッセージ
     */
    virtual std::string toString() const;
  };
  /**
   * @brief コンパイルエラーを報告するための汎用メッセージクラス。
   * 出力時にエラー場所周囲のコードも表示する。
   */
  struct CompileErrorMessage : public OutputMessage {
    /**
     * @brief エラーが発生したファイル名。
     * 
     */
    std::string filename;
    /**
     * @brief エラーが発生した行。
     * 
     */
    size_t line;
    /**
     * @brief エラーが発生したカラム。
     * 
     */
    size_t letter;
    /**
     * @brief エラー部分の長さ。
     * 
     */
    size_t length;
    /**
     * @brief CompileErrorMessageを構築する。
     * 
     * @param filename エラーが発生したファイル名
     * @param line エラーが発生した行
     * @param letter エラーが発生したカラム
     * @param length エラー部分の長さ
     * @param message エラーメッセージ
     */
    CompileErrorMessage(const std::string& filename, size_t line, size_t letter, size_t length, const std::string& message);
    /**
     * @brief コンソールに出力する形にフォーマットしたメッセージを返す。
     * 例えば、以下のようになる。
     * @code {.unparsed}
     * [ERROR] 変数xxxは定義されていません。
     * |1| fn main() {
     * |2|   def a = xxx;
     *               ^^^
     * |3|   return a;
     * @endcode
     * 
     * @return std::string 
     */
    virtual std::string toString() const override;
  };
  /**
   * @brief コンパイラ側で何らかの例外が発生した場合に使用する。
   * このメッセージはOutputMessageKind::Fatalであり、コンパイル作業を直ちに中断する必要がある。
   */
  struct InternalErrorMessage : public OutputMessage {
    /**
     * @brief エラーが発生したファイル名。
     * 
     */
    std::string filename;
    /**
     * @brief InternalErrorMessageを構築する。
     * 
     * @param filename エラーが発生したファイル名。ファイルに関連しない場合は空文字。
     * @param message エラーメッセージ。
     */
    InternalErrorMessage(const std::string& filename, const std::string message);
    virtual std::string toString() const override;
  };

  /**
   * @brief コンパイラがコンソールへ出力する文字列のバッファ用構造体。
   * 
   */
  class OutputBuffer {
    /**
     * @brief メッセージ一覧。
     * 
     */
    std::vector<std::unique_ptr<OutputMessage>> messages;
  public:
    /**
     * @brief メッセージを挿入する。
     * 
     * @param message 挿入するメッセージ。
     */
    void push(std::unique_ptr<OutputMessage>&& message);
    /**
     * @brief メッセージを構築し、挿入する。
     * メッセージをstd::unique_ptrで管理しているため、
     * 挿入が少々面倒であるので、
     * 簡素化するためのヘルパー関数。
     * 
     * @tparam Message 構築するメッセージ。
     * @tparam Value メッセージのコンストラクタ引数の型。
     * @param values メッセージのコンストラクタ引数。
     */
    template<typename Message, typename... Value>
    void emplace(Value&&... values) {
      push(std::make_unique<Message>(std::forward<Value>(values)...));
    }
    /**
     * @brief バッファにたまった文字列を出力する。
     * 出力した内容を削除する場合はOutputBuffer::clearを別途呼び出す。
     * 
     * @param kind 出力するバッファのビットフラグ。
     */
    void output(OutputMessageKind kind) const;
    /**
     * @brief バッファを消去する。
     * 
     */
    void clear();
    /**
     * @brief 指定のフラグのいずれかに当てはまるメッセージが存在するかを判定する。
     * 
     * @param kind ビットフラグ。デフォルトでOutputMessageKind::Error | OutputMessageKind::Fatal
     * @return true 指定のフラグのいずれかに当てはまるメッセージが存在する場合。
     * @return false 指定のフラグのいずれかに当てはまるメッセージが存在しない場合。
     */
    bool has(OutputMessageKind kind = OutputMessageKind::Error | OutputMessageKind::Fatal) const;
  };
  /**
   * @brief コンパイラがコンソールへ出力する文字列のバッファ
   * 
   */
  extern OutputBuffer outputBuffer;
}

#endif // PICKC_OUTPUT_BUFFER_H_