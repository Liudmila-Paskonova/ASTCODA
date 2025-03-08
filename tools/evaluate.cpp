#include <model/Model.h>
#include <iostream>
#include <support/ArgParser/ArgParser.h>

struct Parameters : public argparser::Arguments {
    size_t embDim;
    size_t kernelSize;
    size_t numFilters;
    std::vector<std::string> domainClasses;
    std::string pathDomainIdx;
    std::string pathTestX;
    std::string pathTestSub;
    std::string pathModel;

    Parameters()
    {
        using namespace argparser;

        addParam<"embedding_dim">(embDim, NaturalRangeArgument<>({1, INT_MAX}));
        addParam<"kernel_size">(kernelSize, NaturalRangeArgument<>({1, INT_MAX}));
        addParam<"num_filters">(numFilters, NaturalRangeArgument<>({1, INT_MAX}));
        addParam<"domain_classes">(domainClasses, UnconstrainedArgument<std::vector<std::string>>());
        addParam<"domain_to_idx">(pathDomainIdx, FileArgument<std::string>());
        addParam<"test_x">(pathTestX, FileArgument<std::string>());
        addParam<"test_sub">(pathTestSub, FileArgument<std::string>());
        addParam<"model_path">(pathModel, FileArgument<std::string>());
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
        size_t numClasses = params.domainClasses.size();

        std::ifstream domain2idx(params.pathDomainIdx);
        std::string line;
        size_t numDomains = 0;
        while (std::getline(domain2idx, line)) {
            ++numDomains;
        }
        domain2idx.close();

        std::ifstream f(params.pathTestSub);
        std::string sub;

        model::CNNModel mod(params.pathModel, kernelSize, embDim, numFilters, numClasses * numDomains, numClasses, "c",
                            0, 0.8);

        while (std::getline(f, sub)) {
            auto filePath = params.pathTestX + sub;
            auto vec = mod.run(filePath, 0);
            std::cout << sub;
            for (auto &v : vec) {
                std::cout << " " << v;
            }
            std::cout << std::endl;
        }
        f.close();

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    } catch (const std::string &s) {
        std::cerr << s << std::endl;
        return 1;
    }
}
