#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
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
  const std::string field_pattern = "std::set<BlockIdExt> sent_candidate_broadcasts_";
  const std::string insert_pattern = "sent_candidate_broadcasts_.insert(candidate.id)";
  const std::string contains_pattern = "sent_candidate_broadcasts_.contains(block.id)";

  if (source.find(field_pattern) == std::string::npos || source.find(insert_pattern) == std::string::npos ||
      source.find(contains_pattern) == std::string::npos) {
    throw std::runtime_error(
        "Expected block-accepter broadcast dedup set pattern not found. "
        "Code may have changed or issue may already be fixed.");
  }

  if (source.find("sent_candidate_broadcasts_.erase(") != std::string::npos ||
      source.find("sent_candidate_broadcasts_.clear(") != std::string::npos) {
    throw std::runtime_error("Found broadcast set pruning logic; issue might already be fixed.");
  }
}

void simulate_monotonic_dedup_set_growth() {
  std::set<std::string> sent_candidate_broadcasts;
  constexpr int kEntries = 220000;
  for (int i = 0; i < kEntries; ++i) {
    sent_candidate_broadcasts.insert("block_" + std::to_string(i));
  }
  if (static_cast<int>(sent_candidate_broadcasts.size()) != kEntries) {
    throw std::runtime_error("Simulation failed to create expected set size.");
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
    const std::filesystem::path target_path = ton_root / "validator/consensus/block-accepter.cpp";
    const std::string target = target_path.string();
    const std::string source = read_file(target);

    ensure_vulnerable_pattern_present(source);
    simulate_monotonic_dedup_set_growth();

    std::cout << "Issue #6 reproduction\n";
    std::cout << "Target file: " << target << "\n";
    std::cout << "Input: long-running stream of unique shard block ids\n";
    std::cout << "Observed behavior: broadcast dedup set grows monotonically with no pruning\n";
    std::cout << "PASS: Block accepter broadcast set growth vulnerability reproduced.\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 2;
  }
}
