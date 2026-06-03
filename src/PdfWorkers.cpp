#include "PdfWorkers.h"
#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>

#include <QImage>
#include <QProcess>
#include <QTemporaryDir>
#include <QDir>
#include <QStringList>
#include <QRegularExpression>

// podofo headers
#include <podofo/podofo.h>

// qpdf headers
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFWriter.hh>

namespace PdfWorkers {

bool convertImagesToPdf(const std::vector<std::string>& imagePaths, const std::string& outputPath, std::string& errorMsg) {
    try {
        PoDoFo::PdfMemDocument document;
#if PODOFO_VERSION_MINOR >= 10
        PoDoFo::PdfPainter painter;
#endif
        for (const auto& img_path : imagePaths) {
#if PODOFO_VERSION_MINOR >= 10
            auto image = document.CreateImage();
            image->Load(img_path);
            double img_w = image->GetWidth();
            double img_h = image->GetHeight();
            auto& page = document.GetPages().CreatePage(PoDoFo::Rect(0, 0, img_w, img_h));
            painter.SetCanvas(page);
            painter.DrawImage(*image, 0, 0, 1.0, 1.0);
            painter.FinishDrawing();
#else
            PoDoFo::PdfImage image(&document);
            image.LoadFromFile(img_path.c_str());

            double img_w = image.GetWidth();
            double img_h = image.GetHeight();

            PoDoFo::PdfRect page_rect(0, 0, img_w, img_h);
            PoDoFo::PdfPage* page = document.CreatePage(page_rect);

            PoDoFo::PdfPainter painter;
            painter.SetPage(page);
            painter.DrawImage(0, 0, &image, 1.0, 1.0);
            painter.FinishPage();
#endif
        }
#if PODOFO_VERSION_MINOR >= 10
        document.Save(outputPath);
#else
        document.Write(outputPath.c_str());
#endif
        return true;
#if PODOFO_VERSION_MINOR >= 10
    } catch (PoDoFo::PdfError& e) {
        errorMsg = "PoDoFo error occurred.";
        return false;
    } catch (const std::exception& e) {
        errorMsg = e.what();
        return false;
    }
#else
    } catch (PoDoFo::PdfError& e) {
        errorMsg = "PoDoFo error occurred.";
        return false;
    } catch (const std::exception& e) {
        errorMsg = e.what();
        return false;
    }
#endif
}

bool mergePdfs(const std::vector<std::string>& inputPaths, const std::string& outputPath, std::string& errorMsg) {
    try {
        PoDoFo::PdfMemDocument output_document;
#if PODOFO_VERSION_MINOR >= 10
        bool first = true;
#endif
        for (const auto& pdf_path : inputPaths) {
            PoDoFo::PdfMemDocument input_document;
#if PODOFO_VERSION_MINOR >= 10
            input_document.Load(pdf_path);
            if (first) {
                output_document.GetPages().AppendDocumentPages(input_document);
                first = false;
            } else {
                output_document.GetPages().AppendDocumentPages(input_document);
            }
#else
            input_document.Load(pdf_path.c_str());
            output_document.Append(input_document);
#endif
        }
#if PODOFO_VERSION_MINOR >= 10
        output_document.Save(outputPath);
#else
        output_document.Write(outputPath.c_str());
#endif
        return true;
    } catch (PoDoFo::PdfError& e) {
        errorMsg = "PoDoFo error occurred in merge.";
        return false;
    } catch (const std::exception& e) {
        errorMsg = e.what();
        return false;
    }
}

int normalize_rotation(int angle) {
    int clockwise_angle = (angle % 360 + 360) % 360;
    return ((clockwise_angle + 45) / 90 * 90) % 360;
}

std::set<int> parse_pages(const std::string& page_spec, int total_pages) {
    std::set<int> target_pages;
    if (page_spec == "all") {
        for (int i = 1; i <= total_pages; ++i) target_pages.insert(i);
        return target_pages;
    }
    std::stringstream ss(page_spec);
    std::string item;
    while (std::getline(ss, item, ',')) {
        size_t dash_pos = item.find('-');
        if (dash_pos != std::string::npos) {
            int start = std::stoi(item.substr(0, dash_pos));
            int end = std::stoi(item.substr(dash_pos + 1));
            for (int p = start; p <= end; ++p) {
                if (p >= 1 && p <= total_pages) target_pages.insert(p);
            }
        } else {
            int p = std::stoi(item);
            if (p >= 1 && p <= total_pages) target_pages.insert(p);
        }
    }
    return target_pages;
}

