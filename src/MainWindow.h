#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include "def.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

    void initMainWindow();
    void showPlateRecogProcessInfo();

private:
    //-------Control----------//
    QLabel *m_programNameLabel;
    QLabel *m_programVersionLabel;
    QLabel *m_programCopyrightLabel;

    QLineEdit *m_srcImageLineEdit;
    QPushButton *m_browseButton;

    QLabel *m_srcImageLabel;
    QLabel *m_locatePlateLabel;
    QLabel *m_partCharLabel[CHARS_LENGHT];
    QLabel *m_recognizeCharLabel;

    QGroupBox *m_srcImageGroupBox;
    QGroupBox *m_locatePlateGroupBox;
    QGroupBox *m_partCharGroupBox;
    QGroupBox *m_recognizeCharGroupBox;

    QPushButton *m_recognizeButton;

    // --------Layout----------//
    QHBoxLayout *m_topHBoxLayout;
    QHBoxLayout *m_centerHBoxLayout;
    QHBoxLayout *m_bottomHBoxLayout;
    QVBoxLayout *m_rightVBoxLayout;
    QVBoxLayout *m_mainVBoxLayout;

    QHBoxLayout *m_srcImageHBoxLayout;
    QHBoxLayout *m_locatePlateHBoxLayout;
    QHBoxLayout *m_partCharHBoxLayout;
    QHBoxLayout *m_recognizeCharHBoxLayout;

signals:

public slots:
    void onClickBrowseButton();
    void onClickRecognizeButton();
};

#endif // MAINWINDOW_H
