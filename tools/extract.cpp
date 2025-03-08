#include <extractor/Extractor.h>
#include <support/ArgParser/ArgParser.h>

struct Parameters : public argparser::Arguments {
    size_t numThreads;
    size_t minLen;
    size_t maxSize;
    std::string mapping;
    std::string lang;
    std::string dir;
    std::string traversal;
    std::string token;
    std::string split;
    std::string outdir;

    Parameters()
    {
        using namespace argparser;

        addParam<"threads">(numThreads, NaturalRangeArgument<>({1, std::thread::hardware_concurrency()}));
        addParam<"minlen">(minLen, NaturalRangeArgument<>({1, INT_MAX}));
        addParam<"maxsize">(maxSize, NaturalRangeArgument<>({1, INT_MAX}));
        addParam<"lang">(lang, ConstrainedArgument<std::string>({"c", "cpp"}));
        addParam<"dir">(dir, DirectoryArgument<std::string>());
        addParam<"traversal">(traversal, ConstrainedArgument<std::string>({"root_terminal", "terminal_terminal"}));
        addParam<"token">(token, ConstrainedArgument<std::string>({"masked_identifiers"}));
        addParam<"split">(split, ConstrainedArgument<std::string>({"ids_hash"}));
        addParam<"outdir">(outdir, DirectoryArgument<std::string>(false));
        addParam<"mapping">(mapping, FileArgument<std::string>());
    }
};

int
main(int argc, char *argv[])
{

    try {
        Parameters params;
        params.fromJSON(argv[1]);
        extractor::Extractor e;
        e.run(params);

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    } catch (const std::string &s) {
        std::cerr << s << std::endl;
        return 1;
    }

    return 0;
}