bool rotatePdfPages(const std::string& inputPath, const std::string& outputPath, const std::string& command, std::string& errorMsg) {
    try {
        size_t colon_pos = command.find(':');
        if (colon_pos == std::string::npos) {
            errorMsg = "Invalid command format. Expected page_spec:angle";
            return false;
        }
        std::string page_spec = command.substr(0, colon_pos);
        int input_angle = std::stoi(command.substr(colon_pos + 1));

        PoDoFo::PdfMemDocument document;
#if PODOFO_VERSION_MINOR >= 10
        document.Load(inputPath);
        int total_pages = document.GetPages().GetCount();
#else
        document.Load(inputPath.c_str());
        int total_pages = document.GetPageCount();
#endif

        std::set<int> target_pages = parse_pages(page_spec, total_pages);
        int pdf_rotation = normalize_rotation(input_angle);

        for (int page_num : target_pages) {
#if PODOFO_VERSION_MINOR >= 10
            PoDoFo::PdfPage& page = document.GetPages().GetPageAt(page_num - 1);
            int current_rot = page.GetRotationRaw();
            page.SetRotationRaw((current_rot + pdf_rotation) % 360);
#else
            PoDoFo::PdfPage* page = document.GetPage(page_num - 1);
            int current_rot = page->GetRotation();
            page->SetRotation((current_rot + pdf_rotation) % 360);
#endif
        }

#if PODOFO_VERSION_MINOR >= 10
        document.Save(outputPath);
#else
        document.Write(outputPath.c_str());
#endif
        return true;
    } catch (PoDoFo::PdfError& e) {
        errorMsg = "PoDoFo error in rotate.";
        return false;
    } catch (const std::exception& e) {
        errorMsg = e.what();
        return false;
    }
}

bool createSpreadPdf(const std::string& inputPath, const std::string& outputPath, std::string& errorMsg) {
    try {
        QPDF qpdf;
        qpdf.processFile(inputPath.c_str());

        QPDFPageDocumentHelper dh(qpdf);
        std::vector<QPDFPageObjectHelper> pages = dh.getAllPages();
        size_t num_pages = pages.size();

        QPDF out_qpdf;
        out_qpdf.emptyPDF();
        QPDFPageDocumentHelper out_dh(out_qpdf);

        for (size_t i = 0; i < num_pages; i += 2) {
            QPDFPageObjectHelper left_page = pages[i];
            bool has_right = (i + 1 < num_pages);

            QPDFObjectHandle left_mb = left_page.getMediaBox();
            double left_w = left_mb.getArrayItem(2).getNumericValue();
            double left_h = left_mb.getArrayItem(3).getNumericValue();

            double right_w = left_w;
            double right_h = left_h;
            if (has_right) {
                QPDFObjectHandle right_mb = pages[i + 1].getMediaBox();
                right_w = right_mb.getArrayItem(2).getNumericValue();
                right_h = right_mb.getArrayItem(3).getNumericValue();
            }

            double left_scale_w = 1.0;
            double left_scale_h = 1.0;
            double right_scale_w = 1.0;
            double right_scale_h = 1.0;

            if (has_right) {
                if (left_w > right_w) {
                    left_scale_w = right_w / left_w;
                    left_w = right_w;
                } else if (right_w > left_w) {
                    right_scale_w = left_w / right_w;
                    right_w = left_w;
                }
                if (left_h > right_h) {
                    left_scale_h = right_h / left_h;
                    left_h = right_h;
                } else if (right_h > left_h) {
                    right_scale_h = left_h / right_h;
                    right_h = left_h;
                }
            }

            double spread_w = left_w + right_w;
            double spread_h = std::max(left_h, right_h);

            QPDFObjectHandle spread_page_obj = QPDFObjectHandle::newDictionary();
            spread_page_obj.replaceKey("/Type", QPDFObjectHandle::newName("/Page"));
            
            QPDFObjectHandle new_mb = QPDFObjectHandle::newArray();
            new_mb.appendItem(QPDFObjectHandle::newReal(0.0));
            new_mb.appendItem(QPDFObjectHandle::newReal(0.0));
            new_mb.appendItem(QPDFObjectHandle::newReal(spread_w));
            new_mb.appendItem(QPDFObjectHandle::newReal(spread_h));
            spread_page_obj.replaceKey("/MediaBox", new_mb);

            QPDFObjectHandle contents_array = QPDFObjectHandle::newArray();
            QPDFObjectHandle xobject_dict = QPDFObjectHandle::newDictionary();

            QPDFObjectHandle left_xo = out_qpdf.copyForeignObject(left_page.getFormXObjectForPage());
            std::string left_name = "/LeX";
            xobject_dict.replaceKey(left_name, left_xo);
            std::string left_cmd = "q " + std::to_string(left_scale_w) + " 0 0 " + 
                                   std::to_string(left_scale_h) + " 0 0 cm " + left_name + " Do Q\n";
            contents_array.appendItem(QPDFObjectHandle::newStream(&out_qpdf, left_cmd));

            if (has_right) {
                QPDFObjectHandle right_xo = out_qpdf.copyForeignObject(pages[i + 1].getFormXObjectForPage());
                std::string right_name = "/RiX";
                xobject_dict.replaceKey(right_name, right_xo);
                std::string right_cmd = "q " + std::to_string(right_scale_w) + " 0 0 " + 
                                        std::to_string(right_scale_h) + " " + std::to_string(left_w) + " 0 cm " + 
                                        right_name + " Do Q\n";
                contents_array.appendItem(QPDFObjectHandle::newStream(&out_qpdf, right_cmd));
            }

            QPDFObjectHandle res_dict = QPDFObjectHandle::newDictionary();
            res_dict.replaceKey("/XObject", xobject_dict);
            spread_page_obj.replaceKey("/Resources", res_dict);
            spread_page_obj.replaceKey("/Contents", contents_array);

            out_dh.addPage(QPDFPageObjectHelper(spread_page_obj), false);
        }

        QPDFWriter writer(out_qpdf, outputPath.c_str());
        writer.write();
        return true;
    } catch (const std::exception& e) {
        errorMsg = e.what();
        return false;
    }
}

