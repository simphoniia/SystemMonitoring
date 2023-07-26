#include "agent_core.h"

using namespace std::chrono;

s21::AgentCore::AgentCore() { CheckNewAgents(); }

void s21::AgentCore::UpdateMetrics() {
  LogFileCreation();
  ChangeTimestamp();
  if (update_time_ - execution_time_ > 0)
    std::this_thread::sleep_for(milliseconds(update_time_ - execution_time_));

  auto time_begin = high_resolution_clock::now();

  std::thread check_agents{&AgentCore::CheckNewAgents, this};
  std::thread write_to_log(&AgentCore::WriteToLog, this);

  check_agents.join();
  write_to_log.join();

  auto time_end = high_resolution_clock::now();
  execution_time_ = duration_cast<milliseconds>(time_end - time_begin).count();
  std::cout << "ex_time = " << execution_time_ << '\n';

  file_.close();
}

void s21::AgentCore::CheckNewAgents() {
  static std::string folderPath = "objects";  // Путь к папке
  // /Users/sullustd/MonitoringSystem/src/core/agent_core.cc
  try {
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
      if (entry.is_regular_file()) {
        std::string filePath = entry.path().string();
        if (std::filesystem::path(filePath).extension() == ".dylib") {
          new_agents_.insert(filePath);
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
  }
  DylibCompile();
}

int s21::AgentCore::NumberOfActiveAgents() {
  int result{};
  for (auto it : agents_)
    if (it.second.first == true) ++result;
  return result;
}

void s21::AgentCore::DisableAgent(const std::string& filepath) {
  if (agents_.find(filepath) == agents_.end()) return;
  (*agents_.find(filepath)).second.first = false;
}

void s21::AgentCore::EnableAgent(const std::string& filepath) {
  if (agents_.find(filepath) == agents_.end()) return;
  (*agents_.find(filepath)).second.first = true;
}

s21::AgentCore::~AgentCore() {
  for (auto it : libs_) dlclose(it);
}

void s21::AgentCore::LogFileCreation() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::stringstream stream;
  stream << std::put_time(std::localtime(&time), "%Y-%m-%d");
  std::string date_string = stream.str();

  std::string fname =
      "logs/" + date_string + ".txt";  // путь к файлу (изменить!)
  file_.open(fname, std::ios::app);
}

void s21::AgentCore::ChangeTimestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::stringstream stream;
  stream << std::put_time(std::localtime(&time), "%H:%M:%S");
  if (NumberOfActiveAgents() != 0)
    file_ << "TIMESTAMP: <" << stream.str() << ">\n";
}

void s21::AgentCore::WriteToLog() {
  for (auto it : agents_) {
    if ((it).second.first == true) {
      (it).second.second->RefreshData(file_);
    }
  }
}

void s21::AgentCore::DylibCompile() {
  for (auto it : new_agents_) {
    void* libraryHandle = dlopen((it).c_str(), RTLD_LAZY);
    if (!libraryHandle) {
      throw std::out_of_range("NO FILE");
    }
    s21::BaseAgent* (*createFunction)() =
        reinterpret_cast<s21::BaseAgent* (*)()>(
            dlsym(libraryHandle, "CreateObject"));
    if (!createFunction) {
      throw std::out_of_range(
          "Не удалось найти функцию создания объекта в библиотеке");
      dlclose(libraryHandle);
    }
    std::shared_ptr<s21::BaseAgent> ptr{createFunction()};
    agents_.insert({it, {true, ptr}});
    libs_.push_back(libraryHandle);
  }
}

void s21::AgentCore::SetConfigFile(Config* config) {
  if (config_ != config) config_ = config;

  for (auto& agents : agents_) {
    if ((agents).second.first == true) {
      (agents).second.second->SetConfigFile(config_);
    }
  }
}
