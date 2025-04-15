#ifndef MODEL_MODEL_H
#define MODEL_MODEL_H

#include <Eigen/Dense>
#include <support/Support/Support.h>
#include <support/TreeSitter/TreeSitter.h>
#include <filesystem>
#include <fstream>

namespace model
{

Eigen::MatrixXf loadMatrix(const std::filesystem::path &filePath, size_t rows, size_t cols);

std::unordered_map<std::string, Eigen::VectorXf> loadEmbeddings(const std::filesystem::path &filename);

class ASTCODAModel
{

    size_t kernelSize;
    size_t embDim;
    size_t numFilters;
    size_t numLabels;
    size_t numClasses;
    std::string lang;
    size_t minLen;
    float threshold;
    size_t paddingIdx;
    size_t numDomains;

    std::string modelPath;

    // Vocabulary containing embeddings
    // Each embedding: [embDim]
    std::unordered_map<std::string, Eigen::VectorXf> embeddings;

    // Matrix that contains vectors representing domains. Used to compute attention weights
    // [numDomains, numFilters]
    Eigen::MatrixXf attentionDomains;

    // Batch Normalization
    // [num_filters]
    Eigen::MatrixXf batchNormAlpha;
    // [num_filters]
    Eigen::MatrixXf batchNormBeta;
    // [num_filters]
    Eigen::MatrixXf batchNormMean;
    // [num_filters]
    Eigen::MatrixXf batchNormVar;

    // Convolution layer
    // [num_filters, emb_dim * kernel_size]
    Eigen::MatrixXf convMatrix;
    // [num_filters]
    Eigen::MatrixXf convBias;

    // Fully-connected layer for classification
    // [num_classes * num_domains, num_filters]
    Eigen::MatrixXf fcMatrix;
    // [num_classes * num_domains, 1]
    Eigen::MatrixXf fcBias;

  public:
    ASTCODAModel(const std::string &modelPath, size_t kernelSize, size_t embDim, size_t numFilters, size_t numLabels,
                 size_t numClasses, const std::string &lang, size_t minLen, float threshold, size_t paddingIdx = 0);

    /// Function that processes one submission
    /// @param filePath - path to the submission
    /// @param domainIdx - domain which the submission belongs to
    std::set<size_t> run(const std::string &filePath, size_t domainIdx);
};

} // namespace model

#endif
