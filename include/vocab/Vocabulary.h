#ifndef VOCABULARY_VOCABULARY_H
#define VOCABULARY_VOCABULARY_H
#include <set>
#include <map>
#include <ranges>
#include <vector>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <support/Support/Support.h>

namespace vocabulary
{
using json = nlohmann::json;

/// Class to get token2idx, label2idx, domain2idx and hash2terminal vocabularies from data
class Vocabulary
{
    /// token2idx
    std::set<std::string> tokenSet;
    /// label2idx
    std::set<std::string> labelSet;
    /// domain2idx
    std::set<std::string> domainSet;
    /// Possible classes within a domain
    std::vector<std::string> classes;
    /// hash2terminal
    json terminalMap = json::object();

    /// Function that writes a vocabulary to the given file
    /// @param filepath - path to the file where to write the vocabulary
    /// @param vocab - vocabulary of strings
    void printVocab(const std::string &filepath, const std::set<std::string> &vocab);
    void printVocab(const std::string &filepath, const json &vocab);

  public:
    Vocabulary(const std::vector<std::string> &classesTemp);

    /// Function that appends unique path-tokens from the given file to a dictionary
    /// @param filepath - path to the file with path-tokens (each line is a " " split string)
    void addTokens(const std::string &filepath);

    /// Function that searches for unique labels and domains in the given filepath
    /// @param filepath - path to the file with labels (each line is a label)
    void addLabels(const std::string &filepath);

    /// Function that creates a mapping between terminals and thir hashed values
    /// @param filepath - path to the file with labels (each line is a label)
    void addTerminals(const std::string &filepath);

    /// Function that writes all the vocabularies to files
    /// @param tokensPath - path to the output token2idx file
    /// @param labelsPath - path to the output label2idx file
    /// @param domainsPath - path to the output domain2idx file
    void print(const std::string &tokensPath, const std::string &labelsPath, const std::string &domainsPath,
               const std::string &mappingPath);
};
} // namespace vocabulary
#endif
