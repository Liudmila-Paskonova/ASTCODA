#ifndef SUPPORT_ARGPARSER_ARGPARSER_H
#define SUPPORT_ARGPARSER_ARGPARSER_H

#include <string>
#include <set>
#include <type_traits>
#include <sstream>
#include <any>
#include <format>
#include <filesystem>
#include <map>
#include <memory>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>

namespace argparser
{
using json = nlohmann::json;

template <typename T>
concept IsVector = requires {
    typename T::value_type;
    typename T::allocator_type;
} && std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

/// Abstract class representing one Argument type
struct Argument {

    virtual ~Argument() {};

    /// A function that updates value with a new param value.
    /// @param value - a reference (represented as std::any) to some object.
    /// @param param - a command line argument to convert
    virtual void setValue(std::any &value, const std::string &param) = 0;

    /// A function that tries to convert a given command line argument param into a known type T.
    /// @tparam T - a type
    /// @param value - a reference (represented as std::any) to T object.
    /// @param param - a command line argument to convert
    /// @return the expected T value, @exception if impossible
    template <typename T>
    decltype(auto)
    getValue(std::any &value, const std::string &param)
    {
        auto concreteValue = std::any_cast<T *>(&value);
        if constexpr (IsVector<T>) {
            (**concreteValue).clear();

            using ElementType = typename T::value_type;
            std::istringstream istream(param);
            std::string token;
            while (istream >> token) {
                std::istringstream tokenStream(token);
                ElementType elem;
                if (!(tokenStream >> elem)) {
                    throw std::format("Unable to parse token: {}!", token);
                }
                (**concreteValue).push_back(elem);
            }
        } else {
            std::istringstream istream(param);
            if (!(istream >> **concreteValue)) {
                throw std::format("Unable to parse the string: {}!", param);
            }
        }
        return **concreteValue;
    };
};
/// Class to represent Directory argument
/// @tparam T - a concrete type (only std::string is allowed)
template <typename T = std::string>
    requires(std::same_as<T, std::string>)
class DirectoryArgument : public Argument
{
    bool readOnly;

  public:
    DirectoryArgument(bool readOnly = true) : readOnly(readOnly) {}

    /// @brief Check if a provided value is a directory
    /// @param value - a value to check
    void
    checkValue(const T &value)
    {
        if (readOnly) {
            if (!std::filesystem::is_directory(value) || !std::filesystem::exists(value)) {
                throw std::format("{} is not a directory!", value);
            }
        } else {
            if (std::filesystem::exists(value) && !std::filesystem::is_directory(value)) {
                throw std::format("{} is not a directory!", value);
            } else if (!std::filesystem::exists(value)) {
                std::filesystem::create_directory(value);
            }
        }
    }

    void
    setValue(std::any &value, const std::string &param) override
    {
        auto concreteValue = getValue<T>(value, param);
        checkValue(concreteValue);
    }
};

/// Class to represent File argument
/// @tparam T - a concrete type (only std::string is allowed)
template <typename T = std::string>
    requires(std::same_as<T, std::string>)
class FileArgument : public Argument
{
    bool readOnly;

  public:
    FileArgument(bool readOnly = true) : readOnly(readOnly) {}

    /// @brief Check if a provided value is a file
    /// @param value - a value to check
    void
    checkValue(const T &value)
    {
        if (readOnly) {
            if (!std::filesystem::is_regular_file(value) || !std::filesystem::exists(value)) {
                throw std::format("{} is not a file!", value);
            }
        }
    }

    void
    setValue(std::any &value, const std::string &param) override
    {
        auto concreteValue = getValue<T>(value, param);
        checkValue(concreteValue);
    }
};

/// Class to represent arguments that can be restricted with some range (number of threads, length etc.)
/// @tparam T - a concrete type (only unsigned integral types are allowed)
template <typename T = size_t>
    requires(std::unsigned_integral<T>)
class NaturalRangeArgument : public Argument
{
    T minValue;
    T maxValue;

  public:
    NaturalRangeArgument(const std::pair<T, T> &rangeBorders = {0, std::numeric_limits<T>::max()})
        : minValue(rangeBorders.first), maxValue(rangeBorders.second)
    {
    }

