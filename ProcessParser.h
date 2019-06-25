#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
    private:
        std::ifstream stream;
        static string getProcessStatusElem(string statusFieldName, int elemIndex, string pid);
        static float getSysActiveCpuTime(vector<string> values);
        static float getSysIdleCpuTime(vector<string> values);
    public:
        static string getCmd(string pid);
        static vector<string> getPidList();
        static std::string getVmSize(string pid);
        static std::string getCpuPercent(string pid);
        static long int getSysUpTime();
        static std::string getProcUpTime(string pid);
        static string getProcUser(string pid);
        static vector<string> getSysCpuPercent(string coreNumber = "");
        static float getSysRamPercent();
        static string getSysKernelVersion();
        static int getNumberOfCores();
        static int getTotalThreads();
        static int getTotalNumberOfProcesses();
        static int getNumberOfRunningProcesses();
        static string getOSName();
        static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
        static bool isPidExisting(string pid);
};

// Private members
string ProcessParser::getProcessStatusElem(string statusFieldName, int elemIndex, string pid){    
    auto lines = Util::getFileLines(Path::basePath() + pid + Path::statusPath());
        
    for(auto line : lines){
        if(line.compare(0, statusFieldName.size(), statusFieldName) == 0){
            return Util::getStringElements(line)[elemIndex];
        }
    }

    throw runtime_error("Could not find process status element");
}



// TODO: Define all of the above functions below:
string ProcessParser::getCmd(string pid){
    return Util::getFileLines(Path::basePath() + pid + Path::cmdPath())[0];
}

vector<string> ProcessParser::getPidList(){
    vector<string> ret;

    DIR* dir;
    if(!(dir = opendir("/proc"))){
        throw runtime_error(strerror(errno));
    }

    while(dirent* dirp = readdir(dir)) {
        if(dirp->d_type != DT_DIR){
            continue;
        }

        if(all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c) {return isdigit(c);})){
            ret.push_back(dirp->d_name);
        }
    }

    if(closedir(dir)){
        throw runtime_error(strerror(errno));
    }

    return ret;
}

string ProcessParser::getVmSize(string pid){
    string elemName = "VmData";
    int elemIndex = 1;

    auto val = getProcessStatusElem(elemName, elemIndex, pid);

    return to_string(stof(val)/float(1024*1024));
}

string ProcessParser::getCpuPercent(string pid){
        
    auto lines = Util::getFileLines(Path::basePath() + pid + "/" + Path::statPath());
    auto values = Util::getStringElements(lines[0]);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]); 
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    
    return to_string(100.0*((total_time/freq)/seconds));
}

long int ProcessParser::getSysUpTime() {
    
    auto lines = Util::getFileLines(Path::basePath() + Path::upTimePath());
    auto values = Util::getStringElements(lines[0]);
       
    return stoi(values[0]);
}

string ProcessParser::getProcUpTime(string pid){
    
    auto lines = Util::getFileLines(Path::basePath() + pid + "/" + Path::statPath());
    auto values = Util::getStringElements(lines[0]);
       
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

string ProcessParser::getProcUser(string pid){
    string elemName = "Uid";
    int elemIndex = 1;

    auto val = getProcessStatusElem(elemName, elemIndex, pid);

    ifstream stream;
    Util::getStream("/etc/passwd", stream);
    
    auto token = "x:" + val;
    string line;
    while(getline(stream, line)){
        if(line.find(token) != std::string::npos) {
            return line.substr(0, line.find(":"));
        }
    }

     throw runtime_error("Could not user name for process");
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
    auto lines = Util::getFileLines(Path::basePath() + Path::statPath());

    auto name = "cpu" + coreNumber;
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            return Util::getStringElements(line);
        }
    }

    throw runtime_error("Could not user name for process");
}

float ProcessParser::getSysRamPercent(){
    
    string strMemAvailable = "MemAvailable:";
    string strMemFree = "MemFree:";
    string strBuffers = "Buffers:";

    auto lines = Util::getFileLines(Path::basePath() + Path::memInfoPath());
    
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    
    for (auto line : lines){
        if (total_mem != 0 && free_mem != 0)
            break;
        if (line.compare(0, strMemAvailable.size(), strMemAvailable) == 0) {
            total_mem = stof(Util::getStringElements(line)[1]);
        }
        if (line.compare(0, strMemFree.size(), strMemFree) == 0) {
            free_mem = stof(Util::getStringElements(line)[1]);
        }
        if (line.compare(0, strBuffers.size(), strBuffers) == 0) {
            buffers = stof(Util::getStringElements(line)[1]);
        }
    }
    
    return float(100.0*(1-(free_mem/(total_mem-buffers))));
}

string ProcessParser::getSysKernelVersion(){
    string name = "Linux version ";
    
    auto lines = Util::getFileLines(Path::basePath() + Path::versionPath());
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            return Util::getStringElements(line)[2];
        }
    }
    
    return "";
}

int ProcessParser::getNumberOfCores(){
    string name = "cpu cores";
    auto lines = Util::getFileLines(Path::basePath() + "cpuinfo");
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            return stoi(Util::getStringElements(line)[3]);
        }
    }
    return 0;
}

int ProcessParser::getTotalThreads(){
    int result = 0;
    string name = "Threads:";
    
    vector<string> pid_list = ProcessParser::getPidList();
    for (int i=0 ; i < pid_list.size(); i++) {
        string pid = pid_list[i];
        auto lines = Util::getFileLines(Path::basePath() + pid + Path::statusPath());
        for(auto line : lines){
            if(line.compare(0, name.size(), name) == 0){
                auto tokens = Util::getStringElements(line);
                result += stoi(tokens[1]);
            }
        }
    }
    
    return result;
}

int ProcessParser::getTotalNumberOfProcesses(){
    string name = "processes";
    auto lines = Util::getFileLines(Path::basePath() + Path::statPath());
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            return stoi(Util::getStringElements(line)[1]);
        }
    }
    return 0;
}

int ProcessParser::getNumberOfRunningProcesses(){
    string name = "procs_running";
    auto lines = Util::getFileLines(Path::basePath() + Path::statPath());
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            return stoi(Util::getStringElements(line)[1]);
        }
    }
    return 0;
}

string ProcessParser::getOSName(){
    string name = "PRETTY_NAME=";
    
    auto lines = Util::getFileLines("/etc/os-release");
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            std::size_t found = line.find("=");
            found++;
            string result = line.substr(found);
            result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
            return result;
        }
    }

    return "";
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string> values2){
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    
    return to_string(100.0*(activeTime / (activeTime + idleTime)));
}

bool ProcessParser::isPidExisting(string pid){
    return false;
}

float ProcessParser::getSysActiveCpuTime(vector<string> values){
    return(
        stof(values[S_USER]) +
        stof(values[S_NICE]) +
        stof(values[S_SYSTEM]) +
        stof(values[S_IRQ]) +
        stof(values[S_SOFTIRQ]) +
        stof(values[S_STEAL]) +
        stof(values[S_GUEST]) +
        stof(values[S_GUEST_NICE])
    );
}

float ProcessParser::getSysIdleCpuTime(vector<string> values){
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}
