#include "config_parser.h"

std::string PrintEnum(kCompareType type) {
    if (type == kCompareType::kEqual) 
        return "==";
    else if (type == kCompareType::kEqualGreater)
        return ">=";
    else if (type == kCompareType::kEqualLess)
        return "<=";
    else if (type == kCompareType::kGreater)
        return ">";
    else
        return "<";
}


bool CreateFileWithData(const std::string& file_name, const std::string& folder_name, const std::vector<std::string>& data);
std::pair<double, kCompareType> GetPairOfData(const std::string& data, size_t pos);

int ParseCPU(std::ifstream& file, CPUAgentConfig& cpu_);
int ParseMemory(std::ifstream& file, MemoryAgentConfig& mem_);
int ParseNetwork(std::ifstream& file, NetworkAgentConfig& netw_);
int ParseCpecial(std::ifstream& file, CPUSpecialAgentConfig& cpuspec_);

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

bool Compare(size_t val1, size_t val2, kCompareType& statement) {
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

void Config::SetCurrentCPU(double cpu_loading, size_t process_count) {
    int result = cpu_.Compare(cpu_loading, process_count);

    if (result == 1)
        error_msg = ("CPU loading fail! Current " + std::to_string(cpu_loading));
    else if (result == 2)
        error_msg = ("CPU process count fail! Current " + std::to_string(process_count));
}

void Config::SetCurrentMemory(double total, double usage, double volume,
            size_t hardops, double throughput) {
                int result = mem_.Compare(total, usage, volume, hardops, throughput);


                std::cout << "Compare result: " << result << "\n";
                if (result == 1)
                    error_msg = ("Memory total fail! Current " + std::to_string(total));
                if (result == 2)
                    error_msg = ("Memory usage fail! Current " + std::to_string(usage));
                if (result == 3)
                    error_msg = ("Memory volume fail! Current " + std::to_string(volume));
                if (result == 4)
                    error_msg = ("Memory hardops fail! Current " + std::to_string(hardops));
                if (result == 5)
                    error_msg = ("Memory throughput fail! Current " + std::to_string(throughput));

}

void Config::SetCurrentNetwork(double inet_throughput, bool is_site_up) {
    int result = netw_.Compare(inet_throughput);

    if (result == 1)
        error_msg = ("Network fail! Current " + std::to_string(inet_throughput));
    if (!is_site_up)
        error_msg = ("Network fail! Site is down!");
}

void Config::SetCurrentSpecialCPU(double idle, double user, double priveleged) {
    int result = cpuspec_.Compare(idle, user, priveleged);

    if (result == 1)
        error_msg = ("CPUSpecial idle fail! Current " + std::to_string(idle));
    if (result == 2)
        error_msg = ("CPUSpecial user fail! Current " + std::to_string(user));
    if (result == 3)
        error_msg = ("CPUSpecial priveleged fail! Current " + std::to_string(priveleged));
}

std::string Config::Update() {
    std::string error_message;

    if (!IsExistDirectory()) {
        if (CreateDirectory())
            error_message = "Could not to create a /agents_config/ directory";
    }

    if (CheckFiles())
        error_message = "Could not to create a config file.";

    ParseConfFiles();

    if (error_message.empty() && !error_msg.empty()) {
        error_message = error_msg;
        error_msg.clear();
    }

    return error_message;
}

void Config::ParseConfFiles() {
    std::ifstream current_file;
    
    current_file.open("agents_config/cpu_agent.conf");
    ParseCPU(current_file, cpu_);
    current_file.close();

    current_file.open("agents_config/memory_agent.conf");
    ParseMemory(current_file, mem_);
    current_file.close();

    current_file.open("agents_config/network_agent.conf");
    ParseNetwork(current_file, netw_);
    current_file.close();

    current_file.open("agents_config/special_agent.conf");
    ParseNetwork(current_file, netw_);
    current_file.close();

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
                cpu_.name = (buffer.substr(offset, buffer.size()));
            else if (counter == 1 || counter == 2) {
                std::pair<double, kCompareType> pair_of_data = GetPairOfData(buffer, offset);
                if (pair_of_data.second == kCompareType::kNone) error_code = 1;
                if (!error_code) {
                    if (counter == 1)
                        cpu_.load = pair_of_data;
                    else
                        cpu_.proc_num = pair_of_data;
                }
            } else if (counter == 3)
                cpu_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));
            counter++;
        }
    }
    
    return error_code;
}