    /// @brief Check if a provided value lies in the predefined range
    /// @param value - a value to check
    void
    checkValue(const T &value)
    {
        if (value < minValue || value > maxValue) {
            throw std::format("{} is out of the range [{}, {}]!", value, minValue, maxValue);
        }
    }

    void
    setValue(std::any &value, const std::string &param) override
    {
        auto concreteValue = getValue<T>(value, param);
        checkValue(concreteValue);
    }
};

/// Class to represent arguments from the predefined container (language, options etc.)
/// @tparam T - a concrete type (only arithmetic types and std::string are allowed)
template <typename T = bool>
    requires(std::is_arithmetic<T>() == true || std::same_as<T, std::string>)
class ConstrainedArgument : public Argument
{
    std::set<T> container;

  public:
    ConstrainedArgument(const std::set<T> &cont = {false, true}) : container(cont) {}

    /// @brief Check if a provided value exists in the predefined container
    /// @param value - a value to check
    void
    checkValue(const T &value)
    {
        if (container.find(value) == container.end()) {
            std::stringstream allValues;
            bool flag = true;
            for (auto &c : container) {
                if (flag) {
                    allValues << c;
                    flag = false;
                } else {
                    allValues << ", " << c;
                }
            }

            throw std::format("There's no {} among allowed values: {{{}}}!", value, allValues.rdbuf()->str());
        }
    }

    void
    setValue(std::any &value, const std::string &param) override
    {
        auto concreteValue = getValue<T>(value, param);
        checkValue(concreteValue);
    }
};

template <typename T> class UnconstrainedArgument : public Argument
{
  public:
    UnconstrainedArgument() {}

    void
    setValue(std::any &value, const std::string &param) override
    {
        auto concreteValue = getValue<T>(value, param);
    }
};

/// A compile-time wrapper a short key, e.g. a key, beginning with one '-'
/// @tparam Len - length of the string
template <std::size_t Len> struct ShortArg {
    char argstr[Len]{};

    consteval ShortArg(const char (&str)[Len])
    {
        if (!str) {
            throw "Empty argument!";
        }

        if (strlen(str) <= 1) {
            throw "Short argument is too short";
        }

        std::copy_n(str, Len, argstr);
    }
};

/// Custom key for maps
class KeyParam
{
  public:
    std::string sharg;

    bool operator<(const KeyParam &k2) const;
};

/// Main class
class Arguments
{
    // map storing pointers to Arguments
    std::map<KeyParam, std::unique_ptr<Argument>> parameters;
    // map storing references to Parameters' values (which are in turn represented as std::any)
    std::map<KeyParam, std::any> values;

  protected:
    /// A function to register a new argument rule
    /// @tparam T - type of a parameter object
    /// @tparam sharg - short argument
    /// @tparam larg - long argument
    /// @param value - an object of the type @tparam T
    /// @param obj - concrete Argument type (e.g. DirectoryArgument, NaturalRangeArgument etc.)
    template <ShortArg sharg, typename T, template <typename> class Object>
        requires((std::is_arithmetic<T>() == true || std::same_as<T, std::string> || IsVector<T>) &&
                 (std::same_as<Object<T>, FileArgument<T>> || std::same_as<Object<T>, DirectoryArgument<T>> ||
                  std::same_as<Object<T>, NaturalRangeArgument<T>> || std::same_as<Object<T>, ConstrainedArgument<T>> ||
                  std::same_as<Object<T>, UnconstrainedArgument<T>>) )
    void
    addParam(T &value, const Object<T> &obj)
    {
        parameters[{sharg.argstr}] = std::make_unique<Object<T>>(obj);
        values[{sharg.argstr}] = &value;
    }

  public:
    /// Main function to parse command line arguments
    /// @param argc
    /// @param argv
    void parse(int argc, char *argv[]);

    /// Load arguments from JSON file
    /// @param pathJSON
    void fromJSON(const std::string &pathJSON);
};

template <bool> ConstrainedArgument() -> ConstrainedArgument<bool>;

}; // namespace argparser

#endif
