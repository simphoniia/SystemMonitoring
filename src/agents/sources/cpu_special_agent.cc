#include "../includes/cpu_special_agent.h"

#include "Utils/sub_functions.h"

s21::CpuSpecialAgent* s21::CreateObject() { return new s21::CpuSpecialAgent; }

void s21::CpuSpecialAgent::RefreshData(
    std::ofstream& file, std::chrono::steady_clock::time_point time,
    const std::string& timestamp) {
  if (!file.is_open()) return;
  if (!IsSetConfig()) return;
  if (std::chrono::duration_cast<std::chrono::seconds>(time - time_delta)
          .count() < update_time_)
    return;
  static const std::string get_cpu_idle_usage{
      "top -l 1 | grep 'CPU usage' | awk '{printf(\"%lf\", $7)}'"};
  static const std::string get_cpu_user_usage{
      "top -l 1 | grep 'CPU usage' | awk '{printf(\"%lf\", $3)}'"};
  static const std::string get_cpu_priveleged_usage{
      "top -l 1 | grep 'CPU usage' | awk '{printf(\"%lf\", $5)}'"};

  std::string cpu_idle_usage =
      SubFunctions::ExecCommand(get_cpu_idle_usage.c_str());
  std::string cpu_user_usage =
      SubFunctions::ExecCommand(get_cpu_user_usage.c_str());
  std::string cpu_priveleged_usage =
      SubFunctions::ExecCommand(get_cpu_priveleged_usage.c_str());

  try {
    cpu_idle_usage_ = std::stod(cpu_idle_usage);
    cpu_user_usage_ = std::stod(cpu_user_usage);
    cpu_priveleged_usage_ = std::stod(cpu_priveleged_usage);
  } catch (...) {
    std::cerr << "convertation error!";
  }
  file << "[<" << timestamp << ">]   "
       << "cpu_special_agent: idle: " << cpu_idle_usage_
       << " | user: " << cpu_user_usage_
       << " | priveleged: " << cpu_priveleged_usage_ << '\n';
  config_->SetCurrentSpecialCPU(cpu_idle_usage_, cpu_user_usage_,
                                cpu_priveleged_usage_);
  time_delta = time;
}

inline bool s21::CpuSpecialAgent::IsSetConfig() { return config_; }

inline void s21::CpuSpecialAgent::SetConfigFile(Config* config) {
  config_ = config;
}