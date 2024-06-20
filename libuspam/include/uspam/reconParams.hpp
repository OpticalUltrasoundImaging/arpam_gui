#pragma once
#include "uspam/io.hpp"
#include <armadillo>
#include <filesystem>
#include <rapidjson/document.h>
#include <vector>

namespace uspam::recon {
namespace fs = std::filesystem;

struct ReconParams {
  std::vector<double> filterFreq;
  std::vector<double> filterGain;
  int noiseFloor;
  int desiredDynamicRange;
  int rotateOffset;

  [[nodiscard]] rapidjson::Value
  serialize(rapidjson::Document::AllocatorType &allocator) const;
  static ReconParams deserialize(const rapidjson::Value &obj);
};

struct ReconParams2 {
  ReconParams PA;
  ReconParams US;

  static inline ReconParams2 system2024v1() {
    // NOLINTBEGIN(*-magic-numbers)
    ReconParams PA{
        {0, 0.03, 0.035, 0.2, 0.22, 1}, {0, 0, 1, 1, 0, 0}, 300, 35, 25};
    ReconParams US{{0, 0.1, 0.3, 1}, {0, 1, 1, 0}, 200, 48, 25};

    return ReconParams2{PA, US};
    // NOLINTEND(*-magic-numbers)
  }

  // Serialize to JSON
  [[nodiscard]] rapidjson::Document serializeToDoc() const;
  bool serializeToFile(const fs::path &path) const;

  // Deserialize from JSON
  bool deserialize(const rapidjson::Document &doc);
  bool deserializeFromFile(const fs::path &path);
};

} // namespace uspam::recon