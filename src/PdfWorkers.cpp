#include "PdfWorkers.h"
#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>

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
        for (const auto& img_path : imagePaths) {
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
        }
        document.Write(outputPath.c_str());
        return true;
    } catch (PoDoFo::PdfError& e) {
        errorMsg = "PoDoFo error occurred."; // could fetch more detail if PoDoFo < 0.10
        return false;
    } catch (const std::exception& e) {
        errorMsg = e.what();
        return false;
    }
}

bool mergePdfs(const std::vector<std::string>& inputPaths, const std::string& outputPath, std::string& errorMsg) {
    try {
        PoDoFo::PdfMemDocument output_document;
        for (const auto& pdf_path : inputPaths) {
            PoDoFo::PdfMemDocument input_document;
            input_document.Load(pdf_path.c_str());
            output_document.Append(input_document);
        }
        output_document.Write(outputPath.c_str());
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
        document.Load(inputPath.c_str());
        int total_pages = document.GetPageCount();

        std::set<int> target_pages = parse_pages(page_spec, total_pages);
        int pdf_rotation = normalize_rotation(input_angle);

        for (int page_num : target_pages) {
            PoDoFo::PdfPage* page = document.GetPage(page_num - 1);
            int current_rot = page->GetRotation();
            page->SetRotation((current_rot + pdf_rotation) % 360);
        }

        document.Write(outputPath.c_str());
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

} // namespace PdfWorkers
