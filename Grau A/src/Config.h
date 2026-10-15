#pragma once
// Leitor do arquivo de configuracao (assets/config.txt).
// Formato chave = valor   (linhas com # sao comentarios).
// A chave "layer" pode aparecer varias vezes (uma por camada de parallax).
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class Config
{
public:
    bool load(const std::string& path)
    {
        std::ifstream f(path);
        if (!f) return false;
        std::string line;
        while (std::getline(f, line))
        {
            auto hash = line.find('#');
            if (hash != std::string::npos) line = line.substr(0, hash);
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = trim(line.substr(0, eq));
            std::string val = trim(line.substr(eq + 1));
            if (key.empty()) continue;
            if (key == "layer") layers_.push_back(val);
            else values_[key] = val;
        }
        return true;
    }

    float num(const std::string& key, float def) const
    {
        auto it = values_.find(key);
        if (it == values_.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    }

    int integer(const std::string& key, int def) const { return static_cast<int>(num(key, static_cast<float>(def))); }

    std::string str(const std::string& key, const std::string& def) const
    {
        auto it = values_.find(key);
        return it == values_.end() ? def : it->second;
    }

    // Lista de numeros separados por espaco/virgula (ex. "4 6 2 4")
    std::vector<float> numbers(const std::string& key) const
    {
        std::vector<float> out;
        auto it = values_.find(key);
        if (it == values_.end()) return out;
        std::string s = it->second;
        for (char& c : s) if (c == ',') c = ' ';
        std::istringstream ss(s);
        float v;
        while (ss >> v) out.push_back(v);
        return out;
    }

    const std::vector<std::string>& layers() const { return layers_; }

private:
    static std::string trim(const std::string& s)
    {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    std::map<std::string, std::string> values_;
    std::vector<std::string> layers_;
};
