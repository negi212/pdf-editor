#include "MainWindow.h"
#include "PdfWorkers.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("PDF Editor");

    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
            color: #333333;
        }
        QWidget {
            color: #333333;
        }
        QTabWidget::pane {
            border: 1px solid #dcdcdc;
            background: white;
            border-radius: 4px;
        }
        QTabBar::tab {
            background: #e0e0e0;
            color: #555555;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background: white;
            color: #111111;
            border: 1px solid #dcdcdc;
            border-bottom-color: white;
            font-weight: bold;
        }
        QPushButton {
            background-color: #0078d7;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #005a9e;
        }
        QPushButton:pressed {
            background-color: #004275;
        }
        QListWidget, QLineEdit {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 4px;
            background: white;
            color: #111111;
        }
        QLabel {
            font-size: 14px;
            color: #111111;
        }
        QStatusBar {
            background: white;
            color: #111111;
            border-top: 1px solid #dcdcdc;
        }
    )");

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    tabWidget->addTab(createImageToPdfTab(), "画像PDF変換");
    tabWidget->addTab(createMergePdfTab(), "PDF結合");
    tabWidget->addTab(createRotatePdfTab(), "ページ回転");
    tabWidget->addTab(createSpreadPdfTab(), "見開き化");

    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    showMessage("Ready");
}

MainWindow::~MainWindow() {}

void MainWindow::showMessage(const QString& msg, bool isError) {
    if (isError) {
        statusBar->setStyleSheet("color: red;");
    } else {
        statusBar->setStyleSheet("color: black;");
    }
    statusBar->showMessage(msg);
}

// ----- Tab 1: Image to PDF -----
QWidget* MainWindow::createImageToPdfTab() {
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(w);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    imgInputFileEdit = new QLineEdit();
    imgInputFileEdit->setReadOnly(true);
    imgInputFileEdit->installEventFilter(this);
    imgBrowseBtn = new QPushButton("参照...");
    imgClearBtn = new QPushButton("クリア");
    fileLayout->addWidget(new QLabel("入力画像:"));
    fileLayout->addWidget(imgInputFileEdit);
    fileLayout->addWidget(imgBrowseBtn);
    fileLayout->addWidget(imgClearBtn);
    
    layout->addLayout(fileLayout);
    layout->addStretch();

    imgRunBtn = new QPushButton("PDF生成 (実行)");
    layout->addWidget(imgRunBtn);

    connect(imgBrowseBtn, &QPushButton::clicked, this, &MainWindow::onImgBrowseBtnClicked);
    connect(imgClearBtn, &QPushButton::clicked, this, &MainWindow::onImgClearBtnClicked);
    connect(imgRunBtn, &QPushButton::clicked, this, &MainWindow::onImgRunBtnClicked);

    return w;
}

void MainWindow::onImgBrowseBtnClicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "画像を選択", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!files.isEmpty()) {
        QString current = imgInputFileEdit->text();
        QString added = files.join(" | ");
        if (current.isEmpty()) {
            imgInputFileEdit->setText(added);
        } else {
            imgInputFileEdit->setText(current + " | " + added);
        }
    }
}

void MainWindow::onImgClearBtnClicked() {
    imgInputFileEdit->clear();
}

void MainWindow::onImgRunBtnClicked() {
    if (imgInputFileEdit->text().isEmpty()) {
        showMessage("画像が選択されていません。", true);
        return;
    }

    QString outFile = QFileDialog::getSaveFileName(this, "保存先ファイル名", "", "PDF (*.pdf)");
    if (outFile.isEmpty()) return;
    if (!outFile.endsWith(".pdf", Qt::CaseInsensitive)) outFile += ".pdf";

    QStringList fileList = imgInputFileEdit->text().split(" | ", Qt::SkipEmptyParts);
    std::vector<std::string> paths;
    for (const QString& f : fileList) {
        paths.push_back(f.toStdString());
    }

    std::string err;
    if (PdfWorkers::convertImagesToPdf(paths, outFile.toStdString(), err)) {
        showMessage(QString("保存しました: %1").arg(outFile));
    } else {
        showMessage(QString("エラー: %1").arg(QString::fromStdString(err)), true);
        QMessageBox::critical(this, "Error", QString::fromStdString(err));
    }
}

