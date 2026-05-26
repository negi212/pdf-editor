#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showMessage(const QString& msg, bool isError = false);

private slots:
    void onImgAddBtnClicked();
    void onImgRunBtnClicked();

    void onMergeAddBtnClicked();
    void onMergeRunBtnClicked();

    void onRotateBrowseBtnClicked();
    void onRotateRunBtnClicked();

    void onSpreadBrowseBtnClicked();
    void onSpreadRunBtnClicked();

private:
    QTabWidget *tabWidget;
    QStatusBar *statusBar;
    
    // Tab 1: Image to PDF
    QWidget* createImageToPdfTab();
    QListWidget* imgListWidget;
    QPushButton* imgAddBtn;
    QPushButton* imgRunBtn;

    // Tab 2: Merge PDF
    QWidget* createMergePdfTab();
    QListWidget* mergeListWidget;
    QPushButton* mergeAddBtn;
    QPushButton* mergeRunBtn;

    // Tab 3: Rotate PDF
    QWidget* createRotatePdfTab();
    QLineEdit* rotateInputFileEdit;
    QPushButton* rotateBrowseBtn;
    QLineEdit* rotateConditionEdit;
    QPushButton* rotateRunBtn;

    // Tab 4: Spread PDF
    QWidget* createSpreadPdfTab();
    QLineEdit* spreadInputFileEdit;
    QPushButton* spreadBrowseBtn;
    QPushButton* spreadRunBtn;
};
