#include <support/ArgParser/ArgParser.h>

bool
argparser::KeyParam::operator<(const KeyParam &k2) const
{
    return this->sharg < k2.sharg;
}

void
argparser::Arguments::parse(int argc, char *argv[])
{
    for (int argIndex = 1; argIndex < argc; ++argIndex) {
        std::string arg{argv[argIndex]};
        std::string param;
        if (arg.starts_with("-")) {
            // arg is a key
            arg.erase(arg.begin());
            auto it = std::find_if(parameters.begin(), parameters.end(),
                                   [&arg](const auto &p) { return p.first.sharg == arg; });

            auto vit =
                std::find_if(values.begin(), values.end(), [&arg](const auto &p) { return p.first.sharg == arg; });

            if (it == parameters.end()) {
                throw "Wrong key!\n";
            }
            ++argIndex;

            if (argIndex == argc || std::string(argv[argIndex]).starts_with("-")) {
                // arg is a CostrainedArgument()
                it->second->setValue(vit->second, "1");
                --argIndex;
            } else {
                // value
                param = argv[argIndex];
                it->second->setValue(vit->second, param);
            }
        } else {
            // arg is not a key
            throw std::format("Argument {} is not a key", arg);
        }
    }
}

void
argparser::Arguments::fromJSON(const std::string &pathJSON)
{
    std::ifstream f(pathJSON);
    json tempJson = json::object();
    f >> tempJson;
    f.close();

    for (auto &[key, param] : parameters) {
        auto val = tempJson.find(key.sharg);
        if (val != tempJson.end()) {
            std::string arg = val.key();
            auto vit =
                std::find_if(values.begin(), values.end(), [&arg](const auto &p) { return p.first.sharg == arg; });
            auto value = val.value();
            std::string res;
            if (value.is_boolean()) {
                param->setValue(vit->second, std::to_string(int(value)));
            } else if (value.is_number()) {
                param->setValue(vit->second, value.dump());
            } else if (value.is_string()) {
                param->setValue(vit->second, value);
            } else if (value.is_array()) {
                std::string temp;
                for (auto &elem : value) {
                    if (elem.is_boolean()) {
                        temp += std::to_string(int(elem)) + " ";
                    } else if (elem.is_number()) {
                        temp += elem.dump() + " ";
                    } else if (elem.is_string()) {
                        temp += elem;
                        temp += " ";
                    }
                }
                temp.back() = '\0';
                param->setValue(vit->second, temp);
            }
        } else {
            throw std::format("There's no {} among keys in the given JSON {}!", key.sharg, pathJSON);
        }
    }
}
