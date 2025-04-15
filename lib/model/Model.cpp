#include <model/Model.h>

Eigen::MatrixXf
model::loadMatrix(const std::filesystem::path &filePath, size_t rows, size_t cols)
{
    std::ifstream file(filePath, std::ios::binary);

    Eigen::MatrixXf matrix(rows, cols);
    file.read(reinterpret_cast<char *>(matrix.data()), rows * cols * sizeof(float));

    if (file.gcount() != rows * cols * sizeof(float)) {
        throw std::runtime_error("File size doesn't match expected tensor dimensions!");
    }

    return matrix;
}

std::unordered_map<std::string, Eigen::VectorXf>
model::loadEmbeddings(const std::filesystem::path &filename)
{
    std::unordered_map<std::string, Eigen::VectorXf> embeddings;

    std::ifstream file(filename, std::ios::binary);

    std::string header;
    if (!std::getline(file, header)) {
        throw std::runtime_error("Failed to read header!");
    }

    std::istringstream header_stream(header);
    size_t vocab_size, dim;
    if (!(header_stream >> vocab_size >> dim)) {
        throw std::runtime_error("Invalid header format!");
    }

    for (size_t i = 0; i < vocab_size; ++i) {
        std::string word;
        char c;
        while (file.get(c) && c != ' ') {
            if (c == '\n') {
                file.putback(c);
                break;
            }
            word.push_back(c);
        }

        if (file.eof() || file.fail()) {
            throw std::runtime_error("Error reading word at position " + std::to_string(i));
        }

        std::vector<float> vec(dim);
        file.read(reinterpret_cast<char *>(vec.data()), dim * sizeof(float));

        if (file.gcount() != static_cast<std::streamsize>(dim * sizeof(float))) {
            throw std::runtime_error("Failed to read vector data for word: " + word);
        }

        embeddings[word] = Eigen::Map<Eigen::VectorXf>(vec.data(), dim);
    }

    return embeddings;
}

model::ASTCODAModel::ASTCODAModel(const std::string &modelPath, size_t kernelSize, size_t embDim, size_t numFilters,
                                  size_t numLabels, size_t numClasses, const std::string &lang, size_t minLen,
                                  float threshold, size_t paddingIdx)
    : modelPath(modelPath), kernelSize(kernelSize), embDim(embDim), numFilters(numFilters), numLabels(numLabels),
      numClasses(numClasses), lang(lang), minLen(minLen), threshold(threshold), paddingIdx(paddingIdx)
{
    numDomains = numLabels / numClasses;

    std::filesystem::path weightsFolder = modelPath;

    // load weights
    embeddings = loadEmbeddings(weightsFolder / "embeddings.bin");
    attentionDomains = loadMatrix(weightsFolder / "attention_domains.bin", numDomains, numFilters);
    batchNormAlpha = loadMatrix(weightsFolder / "bn_alpha.bin", numFilters, 1);
    batchNormBeta = loadMatrix(weightsFolder / "bn_beta.bin", numFilters, 1);
    batchNormMean = loadMatrix(weightsFolder / "bn_mean.bin", numFilters, 1);
    batchNormVar = loadMatrix(weightsFolder / "bn_var.bin", numFilters, 1);
    convMatrix = loadMatrix(weightsFolder / "conv_matrix.bin", numFilters, embDim * kernelSize);
    convBias = loadMatrix(weightsFolder / "conv_bias.bin", numFilters, 1);
    fcMatrix = loadMatrix(weightsFolder / "fc_matrices.bin", numClasses * numDomains, numFilters);
    fcBias = loadMatrix(weightsFolder / "fc_biases.bin", numClasses * numDomains, 1);
}

std::set<size_t>
model::ASTCODAModel::run(const std::string &filePath, size_t domainIdx)
{
    treesitter::Tree t(filePath, lang);
    auto tokens = t.process("root_terminal", "masked_identifiers", "ids_hash", minLen);
    auto positions = t.positions;

    Eigen::VectorXf zeroVec(embDim);
    zeroVec.setZero();

    // Vector that stores concatenated embeddings for each token in the padded token sequence
    Eigen::VectorXf allEmb(embDim * (tokens.size() + 2 * kernelSize - 2));

    // Get embeddings and pad the submission
    size_t idx = 0;
    for (size_t i = 0; i < kernelSize - 1; ++i) {
        allEmb.segment(idx, embDim) = zeroVec;
        idx += embDim;
    }

    for (auto &v : tokens) {
        // get a pretrained embedding if exists, otherwise initialize it with "UNK"
        if (embeddings.contains(v)) {
            allEmb.segment(idx, embDim) = embeddings[v];
        } else {
            allEmb.segment(idx, embDim) = embeddings["@@UNK@@"];
        }
        idx += embDim;
    }

    for (size_t i = 0; i < kernelSize - 1; ++i) {
        allEmb.segment(idx, embDim) = zeroVec;
        idx += embDim;
    }

    // Apply convolution layer
    auto numConvs = tokens.size() + kernelSize - 1;
    std::vector<Eigen::VectorXf> features;
    for (auto i = 0; i < numConvs; ++i) {
        // [embDim * kernelSize]
        auto a = allEmb.segment(i * embDim, embDim * kernelSize);
        // [numFilters]
        auto v = convMatrix * a + convBias;
        features.push_back(v);
    }

    // Batch Normalization
    float eps = 1e-5;
    for (auto &feat : features) {
        auto p = (feat.array() - batchNormMean.array()) / (batchNormVar.array() + eps).sqrt();
        // [numFilters]
        feat = p.array() * batchNormAlpha.array() + batchNormBeta.array();
    }

    // ReLU
    for (auto &feat : features) {
        auto p = feat.array().max(0);
        // [numFilters]
        feat = p;
    }

    // Get the domain vector
    // [numFilters]
    auto dom = attentionDomains.row(domainIdx);

    Eigen::VectorXf attentionWeights(numConvs);
    attentionWeights.setZero();

    float sum = 0;

    // Compute the dot product between the domain vector and the feature and store it as a vector
    // [numConvs]
    size_t ind = 0;
    for (auto &feat : features) {
        attentionWeights(ind++) = feat.dot(dom);
    }

    auto maxVal = attentionWeights.maxCoeff();
    auto minVal = attentionWeights.minCoeff();

    Eigen::VectorXf attentionWeightsNorm = -1 + (attentionWeights.array() - minVal) * 2 / (maxVal - minVal);
    std::vector<int> indices;

    for (size_t i = kernelSize - 1; i < attentionWeightsNorm.size() - kernelSize + 1; ++i) {
        if (attentionWeightsNorm[i] >= threshold) {
            indices.push_back(i - kernelSize + 1);
        }
    }

    // Softmax layer
    Eigen::VectorXf expOne = attentionWeights.array().exp();
    float sumExp = expOne.sum();
    // [numConvs]
    attentionWeights = expOne / sumExp;

    // Final representation
    // [numFilters]
    Eigen::VectorXf result(numFilters);
    result.setZero();

    ind = 0;
    for (auto &feat : features) {
        auto p = feat.array() * attentionWeights(ind++);
        result += p.matrix();
    }

    // Logits
    // [numClasses]
    Eigen::VectorXf logits = fcMatrix.block(numClasses * domainIdx, 0, numClasses, numFilters) * result +
                             fcBias.block(numClasses * domainIdx, 0, numClasses, 1);

    std::set<size_t> resultLines;
    if (logits(0) < logits(1)) {
        for (auto &ind : indices) {
            resultLines.insert(positions[ind]);
        }
    } else {
        resultLines.insert(0);
    }
    return resultLines;
}
