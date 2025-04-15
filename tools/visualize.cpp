#include <visualizer/Visualizer.h>
#include <iostream>
#include <support/ArgParser/ArgParser.h>
#include <string>

struct Parameters : public argparser::Arguments {
    std::string pathTestX;
    std::string pathTestY;
    std::string outPath;

    Parameters()
    {
        using namespace argparser;

        addParam<"test_x">(pathTestX, DirectoryArgument<std::string>());
        addParam<"chosen_lines">(outPath, FileArgument<std::string>());
        addParam<"test_y">(pathTestY, FileArgument<std::string>());
    }
};

int
main(int argc, char *argv[])
{
    try {
        Parameters params;
        params.fromJSON(argv[1]);

        QApplication app(argc, argv);

        QString folderPath = params.pathTestX.c_str();
        QString linePath = params.outPath.c_str();
        QString labelsPath = params.pathTestY.c_str();

        visualizer::CodeViewer viewer(folderPath, linePath, labelsPath);
        viewer.resize(800, 600);
        viewer.show();

        return app.exec();
    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    } catch (const std::string &s) {
        std::cerr << s << std::endl;
        return 1;
    }
}
