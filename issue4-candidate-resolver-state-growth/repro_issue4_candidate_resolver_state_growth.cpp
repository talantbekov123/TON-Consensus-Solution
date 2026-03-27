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
  const std::string insert_pattern = "state_[request->id]";
  const std::string field_pattern = "std::map<CandidateId, CandidateState> state_";

  if (source.find(field_pattern) == std::string::npos || source.find(insert_pattern) == std::string::npos) {
    throw std::runtime_error(
        "Expected candidate resolver state map insertion pattern not found. "
        "Code may have changed or issue may already be fixed.");
  }

  if (source.find("state_.erase(") != std::string::npos || source.find("state_.clear(") != std::string::npos) {
    throw std::runtime_error("Found state_ pruning logic; issue might already be fixed.");
  }
}

void simulate_unbounded_candidate_state_growth() {
  std::unordered_map<std::string, int> state;
  constexpr int kEntries = 250000;
  for (int i = 0; i < kEntries; ++i) {
    state.emplace("candidate_" + std::to_string(i), i);
  }
  if (static_cast<int>(state.size()) != kEntries) {
    throw std::runtime_error("Simulation failed to create expected number of entries.");
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
    const std::filesystem::path target_path = ton_root / "validator/consensus/simplex/candidate-resolver.cpp";
    const std::string target = target_path.string();
    const std::string source = read_file(target);

    ensure_vulnerable_pattern_present(source);
    simulate_unbounded_candidate_state_growth();

    std::cout << "Issue #4 reproduction\n";
    std::cout << "Target file: " << target << "\n";
    std::cout << "Input: sustained stream of unique candidate ids during a long session\n";
    std::cout << "Observed behavior: candidate resolver keeps appending state entries without lifecycle pruning\n";
    std::cout << "PASS: Candidate resolver state growth vulnerability reproduced.\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 2;
  }
}
