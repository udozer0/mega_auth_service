#include "validation.hpp"
#include <algorithm>
#include <cctype>
#include <regex>

std::string to_lower_trim(const std::string& s) {
    auto l = s;
    l.erase(l.begin(), std::find_if(l.begin(), l.end(), [](unsigned char c){ return !std::isspace(c);} ));
    l.erase(std::find_if(l.rbegin(), l.rend(), [](unsigned char c){ return !std::isspace(c);} ).base(), l.end());
    std::transform(l.begin(), l.end(), l.begin(), [](unsigned char c){ return std::tolower(c); });
    return l;
}

bool is_valid_email(const std::string& email) {
    static const std::regex re(R"(^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$)", std::regex::icase);
    return std::regex_match(email, re);
}

bool is_strong_password(const std::string& p) {
    if (p.size() < 12) return false;
    bool hasL=false, hasU=false, hasD=false, hasS=false;
    for (unsigned char c : p) {
        if (std::islower(c)) hasL = true;
        else if (std::isupper(c)) hasU = true;
        else if (std::isdigit(c)) hasD = true;
        else hasS = true;
    }
    int classes = (hasL?1:0)+(hasU?1:0)+(hasD?1:0)+(hasS?1:0);
    return classes >= 3;
}
