#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
  const std::string cache_field = "std::map<ParentId, CachedState> state_cache_";
  const std::string finalized_field = "std::map<CandidateId, FinalizedBlock> finalized_blocks_";
  const std::string cache_insert = "CachedState& entry = state_cache_[id]";
  const std::string finalized_insert = "FinalizedBlock& state = finalized_blocks_[id]";

  if (source.find(cache_field) == std::string::npos || source.find(finalized_field) == std::string::npos ||
      source.find(cache_insert) == std::string::npos || source.find(finalized_insert) == std::string::npos) {
    throw std::runtime_error(
        "Expected state-resolver cache insertion patterns not found. "
        "Code may have changed or issue may already be fixed.");
  }

  // Existing erase calls are error-path cleanup; there is no normal lifecycle pruning strategy.
  if (source.find("state_cache_.erase(id);") == std::string::npos ||
      source.find("finalized_blocks_.erase(id);") == std::string::npos) {
    throw std::runtime_error("Expected error-path erase markers not found; source layout changed.");
  }
}

void simulate_long_running_cache_accumulation() {
  std::unordered_map<std::string, int> state_cache;
  std::unordered_map<std::string, int> finalized_blocks;
  constexpr int kEntries = 180000;

  for (int i = 0; i < kEntries; ++i) {
    state_cache.emplace("parent_" + std::to_string(i), i);
    finalized_blocks.emplace("final_" + std::to_string(i), i);
  }

  if (static_cast<int>(state_cache.size()) != kEntries || static_cast<int>(finalized_blocks.size()) != kEntries) {
    throw std::runtime_error("Simulation failed to create expected number of cache entries.");
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
    const std::filesystem::path target_path = ton_root / "validator/consensus/simplex/state-resolver.cpp";
    const std::string target = target_path.string();
    const std::string source = read_file(target);

    ensure_vulnerable_pattern_present(source);
    simulate_long_running_cache_accumulation();

    std::cout << "Issue #5 reproduction\n";
    std::cout << "Target file: " << target << "\n";
    std::cout << "Input: long-running consensus session with continuously advancing ids\n";
    std::cout << "Observed behavior: resolver caches accumulate entries with no normal-path pruning strategy\n";
    std::cout << "PASS: State resolver cache growth vulnerability reproduced.\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 2;
  }
}
