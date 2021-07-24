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
    OutputMessageKind kind;
    std::string message;
    OutputMessage(OutputMessageKind kind, const std::string& message);
    virtual ~OutputMessage();
    virtual std::string toString() const;
  };
  /**
   * @brief コンパイルエラーを報告するための汎用メッセージクラス。
   * 出力時にエラー場所周囲のコードも表示する。
   */
  struct CompileErrorMessage : public OutputMessage {
    std::string filename;
    size_t line;
    size_t letter;
    size_t length;
    CompileErrorMessage(const std::string& filename, size_t line, size_t letter, size_t length, const std::string& message);
    virtual std::string toString() const override;
  };
  /**
   * @brief コンパイラ側で何らかの例外が発生した場合に使用する。
   * このメッセージはOutputMessageKind::Fatalであり、コンパイル作業を直ちに中断する必要がある。
   */
  struct InternalErrorMessage : public OutputMessage {
    std::string filename;
    InternalErrorMessage(const std::string& filename, const std::string message);
    virtual std::string toString() const override;
  };

  /**
   * @brief コンパイラがコンソールへ出力する文字列のバッファ用構造体。
   * 
   */
  class OutputBuffer {
    std::vector<std::unique_ptr<OutputMessage>> messages;
  public:
    void push(std::unique_ptr<OutputMessage>&& message);
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