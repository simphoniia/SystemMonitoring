#include "config_parser.h"


/* ================== 
===       TODO:       ===
=== Remove this func  ===
===   Debug-function  ===
   ================== */
std::string PrintEnum(kCompareType type) {
    if (type == kCompareType::kEqual) 
        return "==";
    else if (type == kCompareType::kEqualGreater)
        return ">=";
    else if (type == kCompareType::kEqualLess)
        return "<=";
    else if (type == kCompareType::kGreater)
        return ">";
    else if (type == kCompareType::kNotEqual)
        return "!=";
    else if (type == kCompareType::kLess)
        return "<";
    return "NON REACHEABLE";
}

bool CreateFileWithData(const std::string& file_name, const std::string& folder_name, const std::vector<std::string>& data);
std::pair<double, kCompareType> GetPairOfData(const std::string& data, size_t pos);

int ParseCPU(std::ifstream& file, CPUAgentConfig& cpu_);
int ParseMemory(std::ifstream& file, MemoryAgentConfig& mem_);
int ParseNetwork(std::ifstream& file, NetworkAgentConfig& netw_);
int ParseSpecial(std::ifstream& file, CPUSpecialAgentConfig& cpuspec_);
int ParseSwap(std::ifstream& file, SwapAgentConfig& swap_);
int ParseSystem(std::ifstream& file, SystemAgentConfig& system_);
int ParseVMemory(std::ifstream& file, VMemoryAgentConfig& vmem_);

bool Compare(size_t val1, size_t val2, kCompareType& statement) {
    bool result{};

    switch (statement) {
        case kCompareType::kEqual:
            result = (val1 == val2);
            break;
        case kCompareType::kEqualGreater:
            result = (val1 >= val2);
            break;
        case kCompareType::kEqualLess:
            result = (val1 <= val2);
            break;
        case kCompareType::kGreater:
            result = (val1 > val2);
            break;
        case kCompareType::kNotEqual:
            result = (val1 != val2);
            break;
        case kCompareType::kLess:
            result = (val1 < val2);
            break;
        default:
            std::cerr << "Unreacheble code.\n";
    };

    return result;
}

bool Compare(long val1, long val2, kCompareType& statement) {
    bool result = false;

    if (statement == kCompareType::kEqual) 
        result = (val1 == val2);
    else if (statement == kCompareType::kEqualGreater)
        result = (val1 >= val2);
    else if (statement == kCompareType::kEqualLess)
        result = (val1 <= val2);
    else if (statement == kCompareType::kGreater)
        result = (val1 > val2);
    else if (statement == kCompareType::kNotEqual)
        result = (val1 != val2);
    else
        result = (val1 < val2);

    return result;
}

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
    else if (statement == kCompareType::kNotEqual)
        result = (val1 != val2);
    else
        result = (val1 < val2);

    return result;
}

bool Compare(int val1, int val2, kCompareType& statement) {
    bool result = false;

    if (statement == kCompareType::kEqual) 
        result = (val1 == val2);
    else if (statement == kCompareType::kEqualGreater)
        result = (val1 >= val2);
    else if (statement == kCompareType::kEqualLess)
        result = (val1 <= val2);
    else if (statement == kCompareType::kGreater)
        result = (val1 > val2);
    else if (statement == kCompareType::kNotEqual)
        result = (val1 != val2);
    else
        result = (val1 < val2);

    return result;
}

inline void Config::WriteErrorIfFail(const std::pair<kAgentError, std::string>& result) {
    if (result.first != kOk)
        error_msg = std::move(result.second);
}

void Config::SetCurrentCPU(double cpu_loading, size_t process_count) {
    std::pair<kAgentError, std::string> result = cpu_.Compare(cpu_loading, process_count);
    WriteErrorIfFail(result);
}

void Config::SetCurrentMemory(double total, double usage, double volume,
            size_t hardops, double throughput) {
    std::pair<kAgentError, std::string> result = 
        mem_.Compare(total, usage, volume, hardops, throughput);
    WriteErrorIfFail(result);
}

