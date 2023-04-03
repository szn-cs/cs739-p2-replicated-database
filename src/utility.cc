#include "./include.h"

// construct a relative path
std::string utility::constructRelativePath(std::string path, std::string rootPath) {
  string relativePath;
  if (!(std::filesystem::path(path)).is_absolute())
    path = std::filesystem::canonical(path);

  relativePath = (std::filesystem::relative(path, rootPath)).generic_string();

  return relativePath;
}

std::string utility::concatenatePath(std::string base, std::string path) {
  std::filesystem::path concatenated;
  std::filesystem::path _base(base), _path(path);
  concatenated = std::filesystem::absolute(_base / _path);

  return concatenated.generic_string();
}

/** 
 * retrive machine's physical time using different units
 * https://stackoverflow.com/questions/21856025/getting-an-accurate-execution-time-in-c-micro-seconds
 * https://stackoverflow.com/questions/6734375/get-current-time-in-milliseconds-using-c-and-boost
*/
std::string utility::getClockTime() {
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

namespace parse {

}  // namespace parse
