#include <fstream>
#include <filesystem>
#include <iostream>
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

bool has_misbehavior_publish_near(const std::string& source, size_t pos, size_t window) {
  if (pos == std::string::npos) {
    return false;
  }
  const size_t start = (pos > window) ? (pos - window) : 0;
  const size_t end = std::min(source.size(), pos + window);
  const std::string area = source.substr(start, end - start);
  return area.find("publish<MisbehaviorReport>") != std::string::npos;
}

void verify_reporting_gap(const std::string& source) {
  const std::string marker_parent = "FIXME: report misbehavior";
  const std::string marker_conflict = "FIXME: Report misbehavior";
  const std::string marker_reject = "FIXME: Report misbehavior";

  const size_t p_parent = source.find(marker_parent);
  if (p_parent == std::string::npos) {
    throw std::runtime_error("Expected marker for invalid parent relation not found.");
  }
  if (has_misbehavior_publish_near(source, p_parent, 250)) {
    throw std::runtime_error("Unexpected MisbehaviorReport publish near invalid parent FIXME marker.");
  }

  // Find second occurrence for pending candidate conflict.
  const size_t p_conflict = source.find(marker_conflict, p_parent + 1);
  if (p_conflict == std::string::npos) {
    throw std::runtime_error("Expected marker for conflicting pending candidate not found.");
  }
  if (has_misbehavior_publish_near(source, p_conflict, 250)) {
    throw std::runtime_error("Unexpected MisbehaviorReport publish near conflict FIXME marker.");
  }

  // Find third occurrence for validation-rejected candidate path.
  const size_t p_reject = source.find(marker_reject, p_conflict + 1);
  if (p_reject == std::string::npos) {
    throw std::runtime_error("Expected marker for validation-rejected candidate not found.");
  }
  if (has_misbehavior_publish_near(source, p_reject, 300)) {
    throw std::runtime_error("Unexpected MisbehaviorReport publish near reject FIXME marker.");
  }
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
    const std::filesystem::path target_path = ton_root / "validator/consensus/simplex/consensus.cpp";
    const std::string target = target_path.string();
    const std::string source = read_file(target);

    verify_reporting_gap(source);

    std::cout << "Issue #3 reproduction\n";
    std::cout << "Target file: " << target << "\n";
    std::cout << "Input: malformed/conflicting/rejected candidate paths in simplex consensus\n";
    std::cout << "Observed behavior: code contains FIXME report markers but no nearby MisbehaviorReport publish\n";
    std::cout << "PASS: Misbehavior reporting gap reproduced.\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 2;
  }
}
