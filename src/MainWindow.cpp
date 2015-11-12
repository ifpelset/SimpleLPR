#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include "MainWindow.h"
#include "PlateRecognition.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    initMainWindow();
}

void MainWindow::initMainWindow()
{
    m_programNameLabel = new QLabel("<i>SimpleLPR</i>");
    m_programVersionLabel = new QLabel("<sub>version 1.0</sub>");
    m_programCopyrightLabel = new QLabel("<i>Powered by</i> IFPELSET");

    m_srcImageLineEdit = new QLineEdit;
    m_browseButton = new QPushButton("浏览...");

    m_srcImageLabel = new QLabel("...");
    m_locatePlateLabel = new QLabel("...");

    for (int i=0; i<CHARS_LENGHT; i++)
        m_partCharLabel[i] = new QLabel(QString("..."));

    m_recognizeCharLabel = new QLabel("...");
    m_recognizeButton = new QPushButton("一键识别");

    m_srcImageGroupBox = new QGroupBox("原图像");
    m_locatePlateGroupBox = new QGroupBox("车牌定位");
    m_partCharGroupBox = new QGroupBox("字符分割");
    m_recognizeCharGroupBox = new QGroupBox("字符识别");

    m_topHBoxLayout = new QHBoxLayout;
    m_centerHBoxLayout = new QHBoxLayout;
    m_bottomHBoxLayout = new QHBoxLayout;
    m_rightVBoxLayout = new QVBoxLayout;
    m_mainVBoxLayout = new QVBoxLayout(this);

    m_srcImageHBoxLayout = new QHBoxLayout;
    m_locatePlateHBoxLayout = new QHBoxLayout;
    m_partCharHBoxLayout = new QHBoxLayout;
    m_recognizeCharHBoxLayout = new QHBoxLayout;

    m_programNameLabel->setStyleSheet("font-size:30px;");
    m_srcImageLineEdit->setPlaceholderText("请点击右边的浏览选择识别的图像");
    m_srcImageLineEdit->setEnabled(false);
    m_recognizeCharLabel->setStyleSheet("font-size:25px;");
    connect(m_browseButton, SIGNAL(clicked(bool)), this, SLOT(onClickBrowseButton()));
    connect(m_recognizeButton, SIGNAL(clicked(bool)), this, SLOT(onClickRecognizeButton()));

    m_topHBoxLayout->addWidget(m_programNameLabel);
    m_topHBoxLayout->addWidget(m_programVersionLabel);
    m_topHBoxLayout->addStretch(1);
    m_topHBoxLayout->addWidget(m_programCopyrightLabel);

    m_centerHBoxLayout->addWidget(m_srcImageLineEdit);
    m_centerHBoxLayout->addWidget(m_browseButton);

    m_srcImageGroupBox->setStyleSheet("border: 2px solid gray;\
                        border-radius: 5px;\
                        margin-top: 1ex;");
    m_srcImageHBoxLayout->addWidget(m_srcImageLabel);
    m_srcImageGroupBox->setLayout(m_srcImageHBoxLayout);

    m_bottomHBoxLayout->addWidget(m_srcImageGroupBox, 2);
    m_bottomHBoxLayout->addLayout(m_rightVBoxLayout, 1);

    m_locatePlateHBoxLayout->addWidget(m_locatePlateLabel);
    m_locatePlateGroupBox->setLayout(m_locatePlateHBoxLayout);
    m_rightVBoxLayout->addWidget(m_locatePlateGroupBox);

    for (int i=0; i<CHARS_LENGHT; i++)
        m_partCharHBoxLayout->addWidget(m_partCharLabel[i]);

    m_partCharGroupBox->setLayout(m_partCharHBoxLayout);
    m_rightVBoxLayout->addWidget(m_partCharGroupBox);

    m_recognizeCharHBoxLayout->addWidget(m_recognizeCharLabel);
    m_recognizeCharGroupBox->setLayout(m_recognizeCharHBoxLayout);
    m_rightVBoxLayout->addWidget(m_recognizeCharGroupBox);

    // 由于Qt存在的bug，该布局管理器不能加入到成员变量中
    // 该布局管理器为让“一键识别”按钮居中显示
    QHBoxLayout *btnHBoxLayout = new QHBoxLayout;
    m_rightVBoxLayout->addLayout(btnHBoxLayout);

    btnHBoxLayout->addStretch(1);
    btnHBoxLayout->addWidget(m_recognizeButton);
    btnHBoxLayout->addStretch(1);

    m_mainVBoxLayout->addLayout(m_topHBoxLayout);
    m_mainVBoxLayout->addLayout(m_centerHBoxLayout);
    m_mainVBoxLayout->addLayout(m_bottomHBoxLayout, 1);

    //920, 520
    resize(975, 560);
}

void MainWindow::showPlateRecogProcessInfo()
{
    // ---- 车牌定位
    QString srcImagePath = m_srcImageLineEdit->text();
    if (srcImagePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "您还未选择图片，请选择需要识别的图片。");
        return ;
    }

    IplImage * plateImage = PlateRecognition::locatePlate(srcImagePath);

    if (plateImage == NULL ){
        return ;
    }

    QImage qimage((unsigned char * )plateImage->imageDataOrigin,
                  plateImage->width,
                  plateImage->height,
                  plateImage->widthStep,
                  QImage::Format_Indexed8);
    QPixmap lpmap;
    m_locatePlateLabel->setPixmap(lpmap.fromImage(qimage,0).scaled(200, 40));

    // --- 字符分割
    IplImage ** charImages = PlateRecognition::partChar(plateImage);
    if (charImages == NULL) {
        return ;
    }

    QImage *charImage[CHARS_LENGHT];
    for (int i=0; i<CHARS_LENGHT; i++) {
         charImage[i] = new QImage((unsigned char * )charImages[i]->imageDataOrigin,
                charImages[i]->width,
                charImages[i]->height,
                charImages[i]->widthStep,
                QImage::Format_Indexed8);
        m_partCharLabel[i]->setPixmap(lpmap.fromImage(*(charImage[i]),0).scaled(30, 40));
    }

    // --- 字符识别
    QString recognizeResult = PlateRecognition::recognizeChar(charImages);
    m_recognizeCharLabel->setText("");
    m_recognizeCharLabel->setText(recognizeResult);
}

void MainWindow::onClickBrowseButton()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"),
        "../",
        tr("Image Files (*.png *.jpg *.bmp)"));

    if (fileName.isEmpty()) {
        return ;
    }

    m_srcImageLineEdit->setText(fileName);
    m_srcImageLabel->setPixmap(QPixmap(fileName).scaled(600, 380));
}

void MainWindow::onClickRecognizeButton()
{
    showPlateRecogProcessInfo();
}