// ----- Tab 2: Merge PDF -----
QWidget* MainWindow::createMergePdfTab() {
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(w);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    mergeInputFileEdit = new QLineEdit();
    mergeInputFileEdit->setReadOnly(true);
    mergeInputFileEdit->installEventFilter(this);
    mergeBrowseBtn = new QPushButton("参照...");
    fileLayout->addWidget(new QLabel("入力PDF:"));
    fileLayout->addWidget(mergeInputFileEdit);
    fileLayout->addWidget(mergeBrowseBtn);
    
    layout->addLayout(fileLayout);
    layout->addStretch();

    mergeRunBtn = new QPushButton("結合 (実行)");
    layout->addWidget(mergeRunBtn);

    connect(mergeBrowseBtn, &QPushButton::clicked, this, &MainWindow::onMergeBrowseBtnClicked);
    connect(mergeRunBtn, &QPushButton::clicked, this, &MainWindow::onMergeRunBtnClicked);

    return w;
}

void MainWindow::onMergeBrowseBtnClicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "PDFを選択", "", "PDF (*.pdf)");
    if (!files.isEmpty()) {
        QString current = mergeInputFileEdit->text();
        QString added = files.join(" | ");
        if (current.isEmpty()) {
            mergeInputFileEdit->setText(added);
        } else {
            mergeInputFileEdit->setText(current + " | " + added);
        }
    }
}

void MainWindow::onMergeRunBtnClicked() {
    if (mergeInputFileEdit->text().isEmpty()) {
        showMessage("結合するPDFが選択されていません。", true);
        return;
    }

    QString outFile = QFileDialog::getSaveFileName(this, "保存先ファイル名", "", "PDF (*.pdf)");
    if (outFile.isEmpty()) return;
    if (!outFile.endsWith(".pdf", Qt::CaseInsensitive)) outFile += ".pdf";

    QStringList fileList = mergeInputFileEdit->text().split(" | ", Qt::SkipEmptyParts);
    std::vector<std::string> paths;
    for (const QString& f : fileList) {
        paths.push_back(f.toStdString());
    }

    std::string err;
    if (PdfWorkers::mergePdfs(paths, outFile.toStdString(), err)) {
        showMessage(QString("結合しました: %1").arg(outFile));
    } else {
        showMessage(QString("エラー: %1").arg(QString::fromStdString(err)), true);
        QMessageBox::critical(this, "Error", QString::fromStdString(err));
    }
}

// ----- Tab 3: Rotate PDF -----
QWidget* MainWindow::createRotatePdfTab() {
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(w);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    rotateInputFileEdit = new QLineEdit();
    rotateInputFileEdit->setReadOnly(true);
    rotateInputFileEdit->installEventFilter(this);
    rotateBrowseBtn = new QPushButton("参照...");
    fileLayout->addWidget(new QLabel("入力PDF:"));
    fileLayout->addWidget(rotateInputFileEdit);
    fileLayout->addWidget(rotateBrowseBtn);

    QHBoxLayout* condLayout = new QHBoxLayout();
    rotateConditionEdit = new QLineEdit();
    rotateConditionEdit->setPlaceholderText("例: 2-4:270 または all:90");
    condLayout->addWidget(new QLabel("条件（ページ:角度）:"));
    condLayout->addWidget(rotateConditionEdit);

    rotateRunBtn = new QPushButton("回転 (実行)");

    layout->addLayout(fileLayout);
    layout->addLayout(condLayout);
    layout->addStretch();
    layout->addWidget(rotateRunBtn);

    connect(rotateBrowseBtn, &QPushButton::clicked, this, &MainWindow::onRotateBrowseBtnClicked);
    connect(rotateRunBtn, &QPushButton::clicked, this, &MainWindow::onRotateRunBtnClicked);

    return w;
}

