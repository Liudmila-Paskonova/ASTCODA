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
#include <chrono>
#include <unordered_map>
#include <format>

namespace extractor
{

/// Function that extracts all path-tokens
/// @param file - path to source file
/// @param params - struct with parameters
/// @param tempDir - directory to store temporary files
template <typename Parameters>
void
extract(const std::filesystem::path &file, const Parameters &params, const std::filesystem::path &tempDir)
{
    treesitter::Tree t(file, params.lang, params.traversal, params.token, params.split, params.minLen);
    auto res = t.process();
    if (res.size() > params.maxSize) {
        return;
    }
    std::string line = file.filename().stem();
    for (const auto &v : res) {
        line += " " + v;
    }
    line += "\n";

    std::stringstream ss;
    ss << std::this_thread::get_id();
    auto id = ss.str();

    auto outFile = tempDir / "tokens" / (id + ".txt");

    std::ofstream tempFile(outFile, std::ios::app);
    tempFile << line;
    tempFile.close();

    auto outVocab = tempDir / "vocabs" / (id + ".txt");
    std::ofstream tempVocab(outVocab, std::ios::app);
    for (auto &[hash, tok] : t.vocab) {
        tempVocab << "___[BOS]___ " << hash << " " << tok << "\n";
    }
    tempVocab.close();
}

/// Class that extracts path-tokens from files concurrently
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

        auto dirName = dirPath.filename().stem();
        std::filesystem::path tokensDir = outDirPath / dirName;
        std::filesystem::create_directory(tokensDir);
        std::filesystem::create_directory(tokensDir / "temp");
        std::filesystem::create_directory(tokensDir / "temp" / "tokens");
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

        // unite all files with path-contexts into one
        std::ofstream outFile(tokensDir /
                              (params.traversal + "__" + params.token + "__" + params.split + "__tokens.txt"));
        for (auto const &dir_entry : std::filesystem::directory_iterator{tokensDir / "temp" / "tokens"}) {
            std::ifstream f(dir_entry.path());
            outFile << f.rdbuf();
            f.close();
        }
        outFile.close();

        // create a vocabulary
        std::unordered_map<std::string, std::string> globalVocab;
        for (auto const &dir_entry : std::filesystem::directory_iterator{tokensDir / "temp" / "vocabs"}) {
            std::ifstream f(dir_entry.path());

            std::string line;
            std::string number;
            std::string content;
            while (std::getline(f, line, '\n')) {
                std::stringstream lineStream(line);
                std::string bos;
                std::getline(lineStream, bos, ' ');
                if (bos != "___[BOS]___") {
                    content += "\n" + line;
                } else {
                    // begin of string
                    if (!number.empty()) {
                        globalVocab[number] = content;
                    }
                    std::getline(lineStream, number, ' ');
                    std::getline(lineStream, content);
                }
            }

            if (!number.empty()) {
                globalVocab[number] = content;
            }

            f.close();
        }
        std::ofstream outVocab(tokensDir /
                               (params.traversal + "__" + params.token + "__" + params.split + "__mapping.txt"));
        outVocab << globalVocab.size() << "\n";
        for (auto &[hash, tok] : globalVocab) {
            outVocab << "___[BOS]___ " << hash << " " << tok << "\n";
        }

        outVocab.close();

        std::filesystem::remove_all(tokensDir / "temp");
    }
};
} // namespace extractor

#endif
