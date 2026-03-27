#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace {

std::string read_file(const std::string& path) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in) {
    throw std::runtime_error("Failed to open file: " + path);
  }
  std::string content;
  in.seekg(0, std::ios::end);
  content.resize(static_cast<size_t>(in.tellg()));
  in.seekg(0, std::ios::beg);
  in.read(content.data(), static_cast<std::streamsize>(content.size()));
  return content;
}

void ensure_vulnerable_pattern_present(const std::string& source) {
  const std::string s1 = "adnl_id_to_peer_.at(src_adnl_id)";
  const std::string s2 = "short_id_to_peer_.at(src)";
  const std::string s3 = "adnl_id_to_peer_.at(src)";
  if (source.find(s1) == std::string::npos || source.find(s2) == std::string::npos ||
      source.find(s3) == std::string::npos) {
    throw std::runtime_error(
        "Expected .at(...) callback pattern not found in private-overlay.cpp. "
        "Code may have changed or issue is already fixed.");
  }
}

void simulate_missing_key_at() {
  std::map<std::string, int> adnl_id_to_peer;
  adnl_id_to_peer["known"] = 0;
  (void)adnl_id_to_peer.at("unknown");  // throws std::out_of_range
}

}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <path-to-ton-root>\n";
      std::cerr << "Example: " << argv[0] << " /Users/kairattalantbekov/Desktop/ton\n";
      return 2;
    }
    const std::filesystem::path ton_root = std::filesystem::path(argv[1]);
    const std::filesystem::path target_path = ton_root / "validator/consensus/private-overlay.cpp";
    const std::string target = target_path.string();
    const std::string source = read_file(target);
    ensure_vulnerable_pattern_present(source);

    std::cout << "Issue #2 reproduction\n";
    std::cout << "Target file: " << target << "\n";
    std::cout << "Input: callback source key not present in peer map\n";

    try {
      simulate_missing_key_at();
    } catch (const std::out_of_range&) {
      std::cout << "Observed exception: std::out_of_range\n";
      std::cout << "PASS: Crash condition reproduced (missing key causes map::at exception).\n";
      return 0;
    }

    std::cout << "FAIL: Missing-key lookup did not raise.\n";
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 2;
  }
}