void MainWindow::onRotateBrowseBtnClicked() {
    QString file = QFileDialog::getOpenFileName(this, "PDFを選択", "", "PDF (*.pdf)");
    if (!file.isEmpty()) {
        rotateInputFileEdit->setText(file);
    }
}

void MainWindow::onRotateRunBtnClicked() {
    QString inFile = rotateInputFileEdit->text();
    QString cond = rotateConditionEdit->text();
    
    if (inFile.isEmpty() || cond.isEmpty()) {
        showMessage("ファイルまたは条件が入力されていません。", true);
        return;
    }

    QString outFile = QFileDialog::getSaveFileName(this, "保存先ファイル名", "", "PDF (*.pdf)");
    if (outFile.isEmpty()) return;
    if (!outFile.endsWith(".pdf", Qt::CaseInsensitive)) outFile += ".pdf";

    std::string err;
    if (PdfWorkers::rotatePdfPages(inFile.toStdString(), outFile.toStdString(), cond.toStdString(), err)) {
        showMessage(QString("回転完了: %1").arg(outFile));
    } else {
        showMessage(QString("エラー: %1").arg(QString::fromStdString(err)), true);
        QMessageBox::critical(this, "Error", QString::fromStdString(err));
    }
}

// ----- Tab 4: Spread PDF -----
QWidget* MainWindow::createSpreadPdfTab() {
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(w);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    spreadInputFileEdit = new QLineEdit();
    spreadInputFileEdit->setReadOnly(true);
    spreadInputFileEdit->installEventFilter(this);
    spreadBrowseBtn = new QPushButton("参照...");
    fileLayout->addWidget(new QLabel("入力PDF:"));
    fileLayout->addWidget(spreadInputFileEdit);
    fileLayout->addWidget(spreadBrowseBtn);

    spreadRunBtn = new QPushButton("見開き化 (実行)");

    layout->addLayout(fileLayout);
    layout->addStretch();
    layout->addWidget(spreadRunBtn);

    connect(spreadBrowseBtn, &QPushButton::clicked, this, &MainWindow::onSpreadBrowseBtnClicked);
    connect(spreadRunBtn, &QPushButton::clicked, this, &MainWindow::onSpreadRunBtnClicked);

    return w;
}

void MainWindow::onSpreadBrowseBtnClicked() {
    QString file = QFileDialog::getOpenFileName(this, "PDFを選択", "", "PDF (*.pdf)");
    if (!file.isEmpty()) {
        spreadInputFileEdit->setText(file);
    }
}

void MainWindow::onSpreadRunBtnClicked() {
    QString inFile = spreadInputFileEdit->text();
    
    if (inFile.isEmpty()) {
        showMessage("ファイルが選択されていません。", true);
        return;
    }

    QString outFile = QFileDialog::getSaveFileName(this, "保存先ファイル名", "", "PDF (*.pdf)");
    if (outFile.isEmpty()) return;
    if (!outFile.endsWith(".pdf", Qt::CaseInsensitive)) outFile += ".pdf";

    std::string err;
    if (PdfWorkers::createSpreadPdf(inFile.toStdString(), outFile.toStdString(), err)) {
        showMessage(QString("見開きPDF作成完了: %1").arg(outFile));
    } else {
        showMessage(QString("エラー: %1").arg(QString::fromStdString(err)), true);
        QMessageBox::critical(this, "Error", QString::fromStdString(err));
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        if (obj == imgInputFileEdit) {
            onImgBrowseBtnClicked();
            return true;
        } else if (obj == mergeInputFileEdit) {
            onMergeBrowseBtnClicked();
            return true;
        } else if (obj == rotateInputFileEdit) {
            onRotateBrowseBtnClicked();
            return true;
        } else if (obj == spreadInputFileEdit) {
            onSpreadBrowseBtnClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

