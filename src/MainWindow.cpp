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

    imgListWidget = new QListWidget();
    layout->addWidget(imgListWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(24);
    imgAddBtn = new QPushButton("画像を選択");
    imgRunBtn = new QPushButton("PDF生成 (実行)");
    
    btnLayout->addWidget(imgAddBtn);
    btnLayout->addWidget(imgRunBtn);
    
    layout->addLayout(btnLayout);

    connect(imgAddBtn, &QPushButton::clicked, this, &MainWindow::onImgAddBtnClicked);
    connect(imgRunBtn, &QPushButton::clicked, this, &MainWindow::onImgRunBtnClicked);

    return w;
}

void MainWindow::onImgAddBtnClicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "画像を選択", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    imgListWidget->addItems(files);
}

void MainWindow::onImgRunBtnClicked() {
    if (imgListWidget->count() == 0) {
        showMessage("画像が選択されていません。", true);
        return;
    }

    QString outFile = QFileDialog::getSaveFileName(this, "保存先ファイル名", "", "PDF (*.pdf)");
    if (outFile.isEmpty()) return;

    std::vector<std::string> paths;
    for (int i = 0; i < imgListWidget->count(); ++i) {
        paths.push_back(imgListWidget->item(i)->text().toStdString());
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

    mergeListWidget = new QListWidget();
    layout->addWidget(mergeListWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(24);
    mergeAddBtn = new QPushButton("PDFを選択");
    mergeRunBtn = new QPushButton("結合 (実行)");
    
    btnLayout->addWidget(mergeAddBtn);
    btnLayout->addWidget(mergeRunBtn);
    
    layout->addLayout(btnLayout);

    connect(mergeAddBtn, &QPushButton::clicked, this, &MainWindow::onMergeAddBtnClicked);
    connect(mergeRunBtn, &QPushButton::clicked, this, &MainWindow::onMergeRunBtnClicked);

    return w;
}

void MainWindow::onMergeAddBtnClicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "PDFを選択", "", "PDF (*.pdf)");
    mergeListWidget->addItems(files);
}

void MainWindow::onMergeRunBtnClicked() {
    if (mergeListWidget->count() < 1) {
        showMessage("結合するPDFが選択されていません。", true);
        return;
    }

    QString outFile = QFileDialog::getSaveFileName(this, "保存先ファイル名", "", "PDF (*.pdf)");
    if (outFile.isEmpty()) return;

    std::vector<std::string> paths;
    for (int i = 0; i < mergeListWidget->count(); ++i) {
        paths.push_back(mergeListWidget->item(i)->text().toStdString());
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

    std::string err;
    if (PdfWorkers::createSpreadPdf(inFile.toStdString(), outFile.toStdString(), err)) {
        showMessage(QString("見開きPDF作成完了: %1").arg(outFile));
    } else {
        showMessage(QString("エラー: %1").arg(QString::fromStdString(err)), true);
        QMessageBox::critical(this, "Error", QString::fromStdString(err));
    }
}
