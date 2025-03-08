#ifndef EXTRACTOR_EXTRACTOR_H
#define EXTRACTOR_EXTRACTOR_H

#include <support/TreeSitter/TreeSitter.h>
#include <support/ThreadPool/ThreadPool.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>
#include <map>
#include <ranges>
#include <concepts>
#include <set>
#include <chrono>
#include <unordered_map>
#include <format>
#include <nlohmann/json.hpp>

namespace extractor
{
using json = nlohmann::json;

/// Function that extracts all path-tokens
/// @param file - path to source file
/// @param params - struct with parameters
/// @param tempDir - directory to store temporary files
template <typename Parameters>
void
extract(const std::filesystem::path &file, const Parameters &params, const std::filesystem::path &tempDir)
{
    treesitter::Tree t(file, params.lang);
    auto res = t.process(params.traversal, params.token, params.split, params.minLen);
    if (res.size() > params.maxSize) {
        return;
    }
    std::string line;
    for (const auto &v : res) {
        line += v + " ";
    }
    line.back() = '\n';

    std::stringstream ss;
    ss << std::this_thread::get_id();
    auto id = ss.str();

    auto outFile = tempDir / "tokens" / (id + ".txt");
    auto outSubs = tempDir / "submissions" / (id + ".txt");
    auto outVocab = tempDir / "vocabs" / (id + ".json");
    {
        std::ofstream tempFile(outFile, std::ios::app);
        tempFile << line;
        tempFile.close();
    }

    {
        std::ofstream tempFile(outSubs, std::ios::app);
        tempFile << file.filename().string() << "\n";
        tempFile.close();
    }

    json threadVocab = json::object();

    {
        if (std::filesystem::exists(outVocab)) {
            std::ifstream tempVocab(outVocab);
            tempVocab >> threadVocab;
            tempVocab.close();
        }
    }

    for (auto &[hash, tok] : t.vocab) {
        threadVocab[std::to_string(hash)] = tok;
    }

    {
        std::ofstream tempVocab(outVocab);
        tempVocab << threadVocab.dump(4);
        tempVocab.close();
    }
}

/// Class that extracts path-tokens from files concurrently
/// >> tokens.txt
/// >> labels.txt
/// >> submissions.txt
/// >> mapping.json
/// @brief - Uses threadpool
class Extractor
{

  public:
    /// Function that runs extractor
    /// @param params - struct with parameters
    template <typename Parameters>
    void
    run(const Parameters &params)
    {
        std::filesystem::path dirPath = params.dir;
        std::filesystem::path outDirPath = params.outdir;
        std::filesystem::path labelsPath = params.mapping;

        auto dirName = dirPath.filename().stem();
        std::filesystem::path tokensDir = outDirPath / dirName;
        std::filesystem::create_directory(tokensDir);
        std::filesystem::create_directory(tokensDir / "temp");
        std::filesystem::create_directory(tokensDir / "temp" / "tokens");
        std::filesystem::create_directory(tokensDir / "temp" / "submissions");
        std::filesystem::create_directory(tokensDir / "temp" / "vocabs");

        std::vector<std::filesystem::path> filePaths;
        for (auto const &dir_entry : std::filesystem::directory_iterator{dirPath}) {
            filePaths.push_back(dir_entry.path());
        }

        std::filesystem::path tempDir = tokensDir / "temp";

        // run threadpool
        {
            threadpool::ThreadPool pool(params.numThreads);
            for (auto &file : filePaths) {
                auto res =
                    pool.addTask(extractor::extract<Parameters>, std::ref(file), std::ref(params), std::ref(tempDir));
            }
        }

        std::set<std::string> threadIDs;
        for (auto const &dir_entry : std::filesystem::directory_iterator{tokensDir / "temp" / "tokens"}) {
            threadIDs.insert(dir_entry.path().stem().string());
        }

        {
            // unite all files with path-contexts into one
            std::ofstream outFile(tokensDir / "tokens.txt");
            for (auto const &threadID : threadIDs) {
                std::ifstream f(tokensDir / "temp" / "tokens" / (threadID + ".txt"));
                outFile << f.rdbuf();
                f.close();
            }
            outFile.close();
        }

        {
            // unite all files with submissions into one
            std::ofstream outFile(tokensDir / "submissions.txt");
            for (auto const &threadID : threadIDs) {
                std::ifstream f(tokensDir / "temp" / "submissions" / (threadID + ".txt"));
                outFile << f.rdbuf();
                f.close();
            }
            outFile.close();
        }
        std::unordered_map<std::string, std::string> sub2label;

        {
            std::ifstream labelsVocab(labelsPath);
            std::string line;
            while (std::getline(labelsVocab, line)) {
                auto arr = std::views::split(line, ',') | std::ranges::to<std::vector<std::string>>();
                sub2label[arr[0]] = arr[1];
            }
            labelsVocab.close();
        }

        {
            // unite all files with submissions into one
            std::ofstream outFile(tokensDir / "labels.txt");
            std::ifstream inFile(tokensDir / "submissions.txt");
            std::string line;
            while (std::getline(inFile, line)) {
                outFile << sub2label[line] << "\n";
            }
            inFile.close();
            outFile.close();
        }

        // create a global vocabulary
        json globalVocab = json::object();

        for (auto const &dir_entry : std::filesystem::directory_iterator{tokensDir / "temp" / "vocabs"}) {
            std::ifstream f(dir_entry.path());
            json tempJson = json::object();

            f >> tempJson;
            f.close();

            for (auto &[key, value] : tempJson.items()) {
                globalVocab[key] = value;
            }
        }
        {
            std::ofstream outVocab(tokensDir / "mapping.json");
            outVocab << globalVocab.dump(4);
            outVocab.close();
        }

        std::filesystem::remove_all(tokensDir / "temp");
    }
};
} // namespace extractor

#endif
