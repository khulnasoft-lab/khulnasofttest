// Copyright 2007, Khulnasoft Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Khulnasoft Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Khulnasoft Mock - a framework for writing C++ mock classes.
//
// This file defines some utilities useful for implementing Khulnasoft
// Mock.  They are subject to change without notice, so please DO NOT
// USE THEM IN USER CODE.

#include "kmock/internal/kmock-internal-utils.h"

#include <ctype.h>

#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>  // NOLINT
#include <string>
#include <vector>

#include "kmock/kmock.h"
#include "kmock/internal/kmock-port.h"
#include "ktest/ktest.h"

namespace testing {
namespace internal {

// Joins a vector of strings as if they are fields of a tuple; returns
// the joined string.
KTEST_API_ std::string JoinAsKeyValueTuple(
    const std::vector<const char*>& names, const Strings& values) {
  KTEST_CHECK_(names.size() == values.size());
  if (values.empty()) {
    return "";
  }
  const auto build_one = [&](const size_t i) {
    return std::string(names[i]) + ": " + values[i];
  };
  std::string result = "(" + build_one(0);
  for (size_t i = 1; i < values.size(); i++) {
    result += ", ";
    result += build_one(i);
  }
  result += ")";
  return result;
}

// Converts an identifier name to a space-separated list of lower-case
// words.  Each maximum substring of the form [A-Za-z][a-z]*|\d+ is
// treated as one word.  For example, both "FooBar123" and
// "foo_bar_123" are converted to "foo bar 123".
KTEST_API_ std::string ConvertIdentifierNameToWords(const char* id_name) {
  std::string result;
  char prev_char = '\0';
  for (const char* p = id_name; *p != '\0'; prev_char = *(p++)) {
    // We don't care about the current locale as the input is
    // guaranteed to be a valid C++ identifier name.
    const bool starts_new_word = IsUpper(*p) ||
                                 (!IsAlpha(prev_char) && IsLower(*p)) ||
                                 (!IsDigit(prev_char) && IsDigit(*p));

    if (IsAlNum(*p)) {
      if (starts_new_word && !result.empty()) result += ' ';
      result += ToLower(*p);
    }
  }
  return result;
}

// This class reports Khulnasoft Mock failures as Khulnasoft Test failures.  A
// user can define another class in a similar fashion if they intend to
// use Khulnasoft Mock with a testing framework other than Khulnasoft Test.
class KhulnasoftTestFailureReporter : public FailureReporterInterface {
 public:
  void ReportFailure(FailureType type, const char* file, int line,
                     const std::string& message) override {
    AssertHelper(type == kFatal ? TestPartResult::kFatalFailure
                                : TestPartResult::kNonFatalFailure,
                 file, line, message.c_str()) = Message();
    if (type == kFatal) {
      posix::Abort();
    }
  }
};

// Returns the global failure reporter.  Will create a
// KhulnasoftTestFailureReporter and return it the first time called.
KTEST_API_ FailureReporterInterface* GetFailureReporter() {
  // Points to the global failure reporter used by Khulnasoft Mock.  gcc
  // guarantees that the following use of failure_reporter is
  // thread-safe.  We may need to add additional synchronization to
  // protect failure_reporter if we port Khulnasoft Mock to other
  // compilers.
  static FailureReporterInterface* const failure_reporter =
      new KhulnasoftTestFailureReporter();
  return failure_reporter;
}

// Protects global resources (stdout in particular) used by Log().
static KTEST_DEFINE_STATIC_MUTEX_(g_log_mutex);

// Returns true if and only if a log with the given severity is visible
// according to the --kmock_verbose flag.
KTEST_API_ bool LogIsVisible(LogSeverity severity) {
  if (KMOCK_FLAG_GET(verbose) == kInfoVerbosity) {
    // Always show the log if --kmock_verbose=info.
    return true;
  } else if (KMOCK_FLAG_GET(verbose) == kErrorVerbosity) {
    // Always hide it if --kmock_verbose=error.
    return false;
  } else {
    // If --kmock_verbose is neither "info" nor "error", we treat it
    // as "warning" (its default value).
    return severity == kWarning;
  }
}

// Prints the given message to stdout if and only if 'severity' >= the level
// specified by the --kmock_verbose flag.  If stack_frames_to_skip >=
// 0, also prints the stack trace excluding the top
// stack_frames_to_skip frames.  In opt mode, any positive
// stack_frames_to_skip is treated as 0, since we don't know which
// function calls will be inlined by the compiler and need to be
// conservative.
KTEST_API_ void Log(LogSeverity severity, const std::string& message,
                    int stack_frames_to_skip) {
  if (!LogIsVisible(severity)) return;

  // Ensures that logs from different threads don't interleave.
  MutexLock l(&g_log_mutex);

  if (severity == kWarning) {
    // Prints a KMOCK WARNING marker to make the warnings easily searchable.
    std::cout << "\nKMOCK WARNING:";
  }
  // Pre-pends a new-line to message if it doesn't start with one.
  if (message.empty() || message[0] != '\n') {
    std::cout << "\n";
  }
  std::cout << message;
  if (stack_frames_to_skip >= 0) {
#ifdef NDEBUG
    // In opt mode, we have to be conservative and skip no stack frame.
    const int actual_to_skip = 0;
#else
    // In dbg mode, we can do what the caller tell us to do (plus one
    // for skipping this function's stack frame).
    const int actual_to_skip = stack_frames_to_skip + 1;
#endif  // NDEBUG

    // Appends a new-line to message if it doesn't end with one.
    if (!message.empty() && *message.rbegin() != '\n') {
      std::cout << "\n";
    }
    std::cout << "Stack trace:\n"
              << ::testing::internal::GetCurrentOsStackTraceExceptTop(
                     actual_to_skip);
  }
  std::cout << ::std::flush;
}

KTEST_API_ WithoutMatchers GetWithoutMatchers() { return WithoutMatchers(); }

KTEST_API_ void IllegalDoDefault(const char* file, int line) {
  internal::Assert(
      false, file, line,
      "You are using DoDefault() inside a composite action like "
      "DoAll() or WithArgs().  This is not supported for technical "
      "reasons.  Please instead spell out the default action, or "
      "assign the default action to an Action variable and use "
      "the variable in various places.");
}

constexpr char UndoWebSafeEncoding(char c) {
  return c == '-' ? '+' : c == '_' ? '/' : c;
}

constexpr char UnBase64Impl(char c, const char* const base64, char carry) {
  return *base64 == 0 ? static_cast<char>(65)
         : *base64 == c
             ? carry
             : UnBase64Impl(c, base64 + 1, static_cast<char>(carry + 1));
}

template <size_t... I>
constexpr std::array<char, 256> UnBase64Impl(IndexSequence<I...>,
                                             const char* const base64) {
  return {
      {UnBase64Impl(UndoWebSafeEncoding(static_cast<char>(I)), base64, 0)...}};
}

constexpr std::array<char, 256> UnBase64(const char* const base64) {
  return UnBase64Impl(MakeIndexSequence<256>{}, base64);
}

static constexpr char kBase64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static constexpr std::array<char, 256> kUnBase64 = UnBase64(kBase64);

bool Base64Unescape(const std::string& encoded, std::string* decoded) {
  decoded->clear();
  size_t encoded_len = encoded.size();
  decoded->reserve(3 * (encoded_len / 4) + (encoded_len % 4));
  int bit_pos = 0;
  char dst = 0;
  for (int src : encoded) {
    if (std::isspace(src) || src == '=') {
      continue;
    }
    char src_bin = kUnBase64[static_cast<size_t>(src)];
    if (src_bin >= 64) {
      decoded->clear();
      return false;
    }
    if (bit_pos == 0) {
      dst |= static_cast<char>(src_bin << 2);
      bit_pos = 6;
    } else {
      dst |= static_cast<char>(src_bin >> (bit_pos - 2));
      decoded->push_back(dst);
      dst = static_cast<char>(src_bin << (10 - bit_pos));
      bit_pos = (bit_pos + 6) % 8;
    }
  }
  return true;
}

}  // namespace internal
}  // namespace testing
