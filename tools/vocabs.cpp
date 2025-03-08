#include <support/ArgParser/ArgParser.h>
#include <vocab/Vocabulary.h>
#include <iostream>

struct Parameters : public argparser::Arguments {
    std::string pathTrainX;
    std::string pathValidX;
    std::string pathTestX;
    std::string pathTrainY;
    std::string pathValidY;
    std::string pathTestY;
    std::string pathTrainVocab;
    std::string pathValidVocab;
    std::string pathTestVocab;
    std::string pathTokenIdx;
    std::string pathLabelIdx;
    std::string pathDomainIdx;
    std::string pathTerminalHash;
    std::vector<std::string> domainClasses;

    Parameters()
    {
        using namespace argparser;

        addParam<"train_x">(pathTrainX, FileArgument<std::string>());
        addParam<"train_y">(pathTrainY, FileArgument<std::string>());
        addParam<"train_vocab">(pathTrainVocab, FileArgument<std::string>());
        addParam<"test_x">(pathTestX, FileArgument<std::string>());
        addParam<"test_y">(pathTestY, FileArgument<std::string>());
        addParam<"test_vocab">(pathTestVocab, FileArgument<std::string>());
        addParam<"valid_x">(pathValidX, FileArgument<std::string>());
        addParam<"valid_y">(pathValidY, FileArgument<std::string>());
        addParam<"valid_vocab">(pathValidVocab, FileArgument<std::string>());
        addParam<"domain_classes">(domainClasses, UnconstrainedArgument<std::vector<std::string>>());
        addParam<"token_to_idx">(pathTokenIdx, FileArgument<std::string>(false));
        addParam<"label_to_idx">(pathLabelIdx, FileArgument<std::string>(false));
        addParam<"domain_to_idx">(pathDomainIdx, FileArgument<std::string>(false));
        addParam<"hash_to_terminal">(pathTerminalHash, FileArgument<std::string>(false));
    }
};

int
main(int argc, char *argv[])
{

    try {
        Parameters params;
        params.fromJSON(argv[1]);

        vocabulary::Vocabulary vocab(params.domainClasses);

        vocab.addTokens(params.pathTrainX);
        vocab.addTokens(params.pathValidX);
        vocab.addTokens(params.pathTestX);

        vocab.addLabels(params.pathTrainY);
        vocab.addLabels(params.pathValidY);
        vocab.addLabels(params.pathTestY);

        vocab.addTerminals(params.pathTrainVocab);
        vocab.addTerminals(params.pathValidVocab);
        vocab.addTerminals(params.pathTestVocab);

        vocab.print(params.pathTokenIdx, params.pathLabelIdx, params.pathDomainIdx, params.pathTerminalHash);

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    } catch (const std::string &s) {
        std::cerr << s << std::endl;
        return 1;
    }
}