int ParseMemory(std::ifstream& file, MemoryAgentConfig& mem_) {
    int error_code = 0;

    static std::vector<std::string> need_data = {
        "agent_name=",
        "total_metric=\"",
        "usage_metric=\"",
        "volume_metric=\"",
        "hardops_metric=\"",
        "hardthroughput_metric=\"",
        "update_time="
    };


    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(50);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset == std::string::npos) continue;
        offset += need_data[counter].size();

        if (counter == 0)
            mem_.name = (buffer.substr(offset, buffer.size()));

        if (counter > 0 && counter < 6) {
            std::pair<double, kCompareType> pair_of_data = GetPairOfData(buffer, offset);
            if (pair_of_data.second == kCompareType::kNone && counter != 1) error_code = 1;

            if (!error_code && counter == 1)
                mem_.ram = pair_of_data;

            if (!error_code && counter == 2)
                mem_.usage = pair_of_data;

            if (!error_code && counter == 3)
                mem_.volume = pair_of_data;
            
            if (!error_code && counter == 4)
                mem_.hardops = pair_of_data;

            if (!error_code && counter == 5)
                mem_.throughput = pair_of_data;

        }
            
        if (counter == 6)
            mem_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

        counter++;
    }

    return error_code;
}

int ParseNetwork(std::ifstream& file, NetworkAgentConfig& netw_) {
    int error_code = 0;

    static std::vector<std::string> need_data = {
        "agent_name=",
        "url=\"",
        "inet_throughput_metric=\"",
        "update_time=",
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(50);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset == std::string::npos) continue;
        offset += need_data[counter].size();

        if (counter == 0)
            netw_.name = (buffer.substr(offset, buffer.size()));
        
        if (counter == 1) {
            netw_.network_url = (buffer.substr(offset, buffer.size()));
            if (netw_.network_url.back() == '"')
                netw_.network_url.pop_back();
        }
            
        if (counter == 2)
            netw_.inet_throughput = GetPairOfData(buffer, offset);

        if (counter == 3)
            netw_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

        counter++;
    }

    return error_code;
}

int ParseCpecial(std::ifstream& file, CPUSpecialAgentConfig& cpuspec_) {
    int error_code = 0;

    static std::vector<std::string> need_data = {
        "agent_name=",
        "idle=\"",
        "user_usage=\"",
        "priveleged=\"",
        "update_time=",
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(50);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset == std::string::npos) continue;
        offset += need_data[counter].size();

        if (counter == 0)
            cpuspec_.name = (buffer.substr(offset, buffer.size()));
        
        if (counter == 1)
            cpuspec_.idle = GetPairOfData(buffer, offset);
        
        if (counter == 2)
            cpuspec_.user = GetPairOfData(buffer, offset);

        if (counter == 3)
            cpuspec_.priveleged = GetPairOfData(buffer, offset);

        if (counter == 4)
            cpuspec_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

        counter++;
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
    double value{};
    try {
        if (!s_value.empty())
            value = std::stod(s_value);
    } catch (...) {
        value = 0;
    }

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
        "network_agent.conf",
        "special_agent.conf"
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
        },
        {
            "agent_name=CPU_SPECIAL_AGENT\n",
            "idle=\">= 1000\"\n",
            "user_usage=\">= 1000\"\n",
            "priveleged=\">= 1000\"\n",
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