void Config::SetCurrentNetwork(double inet_throughput, bool is_site_up) {
    std::pair<kAgentError, std::string> result = 
        netw_.Compare(inet_throughput, is_site_up);
    WriteErrorIfFail(result);
}

void Config::SetCurrentSpecialCPU(double idle, double user, double priveleged) {
    std::pair<kAgentError, std::string> result = 
        cpuspec_.Compare(idle, user, priveleged);
    WriteErrorIfFail(result);
}

void Config::SetCurrentSwap(double swap, double usedswap, double proc_queue) {
    std::pair<kAgentError, std::string> result = 
        swap_.Compare(swap, usedswap, proc_queue);
    WriteErrorIfFail(result);
}

void Config::SetCurrentSystem(long inodes, double hardreadtime, int errors, int auths, int disknum) {
    std::pair<kAgentError, std::string> result = 
        system_.Compare(inodes, hardreadtime, errors, auths, disknum);
    WriteErrorIfFail(result);
}

void Config::SetCurrentVMemory(double volume, double free) {
    std::pair<kAgentError, std::string> result = 
        vmem_.Compare(volume, free);
    WriteErrorIfFail(result);
}

std::string Config::Update() {
    std::string error_message;

    if (!IsExistDirectory() && CreateDirectory())
        error_message = "Could not to create a /agents_config/ directory";

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
    ParseSpecial(current_file, cpuspec_);
    current_file.close();

    current_file.open("agents_config/swap_agent.conf");
    ParseSwap(current_file, swap_);
    current_file.close();

    current_file.open("agents_config/system_agent.conf");
    ParseSystem(current_file, system_);
    current_file.close();

    current_file.open("agents_config/vmemory_agent.conf");
    ParseVMemory(current_file, vmem_);
    current_file.close();
}

int ParseCPU(std::ifstream& file, CPUAgentConfig& cpu_) {
    int error_code{};

    static std::vector<std::string> need_data = {
        "agent_name=",
        "load_metric=\"",
        "processes_metric=\"",
        "update_time="
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(70);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset != std::string::npos) {
            offset += need_data[counter].size();
            if (counter == 0)
                cpu_.name = (buffer.substr(offset, buffer.size()));

            if (counter == 1)
                cpu_.load = GetPairOfData(buffer, offset);

            if (counter == 2)
                cpu_.proc_num = GetPairOfData(buffer, offset);

            if (counter == 3)
                cpu_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

            counter++;
        }
    }
    
    return error_code;
}

