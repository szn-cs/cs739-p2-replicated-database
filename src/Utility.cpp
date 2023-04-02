#pragma once

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <termcolor/termcolor.hpp>

using namespace std;
using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan, termcolor::grey;
namespace fs = std::filesystem;

class Utility {
 public:
  // construct a relative path
  static std::string constructRelativePath(std::string path, std::string rootPath) {
    string relativePath;
    if (!(fs::path(path)).is_absolute())
      path = fs::canonical(path);

    relativePath = (fs::relative(path, rootPath)).generic_string();

    return relativePath;
  }

  static std::string concatenatePath(std::string base, std::string path) {
    fs::path concatenated;
    fs::path _base(base), _path(path);
    concatenated = fs::absolute(_base / _path);

    return concatenated.generic_string();
  }

  /** 
   * retrive machine's physical time using different units
   * https://stackoverflow.com/questions/21856025/getting-an-accurate-execution-time-in-c-micro-seconds
   * https://stackoverflow.com/questions/6734375/get-current-time-in-milliseconds-using-c-and-boost
  */
  static inline std::string getClockTime() {
    std::stringstream output;
    std::string time_micro;

    {
      uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now().time_since_epoch())
                        .count();
      time_micro = std::to_string(us);
    }

    {
      const auto now = std::chrono::system_clock::now();
      const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
      const auto now_us = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000000;
      output << std::put_time(std::localtime(&now_as_time_t), "%T")
             << '.' << std::setfill('0') << std::setw(3) << now_us.count() << " μ=" << time_micro << endl;
    }

    return output.str();
  }
};

#define TIME termcolor::grey << Utility::getClockTime() << termcolor::reset