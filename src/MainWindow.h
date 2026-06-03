#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QEvent>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
    void showMessage(const QString& msg, bool isError = false);

private slots:
    void onImgBrowseBtnClicked();
    void onImgClearBtnClicked();
    void onImgRunBtnClicked();

    void onMergeBrowseBtnClicked();
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
    QLineEdit* imgInputFileEdit;
    QPushButton* imgBrowseBtn;
    QPushButton* imgClearBtn;
    QPushButton* imgRunBtn;

    // Tab 2: Merge PDF
    QWidget* createMergePdfTab();
    QLineEdit* mergeInputFileEdit;
    QPushButton* mergeBrowseBtn;
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
