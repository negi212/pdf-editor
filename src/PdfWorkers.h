#pragma once

#include <vector>
#include <string>

namespace PdfWorkers {
    bool convertImagesToPdf(const std::vector<std::string>& imagePaths, const std::string& outputPath, std::string& errorMsg);
    bool mergePdfs(const std::vector<std::string>& inputPaths, const std::string& outputPath, std::string& errorMsg);
    bool rotatePdfPages(const std::string& inputPath, const std::string& outputPath, const std::string& command, std::string& errorMsg);
    bool createSpreadPdf(const std::string& inputPath, const std::string& outputPath, std::string& errorMsg);
    bool makeWhiteBackground(const std::string& inputPath, const std::string& outputPath, std::string& errorMsg);
}