bool makeWhiteBackground(const std::string& inputPath, const std::string& outputPath, std::string& errorMsg) {
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        errorMsg = "Could not create temporary directory.";
        return false;
    }

    // 1. Convert PDF to PNG images using pdftoppm
    QProcess process;
    QString prefix = tempDir.path() + "/page";
    process.start("pdftoppm", QStringList() << "-png" << "-r" << "300" << QString::fromStdString(inputPath) << prefix);
    process.waitForFinished(-1);

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        errorMsg = "pdftoppm failed to convert PDF to images.";
        return false;
    }

    // 2. Read and process generated PNG files
    QDir dir(tempDir.path());
    QStringList filters;
    filters << "page-*.png";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();

    if (fileList.isEmpty()) {
        errorMsg = "No pages found or converted.";
        return false;
    }

    std::sort(fileList.begin(), fileList.end(), [](const QFileInfo& a, const QFileInfo& b) {
        QRegularExpression re("page-(\\d+)\\.png");
        auto matchA = re.match(a.fileName());
        auto matchB = re.match(b.fileName());
        int numA = matchA.hasMatch() ? matchA.captured(1).toInt() : 0;
        int numB = matchB.hasMatch() ? matchB.captured(1).toInt() : 0;
        return numA < numB;
    });

    std::vector<std::string> processedImages;

    for (const QFileInfo& fileInfo : fileList) {
        QImage img(fileInfo.absoluteFilePath());
        if (img.isNull()) continue;

        img = img.convertToFormat(QImage::Format_RGB888);
        for (int y = 0; y < img.height(); ++y) {
            uchar* line = img.scanLine(y);
            for (int x = 0; x < img.width(); ++x) {
                int r = line[x * 3];
                int g = line[x * 3 + 1];
                int b = line[x * 3 + 2];
                if (r > 150 && g > 150 && b > 180) {
                    line[x * 3] = 255;
                    line[x * 3 + 1] = 255;
                    line[x * 3 + 2] = 255;
                }
            }
        }
        
        QString outImgPath = fileInfo.absoluteFilePath() + "_proc.jpg"; // Use jpeg for smaller size like in python script
        img.save(outImgPath, "JPEG", 100);
        processedImages.push_back(outImgPath.toStdString());
    }

    // 3. Convert back to PDF
    return convertImagesToPdf(processedImages, outputPath, errorMsg);
}

} // namespace PdfWorkers
