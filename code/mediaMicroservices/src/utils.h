#ifndef MEDIA_MICROSERVICES_UTILS_H
#define MEDIA_MICROSERVICES_UTILS_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "logger.h"

namespace media_service {
using json = nlohmann::json;

inline int load_config_file(const std::string &file_name, json *config_json) {
  std::ifstream json_file;
  json_file.open(file_name);
  if (json_file.is_open()) {
    json_file >> *config_json;
    json_file.close();
    return 0;
  }
  LOG(error) << "Cannot open config file: " << file_name;
  return -1;
}

inline int load_service_config(json *config_json) {
  const char *config_env = std::getenv("SERVICE_CONFIG_FILE");
  const std::string config_path = config_env != nullptr && config_env[0] != '\0'
      ? std::string(config_env)
      : std::string("config/service-config.json");
  return load_config_file(config_path, config_json);
}

} // namespace media_service

#endif // MEDIA_MICROSERVICES_UTILS_H
