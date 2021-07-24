/**
 * @file output_buffer.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief output_buffer.hの実装
 * @version 0.1
 * @date 2021-07-22
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "output_buffer.h"

#include <iostream>

namespace pickc {
  OutputBuffer outputBuffer;

  OutputMessageKind::OutputMessageKind(unsigned val) : _kind(static_cast<Kind>(val)) {}
  OutputMessageKind::operator OutputMessageKind::Kind() {
    return _kind;
  }

  OutputMessage::OutputMessage(OutputMessageKind kind, const std::string& message) : kind(kind), message(message) {}
  OutputMessage::~OutputMessage() {}
  std::string OutputMessage::toString() const {
    return message;
  }

  CompileErrorMessage::CompileErrorMessage(const std::string& filename, size_t line, size_t letter, size_t length, const std::string& message):
    OutputMessage(OutputMessageKind::Error, message),
    filename(filename), line(line), letter(letter), length(length) {}
  std::string CompileErrorMessage::toString() const {
    // TODO: implement this
    return "TODO(CompileErrorMessage::toString)";
  }

  InternalErrorMessage::InternalErrorMessage(const std::string& filename, const std::string message):
    OutputMessage(OutputMessageKind::Fatal, message),
    filename(filename) {}
  std::string InternalErrorMessage::toString() const {
    // TODO: implement this
    return "TODO(InternalErrorMessage::toString)";
  }

  void OutputBuffer::push(std::unique_ptr<OutputMessage>&& message) {
    messages.emplace_back(std::move(message));
  }
  void OutputBuffer::output(OutputMessageKind kind) const {
    for(const auto& message : messages) {
      if(message->kind & kind) {
        std::cout << message->toString() << std::endl;
      }
    }
  }
  void OutputBuffer::clear() {
    messages.clear();
  }
  bool OutputBuffer::has(OutputMessageKind kind) const {
    for(const auto& message : messages) {
      if(message->kind & kind) return true;
    }
    return false;
  }
}