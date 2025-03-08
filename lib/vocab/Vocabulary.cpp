#include <vocab/Vocabulary.h>

void
vocabulary::Vocabulary::printVocab(const std::string &filepath, const std::set<std::string> &vocab)
{
    std::ofstream f(filepath);
    for (auto &v : vocab) {
        f << v << "\n";
    }
    f.close();
}

void
vocabulary::Vocabulary::printVocab(const std::string &filepath, const json &vocab)
{
    std::ofstream f(filepath);
    f << terminalMap.dump(4);
    f.close();
}

vocabulary::Vocabulary::Vocabulary(const std::vector<std::string> &classesTemp) : classes(classesTemp)
{
    std::sort(classes.begin(), classes.end());
};

void
vocabulary::Vocabulary::addTokens(const std::string &filepath)
{
    std::ifstream f(filepath);
    for (const auto &line : std::ranges::istream_view<std::string>(f)) {

        auto tokens = support::splitLine(line, ' ');
        for (auto &token : tokens) {
            tokenSet.insert(token);
        }
    }
    f.close();
}

void
vocabulary::Vocabulary::addLabels(const std::string &filepath)
{
    std::ifstream f(filepath);
    for (const auto &line : std::ranges::istream_view<std::string>(f)) {

        auto tokens = support::splitLine(line, '_');
        if (!domainSet.contains(tokens[0])) {
            domainSet.insert(tokens[0]);
            for (auto &cl : classes) {
                labelSet.insert(tokens[0] + "_" + cl);
            }
        }
    }
    f.close();
}

void
vocabulary::Vocabulary::addTerminals(const std::string &filepath)
{

    std::ifstream f(filepath);
    json tempJson = json::object();
    f >> tempJson;
    f.close();

    for (auto &[key, value] : tempJson.items()) {
        terminalMap[key] = value;
    }
}

void
vocabulary::Vocabulary::print(const std::string &tokensPath, const std::string &labelsPath,
                              const std::string &domainsPath, const std::string &mappingPath)
{
    printVocab(tokensPath, tokenSet);
    printVocab(labelsPath, labelSet);
    printVocab(domainsPath, domainSet);
    printVocab(mappingPath, terminalMap);
}
