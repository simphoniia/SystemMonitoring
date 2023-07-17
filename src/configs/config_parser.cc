#include "config_parser.h"

/*
    TODO:
    Need to parse log file from end to begin;

*/


bool CreateFileWithData(const std::string& file_name, const std::string& folder_name, const std::vector<std::string>& data);
std::pair<double, kCompareType> GetPairOfData(const std::string& data, size_t pos);
int ParseCPU(std::ifstream& file, CPUAgentConfig& cpu_);

bool Compare(double val1, double val2, kCompareType& statement) {
    bool result = false;

    if (statement == kCompareType::kEqual) 
        result = (val1 == val2);
    else if (statement == kCompareType::kEqualGreater)
        result = (val1 >= val2);
    else if (statement == kCompareType::kEqualLess)
        result = (val1 <= val2);
    else if (statement == kCompareType::kGreater)
        result = (val1 > val2);
    else
        result = (val1 < val2);

    return result;
}

std::string Config::Update(std::ifstream& log_file) {
    int error{};
    std::string error_message;


    if (!IsExistDirectory()) 
        error = CreateDirectory();
    
    if (error) 
        error_message = "Could not to create a /agents_config/ directory";

    error = CheckFiles();

    if (error) 
        error_message = "Could not to create a config file.";

    ParseConfFiles();

    ParseLog(log_file);

    return error_message;
}

std::string Config::ParseLog(std::ifstream& log_file) {
    
    static std::vector<std::string> data = {
        "cpu: ",
        "processes: ",
    };
    
    size_t newest_info{};
    size_t offset;
    std::string line;

    size_t counter{};
    while (std::getline(log_file, line)) {
        if (line.find("TIMESTAMP:") != std::string::npos) 
            newest_info = counter;
        counter++;

        std::cout << "counter: " << counter << " newest: " << newest_info << "\n";
    }



    counter = 0;
    while (std::getline(log_file, line)) {
        if (newest_info >= counter) {
            counter++;
            continue;
        }

        offset = line.find(data[0]);
        if (offset != std::string::npos)
            std::cout << SubFunctions::GetOnlyDigits(line.substr(offset, line.find('|'))) << "\n";

    }
    


    return line;
}

void Config::ParseConfFiles() {

    std::vector<std::pair<std::string, int(*)(std::ifstream&, CPUAgentConfig&)>> files = {
        { "agents_config/cpu_agent.conf", ParseCPU }
    };
    
    std::ifstream current_file;
    
    for (auto& current_agent : files) {
        current_file.open(current_agent.first);
        if (current_file.is_open()) {
            current_agent.second(current_file, cpu_);
            current_file.close();
        }
    }
}

int ParseCPU(std::ifstream& file, CPUAgentConfig& cpu_) {
    int error_code = 0;

    static std::vector<std::string> need_data = {
        "agent_name=",
        "load_metric=\"",
        "processes_metric=\"",
        "update_time="
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(50);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset != std::string::npos) {
            offset += need_data[counter].size();
            if (counter == 0)
                cpu_.SetName(buffer.substr(offset, buffer.size()));
            else if (counter == 1 || counter == 2) {
                std::pair<double, kCompareType> pair_of_data = GetPairOfData(buffer, offset);
                if (pair_of_data.second == kCompareType::kNone) error_code = 1;
                std::cout << "Value is " << pair_of_data.first << "\n";
                if (!error_code)
                    counter == 1 ? cpu_.SetCPULoadConfig(pair_of_data) : cpu_.SetProcNumber(pair_of_data);
            } else if (counter == 3)
                cpu_.SetUpdateTime(std::stoi(SubFunctions::GetOnlyDigits(buffer)));
            counter++;
        }
    }
    
    return error_code;
}

std::pair<double, kCompareType> GetPairOfData(const std::string& data, size_t pos) {
    static std::string probably_condition = "=><";
    std::string condition;

    if (probably_condition.find(data[pos]) != std::string::npos)
        condition.push_back(data[pos]);

    if (probably_condition.find(data[pos + 1]) != std::string::npos)
        condition.push_back(data[pos + 1]);
    
    if (probably_condition.find(data[pos + 2]) != std::string::npos)
        condition.clear();

    static std::map<std::string, kCompareType> state_key = {
        { ">=", kEqualGreater },
        { ">", kGreater },
        { "==", kEqual },
        { "<", kLess },
        {"<=", kEqualLess }
    };

    kCompareType if_state = state_key.find(condition)->second;

    if (if_state == state_key.end()->second)
        if_state = kCompareType::kNone;

    std::string s_value = SubFunctions::GetOnlyDigits(data);
    double value = std::stod(s_value);

    return std::pair<double, kCompareType>(value, if_state);
}

inline bool Config::IsExistDirectory() {
    return std::filesystem::is_directory("./agents_config/");
}

int Config::CreateDirectory() {
    SubFunctions::ExecCommand("mkdir agents_config");

    int error = 0;
    error = !(IsExistDirectory());
    return error;
}

int Config::CheckFiles() {
    int error = 0;

    static std::vector<std::string> agents_config = {  
        "cpu_agent.conf",
        "memory_agent.conf",
        "network_agent.conf"
        //"special_agent.conf"
    };

    for (size_t i = 0; i < agents_config.size() && !error; i++) {
        std::string path = "./agents_config/" + agents_config[i];
        
        if (!std::filesystem::exists(path))
            error = CreateDefaultFiles(std::pair<std::string, int>(agents_config[i], i));
    }

    return error;
}

inline int CreateFile(const std::string& file_name, const std::string& folder) {
    SubFunctions::ExecCommand(std::string("touch " + folder + "/" + file_name).c_str());
    return !std::filesystem::exists(folder + "/" + file_name);
} 

int Config::CreateDefaultFiles(const std::pair<std::string, int>& agents_name) {
    int error = 0;

    static std::string folder_name = "agents_config";
    static std::vector<std::vector<std::string>> default_file_data = {
        {
            "agent_name=CPU_AGENT\n",
            "agent_type=cpu_agent\n\n",
            "load_metric=\">= 50\"\n",
            "processes_metric=\">= 1000\"\n",
            "\nupdate_time=4"
        }, 
        {
            "agent_name=MEMORY_AGENT\n",
            "agent_type=memory_agent\n\n",
            "total_metric=\"\"\n",
            "usage_metric=\">= 80\"\n",
            "volume_metric=\"<= 1.0\"\n",
            "hardops_metric=\"== 0\"\n",
            "hardthroughput_metric=\"== 0\"\n",
            "\nupdate_time=4\n"
        },
        {
            "agent_name=NETWORK_AGENT\n",
            "agent_type=network_agent\n\n",
            "url=\"2ip.ru\"\n",
            "inet_throughput_metric=\"> 0\"\n",
            "update_time=5\n"
        }
    };

    error = CreateFileWithData(agents_name.first, folder_name, default_file_data[agents_name.second]);

    return error;
}

inline void FillFile(std::ofstream& stream, const std::vector<std::string>& data) {
    for (auto& line : data) stream << line;
}

bool CreateFileWithData(const std::string& file_name, const std::string& folder_name, const std::vector<std::string>& data) {
    bool is_created_file = !CreateFile(file_name, folder_name);
    if (is_created_file) {
        std::ofstream file;
        file.open(folder_name + "/" + file_name);
        FillFile(file, data);
        file.close();
    }

    return !is_created_file;
}