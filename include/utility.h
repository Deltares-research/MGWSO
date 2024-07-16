#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <vector>
#if defined(WIN32) || defined(_WIN32) 
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

//!function to split a string based upon the delimiter given
inline std::vector<std::string> split_string(std::string input_string, char delimiter)
{
    size_t pos_start = 0, pos_end = 0;
    std::string token;
    std::vector<std::string> res;
    pos_end = input_string.find(delimiter, pos_start);
    while (pos_end != std::string::npos) {
        res.push_back(input_string.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + 1;
        pos_end = input_string.find(delimiter, pos_start);
    }
    res.push_back(input_string.substr(pos_start));
    return res;
}


//!function to convert a string to a time point
inline  std::chrono::system_clock::time_point string2timepoint(const std::string& datetimeString, const std::string& format)
{
    tm tmStruct = {};
    std::istringstream ss(datetimeString);
    ss >> std::get_time(&tmStruct, format.c_str());
    return std::chrono::system_clock::from_time_t(
        mktime(&tmStruct));
}

inline  std::string join_string(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, char delimiter = '.')
{
	std::string res;
	for (auto it = start; it != end; it++)
    {
		res += *it;
		if (it != end - 1)
		{
			res += delimiter;
		}
	}
	return res;
}


#if defined(WIN32) || defined(_WIN32)
// https://stackoverflow.com/questions/42946335/deprecated-header-codecvt-replacement
inline  std::string wide_string_to_string(const std::wstring& wide_string)
{
    if (wide_string.empty())
    {
        return "";
    }

    const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
    }

    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), result.data(), size_needed, nullptr, nullptr);
    return result;
}
#endif

inline std::string get_exe_path() {
    
#if defined(WIN32) || defined(_WIN32)
    LPSTR buffer = new TCHAR[MAX_PATH];
    if (GetModuleFileName(NULL, buffer, MAX_PATH) == 0) {
        throw std::runtime_error("Failed to get executable path on Windows");
    }
    return std::string(buffer);
#elif defined(__linux__)
    std::vector<char> buffer;
    buffer.resize(PATH_MAX);
    if (readlink("/proc/self/exe", &buffer[0], buffer.size()) == -1) {
        throw std::runtime_error("Failed to get executable path on Linux");
    }
    return std::string(buffer.begin(), buffer.end());
#elif defined(__APPLE__)
    std::vector<char> buffer;
    uint32_t size = PATH_MAX;
    buffer.resize(size);
    if (_NSGetExecutablePath(&buffer[0], &size) != 0) {
        throw std::runtime_error("Failed to get executable path on macOS");
    }
    return std::string(buffer.begin(), buffer.end());
#endif
    
}

