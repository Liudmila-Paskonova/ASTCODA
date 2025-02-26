#include <support/Support/Support.h>

std::vector<std::filesystem::path>
support::getNRandomFiles(const std::filesystem::path &dir, size_t n)
{
    std::vector<std::filesystem::path> files;
    for (const auto &sub : std::filesystem::directory_iterator(dir)) {
        if (sub.is_regular_file()) {
            files.push_back(sub.path());
        }
    }
    if (n >= files.size()) {
        return files;
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(files.begin(), files.end(), g);
    return std::vector<std::filesystem::path>(files.begin(), files.begin() + n);
}

std::vector<size_t>
support::trainTestValidSplit(size_t trainNumber, size_t validNumber, size_t testNumber)
{
    auto n = trainNumber + testNumber + validNumber;
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<size_t> indices(n);
    for (size_t i = 0; i < n; ++i) {
        indices[i] = i;
    }
    std::vector<size_t> classes(n);

    std::shuffle(indices.begin(), indices.end(), gen);

    for (size_t i = 0; i < trainNumber; ++i) {
        classes[indices[i]] = 0;
    }

    for (size_t i = trainNumber; i < trainNumber + validNumber; ++i) {
        classes[indices[i]] = 1;
    }

    for (size_t i = trainNumber + validNumber; i < n; ++i) {
        classes[indices[i]] = 2;
    }

    return classes;
}