int ParseMemory(std::ifstream& file, MemoryAgentConfig& mem_) {
    int error_code{};

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
    buffer.reserve(70);

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
    int error_code{};

    static std::vector<std::string> need_data = {
        "agent_name=",
        "url=\"",
        "inet_throughput_metric=\"",
        "update_time=",
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(70);

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

int ParseSpecial(std::ifstream& file, CPUSpecialAgentConfig& cpuspec_) {
    int error_code{};

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
    buffer.reserve(70);

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

int ParseSwap(std::ifstream& file, SwapAgentConfig& swap_) {
    int error_code{};

    static std::vector<std::string> need_data = {
        "agent_name=",
        "total=\"",
        "used=\"",
        "proc_queue=\"",
        "update_time=",
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(70);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset == std::string::npos) continue;
        offset += need_data[counter].size();

        if (counter == 0)
            swap_.name = (buffer.substr(offset, buffer.size()));
        
        if (counter == 1)
            swap_.total_swap = GetPairOfData(buffer, offset);
        
        if (counter == 2)
            swap_.used_swap = GetPairOfData(buffer, offset);

        if (counter == 3)
            swap_.proc_queue = GetPairOfData(buffer, offset);

        if (counter == 4)
            swap_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

        counter++;
    }

    return error_code;
}


int ParseSystem(std::ifstream& file, SystemAgentConfig& system_) {
    int error_code{};

    static std::vector<std::string> need_data = {
        "agent_name=",
        "inodes=\"",
        "hard_read_time=\"",
        "system_errors=\"",
        "user_auths=\"",
        "disk_number=\"",
        "update_time=",
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(70);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset == std::string::npos) continue;
        offset += need_data[counter].size();

        if (counter == 0)
            system_.name = (buffer.substr(offset, buffer.size()));
        
        if (counter == 1)
            system_.inodes = GetPairOfData(buffer, offset);

        if (counter == 2)
            system_.hard_read_time = GetPairOfData(buffer, offset);

        if (counter == 3)
            system_.system_errors = GetPairOfData(buffer, offset);

        if (counter == 4)
            system_.user_auths = GetPairOfData(buffer, offset);

        if (counter == 5)
            system_.disknum = GetPairOfData(buffer, offset);

        if (counter == 6)
            system_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

        counter++;
    }

    return error_code;
}

int ParseVMemory(std::ifstream& file, VMemoryAgentConfig& vmem_) {
    int error_code{};

    static std::vector<std::string> need_data = {
        "agent_name=",
        "volume=\"",
        "mem_free=\"",
        "free=\"",
        "update_time=",
    };

    size_t counter{};
    size_t offset;
    std::string buffer;
    buffer.reserve(70);

    while (std::getline(file, buffer)) {
        offset = buffer.find(need_data[counter]);
        if (offset == std::string::npos) continue;
        offset += need_data[counter].size();

        if (counter == 0)
            vmem_.name = (buffer.substr(offset, buffer.size()));
        
        if (counter == 1)
            vmem_.vmem_volume = GetPairOfData(buffer, offset);

        if (counter == 2)
            vmem_.vmem_free = GetPairOfData(buffer, offset);

        if (counter == 3)
            vmem_.update_time = (std::stof(SubFunctions::GetOnlyDigits(buffer)));

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
        { "<=", kEqualLess },
        { "!=", kNotEqual }
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

inline bool Config::CreateDirectory() {
    SubFunctions::ExecCommand("mkdir agents_config");
    return !(IsExistDirectory());
}

int Config::CheckFiles() {
    static const std::vector<std::string> agents_config = {  
        "cpu_agent.conf",
        "memory_agent.conf",
        "network_agent.conf",
        "special_agent.conf",
        "swap_agent.conf",
        "system_agent.conf",
        "vmemory_agent.conf"
    };

    int error{};
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

bool Config::CreateDefaultFiles(const std::pair<std::string, int>& agents_name) {
    static const std::string folder_name = "agents_config";
    static const std::vector<std::vector<std::string>> default_file_data = {
        {
            "agent_name=CPU_AGENT\n",
            "load_metric=\">= 50\"\n",
            "processes_metric=\">= 1000\"\n",
            "update_time=4"
        }, 
        {
            "agent_name=MEMORY_AGENT\n",
            "total_metric=\"\"\n",
            "usage_metric=\">= 80\"\n",
            "volume_metric=\"<= 1.0\"\n",
            "hardops_metric=\"== 0\"\n",
            "hardthroughput_metric=\"== 0\"\n",
            "update_time=4"
        },
        {
            "agent_name=NETWORK_AGENT\n",
            "url=\"2ip.ru\"\n",
            "inet_throughput_metric=\"> 0\"\n",
            "update_time=5"
        },
        {
            "agent_name=CPU_SPECIAL_AGENT\n",
            "idle=\">= 1000\"\n",
            "user_usage=\">= 1000\"\n",
            "priveleged=\">= 1000\"\n",
            "update_time=5"
        },
        {
            "agent_name=SWAP_AGENT\n",
            "total=\">= 1000\"\n",
            "used=\">= 1000\"\n",
            "proc_queue=\">= 1000\"\n",
            "update_time=5"
        },
        {
            "agent_name=SYSTEM_AGENT\n",
            "inodes=\"> 0\"\n",
            "hard_read_time=\">= 80\"\n",
            "system_errors=\"<= 1.0\"\n",
            "user_auths=\"!= 0\"\n",
            "disk_number=\"!= 0\"\n",
            "update_time=4"
        },
        {
            "agent_name=VMEMORY_AGENT\n",
            "volume=\"> 34000\"\n",
            "mem_free=\">= 13000\"\n",
            "update_time=4"
        }
    };

    return CreateFileWithData(agents_name.first, folder_name, default_file_data[agents_name.second]);;
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
