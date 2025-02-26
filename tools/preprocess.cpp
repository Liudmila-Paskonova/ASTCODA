/*#########################################################################################################//
Tool for dataset preprocessing
//#########################################################################################################*/

#include <support/Support/Support.h>
#include <support/ArgParser/ArgParser.h>
#include <filesystem>
#include <algorithm>
#include <random>
#include <vector>
#include <numeric>
#include <set>
#include <print>
#include <format>
#include <fstream>
#include <ranges>
#include <unordered_map>

struct Parameters : public argparser::Arguments {
    size_t trainSplit;
    size_t testSplit;
    size_t validSplit;
    std::string tokensPath;
    std::string labelsPath;
    std::string outDir;

    Parameters()
    {
        using namespace argparser;

        addParam<"train_split">(trainSplit, NaturalRangeArgument<>(80, {0, 100}));
        addParam<"test_split">(testSplit, NaturalRangeArgument<>(10, {0, 100}));
        addParam<"valid_split">(validSplit, NaturalRangeArgument<>(10, {0, 100}));
        // <filename> <token1> <token2> ... <token m> ...
        addParam<"tokens_path">(tokensPath, FileArgument<std::string>("/home"));
        // <filename> <domain>_<class>
        addParam<"labels_path">(labelsPath, FileArgument<std::string>("/home"));
        addParam<"output_directory">(outDir, DirectoryArgument<std::string>("/home"));
    }
};

namespace fs = std::filesystem;

int
main(int argc, char *argv[])
{

    Parameters p;
    p.parse(argc, argv);

    if (p.testSplit + p.trainSplit + p.validSplit != 100) {
        throw std::format("Train, test and validation splits must be 100\% in sum, but it's {}!",
                          p.testSplit + p.trainSplit + p.validSplit);
    }

    std::string line;

    std::unordered_map<std::string, std::string> file2label;
    std::ifstream labelsFile(p.labelsPath);

    while (std::getline(labelsFile, line)) {
        auto arr = std::views::split(line, ' ') | std::ranges::to<std::vector<std::string>>();
        file2label[arr[0]] = arr[1];
    }
    labelsFile.close();

    size_t subNumber = 0;

    std::ifstream tokensFile(p.tokensPath);

    while (std::getline(tokensFile, line)) {
        subNumber++;
    }
    tokensFile.close();

    size_t trainNumber = subNumber / 100 * p.trainSplit;
    size_t validNumber = subNumber / 100 * p.validSplit;
    auto testNumber = subNumber - trainNumber - validNumber;

    auto classes = support::trainTestValidSplit(trainNumber, validNumber, testNumber);

    std::ofstream tokensTrainFile(p.outDir + "/tokens_train.txt");
    std::ofstream tokensValidFile(p.outDir + "/tokens_valid.txt");
    std::ofstream tokensTestFile(p.outDir + "/tokens_test.txt");
    std::ofstream labelsTrainFile(p.outDir + "/labels_train.txt");
    std::ofstream labelsValidFile(p.outDir + "/labels_valid.txt");
    std::ofstream labelsTestFile(p.outDir + "/labels_test.txt");
    size_t ind = 0;
    while (std::getline(tokensFile, line)) {
        auto arr = std::views::split(line, ' ');
        auto file_range = arr | std::ranges::views::take(1);
        auto subrange = *file_range.begin();
        auto filename = std::string(subrange.begin(), subrange.end());
        auto label = file2label[filename];

        auto sent = arr | std::ranges::views::drop(1) | std::views::join_with(' ') | std::ranges::to<std::string>();
        std::ofstream tokenStream, labelStream;
        if (classes[ind] == 0) {
            tokensTrainFile << sent << "\n";
            labelsTrainFile << label << "\n";
        } else if (classes[ind] == 1) {
            tokensValidFile << sent << "\n";
            labelsValidFile << label << "\n";
        } else if (classes[ind] == 2) {
            tokensTestFile << sent << "\n";
            labelsTestFile << label << "\n";
        }
    }
    tokensFile.close();
    tokensTrainFile.close();
    tokensValidFile.close();
    tokensTestFile.close();
    labelsTrainFile.close();
    labelsValidFile.close();
    labelsTestFile.close();
}
