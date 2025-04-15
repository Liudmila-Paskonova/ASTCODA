#include <model/Model.h>
#include <iostream>
#include <string>
#include <support/ArgParser/ArgParser.h>
#include <support/Support/Support.h>
#include <filesystem>
#include <map>

struct Parameters : public argparser::Arguments {
    size_t embDim;
    size_t kernelSize;
    size_t numFilters;
    std::string pathDomainIdx;
    std::string pathLabelIdx;
    std::string pathTestX;
    std::string pathTestY;
    std::string pathModel;
    std::string lang;
    std::string outPath;
    size_t minLen;
    double threshold;

    Parameters()
    {
        using namespace argparser;

        addParam<"test_x">(pathTestX, DirectoryArgument<std::string>());
        addParam<"test_y">(pathTestY, FileArgument<std::string>());
        addParam<"label_to_idx">(pathLabelIdx, FileArgument<std::string>());
        addParam<"domain_to_idx">(pathDomainIdx, FileArgument<std::string>());
        addParam<"embedding_dim">(embDim, RangeArgument<size_t>({1, INT_MAX}));
        addParam<"kernel_size">(kernelSize, RangeArgument<size_t>({1, INT_MAX}));
        addParam<"num_filters">(numFilters, RangeArgument<size_t>({1, INT_MAX}));
        addParam<"weights_path">(pathModel, DirectoryArgument<std::string>());
        addParam<"lang">(lang, ConstrainedArgument<std::string>({"c", "cpp"}));
        addParam<"minlen">(minLen, RangeArgument<size_t>({1, INT_MAX}));
        addParam<"threshold">(threshold, RangeArgument<double>({-1.0, 1.0}));
        addParam<"chosen_lines">(outPath, FileArgument<std::string>(false));
    }
};

int
main(int argc, char *argv[])
{
    try {
        Parameters params;
        params.fromJSON(argv[1]);

        size_t embDim = params.embDim;
        size_t kernelSize = params.kernelSize;
        size_t numFilters = params.numFilters;

        // read domains
        std::map<std::string, size_t> domain2idx;
        std::ifstream domain2idxFile(params.pathDomainIdx);
        std::string line;
        size_t numDomains = 0;
        while (std::getline(domain2idxFile, line)) {
            domain2idx[line] = numDomains++;
        }
        domain2idxFile.close();

        // read labels
        std::ifstream label2idxFile(params.pathLabelIdx);
        size_t numLabels = 0;
        while (std::getline(label2idxFile, line)) {
            ++numLabels;
        }
        label2idxFile.close();

        // read csv
        std::map<std::string, size_t> y2domain;
        std::ifstream testY(params.pathTestY);

        while (std::getline(testY, line)) {
            auto li = support::splitLine(line, ',');
            auto dom = support::splitLine(li[1], '_');
            y2domain[li[0]] = domain2idx[dom[0]];
        }
        testY.close();

        model::ASTCODAModel mod(params.pathModel, kernelSize, embDim, numFilters, numLabels, numLabels / numDomains,
                                params.lang, params.minLen, params.threshold);

        std::filesystem::path testFolder = params.pathTestX;
        std::ofstream outFile(params.outPath);
        for (auto const &fileEntry : std::filesystem::directory_iterator{testFolder}) {
            auto fname = fileEntry.path().filename().string();

            auto vec = mod.run(fileEntry.path().string(), y2domain[fname]);

            outFile << fname;
            for (auto &v : vec) {
                outFile << " " << v;
            }
            outFile << "\n";
        }
        outFile.close();

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    } catch (const std::string &s) {
        std::cerr << s << std::endl;
        return 1;
    }
}
