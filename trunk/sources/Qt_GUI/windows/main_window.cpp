/****************************************************************************
**
** Copyright (C) 2010-2013 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

// origin code from Qt exemple "imageviewer"
// http://doc.qt.nokia.com/4.7/widgets-imageviewer.html

////////////////////////// Ofeli /////////////////////////////

// Qt GUI part of Ofeli

#include "main_window.hpp"
#include "settings_window.hpp"
#include "evaluation_window.hpp"
#include "camera_window.hpp"
#include "about_window.hpp"
#include "language_window.hpp"
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"

                    /////////////////////

// image processing part of Ofeli

#include "region_based_active_contour.hpp"
#include "region_based_active_contour_yuv.hpp"
#include "edge_based_active_contour.hpp"
#include "filters.hpp"

//////////////////////////////////////////////////////////////

#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include <ctime>         // for std::clock_t, std::clock() and CLOCKS_PER_SEC
#include <cstring>       // for std::memcpy
// if you want a partial support of DICOM image, without support of JPEG-2000 encapsulated image
// install DCMTK library to the project
// uncomment row 72, 1002 and the block at 1074.
//#include "dcmtk/dcmimgle/dcmimage.h"
#include "matrix.hpp"
namespace ofeli
{

MainWindow::MainWindow() :
    img1(nullptr), filters1(nullptr), ac(nullptr), image_result_uchar(nullptr)
{
    setWindowTitle( tr("Ofeli") );

    if( operating_system == 3 )
    {
        setWindowIcon(QIcon(":Ofeli.png"));
    }

    QSettings settings("Bessy", "Ofeli");

    resize( settings.value("Main/Window/size", QSize(600, 375)).toSize() );
    move( settings.value("Main/Window/position", QPoint(200, 200)).toPoint() );

    //////////////////////////////////////////////////////////////////////////

    scrollArea = new ofeli::ScrollAreaWidget(this);
    imageLabel = new ofeli::PixmapWidget(scrollArea);
    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    imageLabel->setFocusPolicy(Qt::StrongFocus);
    imageLabel->setMouseTracking(true);
    imageLabel->installEventFilter(this);
    imageLabel->setAcceptDrops(true);
    imageLabel->resize(280, 487);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->set_text(tr("<drag image>"));

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->installEventFilter(this);
    connect(scrollArea->verticalScrollBar(),SIGNAL(rangeChanged(int,int)),this,SLOT(adjustVerticalScroll(int,int)));
    connect(scrollArea->horizontalScrollBar(),SIGNAL(rangeChanged(int,int)),this,SLOT(adjustHorizontalScroll(int,int)));

    //////////////////////////////////////////////////////////////////////////

    Cin_text = new QLabel(this);
    Cin_text->setText("Cin =");
    Cin_text->setAlignment(Qt::AlignRight);
    Cout_text = new QLabel(this);
    Cout_text->setText("Cout =");
    Cout_text->setToolTip(tr("outside average of the image"));
    Cout_text->setAlignment(Qt::AlignRight);
    Cin_text->setToolTip(tr("inside average of the image"));
    threshold_text = new QLabel(this);
    threshold_text->setToolTip(tr("0.7 times this threshold before to begin the steepest descent"));
    threshold_text->setAlignment(Qt::AlignRight);

    QGroupBox* speed1_group = new QGroupBox(tr("Chan-Vese model information"));
    QVBoxLayout* chanvese_text = new QVBoxLayout;
    chanvese_text->addWidget(Cout_text);
    chanvese_text->addWidget(Cin_text);
    speed1_group->setLayout(chanvese_text);

    QGroupBox* speed2_group = new QGroupBox(tr("Geodesic model information"));
    QVBoxLayout* threshold_layout = new QVBoxLayout;
    threshold_layout->addWidget(threshold_text);
    speed2_group->setLayout(threshold_layout);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(speed1_group);
    stackedWidget->addWidget(speed2_group);

    //////////////////////////////////////////////////////////////////////////

    text = new QLabel(this);
    text->setText(tr("iteration t ="));
    text->setToolTip(tr("must be strictly less than iteration_max = 5 × max(img_width,img_height)"));
    text->setAlignment(Qt::AlignRight);
    changes_text = new QLabel(this);
    changes_text->setText(tr("lists changes ="));
    changes_text->setToolTip(tr("updated only for each iteration of the cycle 1"));
    changes_text->setAlignment(Qt::AlignRight);
    oscillation_text = new QLabel(this);
    oscillation_text->setText(tr("oscillation ="));
    oscillation_text->setToolTip(tr("updated only at the end of the cycle 2"));
    oscillation_text->setAlignment(Qt::AlignRight);

    QGroupBox* stop_group = new QGroupBox(tr("Stopping conditions"));
    QVBoxLayout* condi_layout = new QVBoxLayout;
    condi_layout->addWidget(text);
    condi_layout->addWidget(changes_text);
    condi_layout->addWidget(oscillation_text);
    stop_group->setLayout(condi_layout);

    //////////////////////////////////////////////////////////////////////////

    pixel_text = new QLabel(this);
    pixel_text->setMinimumWidth(248);
    pixel_text->setText(tr("img(x,y) ="));
    pixel_text->setToolTip(tr("pixel value at the position (x,y)"));
    pixel_text->setAlignment(Qt::AlignRight);
    phi_text = new QLabel(this);
    phi_text->setText("ϕ(x,y,t) =");
    phi_text->setToolTip(tr("level set function value at the position (x,y,t), value ∈ { -3, -1, 1, 3 }"));
    phi_text->setAlignment(Qt::AlignRight);
    lists_text = new QLabel(this);
    lists_text->setText("Lin(t) & Lout(t)");
    lists_text->setAlignment(Qt::AlignRight);

    QGroupBox* data_group = new QGroupBox(tr("Data, level set function and lists"));
    QVBoxLayout* data_layout = new QVBoxLayout;
    data_layout->addWidget(pixel_text);
    data_layout->addWidget(phi_text);
    data_layout->addWidget(lists_text);
    data_group->setLayout(data_layout);

    //////////////////////////////////////////////////////////////////////////

    time_text = new QLabel(this);
    time_text->setText(tr("time ="));
    time_text->setToolTip(tr("elapsed time in the loop, without the initialization time when the constructor is called"));
    time_text->setAlignment(Qt::AlignRight);

    QGroupBox* time_group = new QGroupBox(tr("Elapsed time"));
    QVBoxLayout* elapsed_layout = new QVBoxLayout;
    elapsed_layout->addWidget(time_text);
    time_group->setLayout(elapsed_layout);

    //////////////////////////////////////////////////////////////////////////

    scale_spin0 = new QSpinBox;
    scale_spin0->setSingleStep(25);
    scale_spin0->setMinimum(1);
    scale_spin0->setMaximum(5000);
    scale_spin0->setSuffix(" %");
    scale_spin0->installEventFilter(this);
    scale_spin0->setMouseTracking(true);
    scale_slider0 = new QSlider(Qt::Horizontal, this);
    if( operating_system == 3 )
    {
        scale_slider0->setTickPosition(QSlider::TicksBelow);
    }
    else
    {
        scale_slider0->setTickPosition(QSlider::TicksAbove);
    }
    scale_slider0->setMinimum(1);
    scale_slider0->setMaximum(1000);
    scale_slider0->setTickInterval(100);
    scale_slider0->setSingleStep(25);
    scale_slider0->setValue(100);
    scale_slider0->installEventFilter(this);
    scale_slider0->setMouseTracking(true);

    QFormLayout* scale_spin_layout = new QFormLayout;
    scale_spin_layout->addRow(tr("scale :"), scale_spin0);
    scale_spin_layout->setFormAlignment(Qt::AlignRight);
    QVBoxLayout* scale_layout = new QVBoxLayout;
    scale_layout->addLayout(scale_spin_layout);
    scale_layout->addWidget(scale_slider0);
    QGroupBox* scale_group = new QGroupBox(tr("Display"));
    scale_group->setLayout(scale_layout);

    connect(scale_spin0,SIGNAL(valueChanged(int)),this,SLOT(do_scale0(int)));
    connect(scale_slider0,SIGNAL(valueChanged(int)),scale_spin0,SLOT(setValue(int)));
    connect(scale_spin0,SIGNAL(valueChanged(int)),scale_slider0,SLOT(setValue(int)));

    scale_spin0->setValue(settings.value("Settings/Display/zoom_factor", 100).toInt());
    imageLabel->set_zoomFactor(float(scale_spin0->value())/100.0f);

    //////////////////////////////////////////////////////////////////////////

    QVBoxLayout* info_layout = new QVBoxLayout;
    info_layout->addWidget(stackedWidget);
    info_layout->addWidget(stop_group);
    info_layout->addWidget(time_group);
    info_layout->addWidget(data_group);
    info_layout->addWidget(scale_group);
    info_layout->addStretch(1);

    QHBoxLayout *layout_this = new QHBoxLayout;
    layout_this->addWidget(scrollArea,1);
    layout_this->addLayout(info_layout,0);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    QLabel* widget_this = new QLabel;
    widget_this->setLayout(layout_this);

    setCentralWidget(widget_this);

    //////////////////////////////////////////////////////////////////////////

    settings_window2 = new ofeli::SettingsWindow(this);
    evaluation_window = new ofeli::EvaluationWindow(this);
    camera_window = new ofeli::CameraWindow(this,settings_window2);
    about_window = new ofeli::AboutWindow(this);
    language_window = new ofeli::LanguageWindow(this);

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Actions                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    QStyle* style =  QApplication::style();

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an image file (*.png, *.bmp, *.jpg, *.jpeg, *.tiff, *.tif, *.gif, *.pbm, *.pgm, *.ppm, *.svg, *.svgz, *.mng, *.xbm, *.xpm)."));
    openAct->setEnabled(true);
    openAct->setIcon( style->standardIcon(QStyle::SP_DirOpenIcon) );
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFileName()));

    cameraAct = new QAction(tr("Open &Camera"), this);
    cameraAct->setShortcut(tr("Ctrl+C"));
    cameraAct->setEnabled(true);
    cameraAct->setIcon( QIcon(":Camera.png") );
    connect(cameraAct, SIGNAL(triggered()), camera_window, SLOT(show_camera()));

    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        recentFileActs[i]->setIcon( QIcon(":RecentFile.png") );
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    deleteAct = new QAction(tr("Clear list"), this);
    deleteAct->setStatusTip(tr("Clean the recent files list."));
    deleteAct->setIcon( QIcon(":ClearList.png") );
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteList()));

    saveAct = new QAction(tr("S&ave..."), this);
    saveAct->setShortcut(tr("Ctrl+A"));
    saveAct->setEnabled(false);
    saveAct->setStatusTip(tr("Save the displayed and preprocessed images."));
    saveAct->setIcon( style->standardIcon(QStyle::SP_DialogSaveButton) );
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveImage()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(false);
    printAct->setIcon( QIcon(":Print.png") );
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+X"));
    exitAct->setIcon( style->standardIcon(QStyle::SP_TitleBarCloseButton) );
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    zoomInAct->setIcon( QIcon(":ZoomIn.png") );
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    zoomOutAct->setIcon( QIcon(":ZoomOut.png") );
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+N"));
    normalSizeAct->setEnabled(false);
    normalSizeAct->setIcon( QIcon(":Zoom1.png") );
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    startAct = new QAction(tr("&Start"), this);
    startAct->setShortcut(tr("Ctrl+S"));
    startAct->setEnabled(false);
    startAct->setStatusTip(tr("Start the active contour."));
    startAct->setIcon( style->standardIcon(QStyle::SP_MediaPlay) );
    connect(startAct, SIGNAL(triggered()), this, SLOT(start()));

    evaluateAct = new QAction(tr("E&valuate"), this);
    evaluateAct->setStatusTip(tr("Compute the modified Hausdorff distance."));
    evaluateAct->setShortcut(tr("Ctrl+V"));
    evaluateAct->setEnabled(true);
    evaluateAct->setIcon( QIcon(":Evaluate.png") );
    connect( evaluateAct, SIGNAL(triggered()), evaluation_window, SLOT(show()) );

    settingsAct = new QAction(tr("S&ettings"), this);
    settingsAct->setStatusTip(tr("Image preprocessing and active contour initialization."));
    settingsAct->setShortcut(tr("Ctrl+E"));
    settingsAct->setEnabled(true);
    settingsAct->setIcon( QIcon(":Settings.png") );
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(settings()));

    menuBar()->addSeparator();

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Information, license and home page."));
    aboutAct->setIcon( QIcon(":Ofeli.png") );
    connect(aboutAct, SIGNAL(triggered()), about_window, SLOT(show()));

    languageAct = new QAction(tr("&Language"), this);
    languageAct->setStatusTip(tr("Choose the application language."));
    languageAct->setIcon( QIcon(":Langage.png") );
    connect(languageAct, SIGNAL(triggered()), this, SLOT(language()));

    docAct = new QAction(tr("&Documentation"), this);
    docAct->setStatusTip(tr("Online developer's documentation."));
    docAct->setIcon( QIcon(":Documentation.png") );
    connect(docAct, SIGNAL(triggered()), this, SLOT(doc()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Menus                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(cameraAct);

    separatorAct = fileMenu->addSeparator();
    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        fileMenu->addAction(recentFileActs[i]);
    }

    fileMenu->addAction(deleteAct);

    fileMenu->addSeparator();

    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    updateRecentFileActions();

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);

    segmentationMenu = new QMenu(tr("&Segmentation"), this);
    segmentationMenu->addAction(startAct);
    segmentationMenu->addAction(evaluateAct);
    segmentationMenu->addAction(settingsAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(languageAct);
    helpMenu->addAction(docAct);
    //helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(segmentationMenu);
    menuBar()->addMenu(helpMenu);

    //////////////////////////////////////////////////////////////////////////

    nameFilters << "*.bmp"
                //<< "*.dcm"
                << "*.gif"
                << "*.jpg" << "*.jpeg" << "*.mng"
                << "*.pbm" << "*.png" << "*.pgm"
                << "*.ppm" << "*.svg" << "*.svgz"
                << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    nameFilters.removeDuplicates();

    last_directory_used = settings.value("Main/Name/last_directory_used",QDir().homePath()).toString();

    statusBar()->clearMessage();

    connect( this, SIGNAL(signal_open()), this, SLOT(stop()), Qt::DirectConnection);
    connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
}

void MainWindow::openFileName()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open File"),
                                            last_directory_used,
                                            tr("Image Files (%1)").arg(nameFilters.join(" ")));


    emit signal_open();
}

// Fonction appelée lorsqu'on ouvre une image
void MainWindow::open()
{
    if( !fileName.isEmpty() )
    {
        QFileInfo fi(fileName);

        // if you want a partial support of DICOM image, without support of JPEG-2000 encapsulated image
        // install DCMTK library to the project (there is the folder "external_libraries")
        // and uncomment below

        /*if( fi.completeSuffix() == "dcm" || fi.completeSuffix() == "DCM" )
        {
            DicomImage* dcm_img = new DicomImage( fileName.toStdString().c_str() );

            if( dcm_img != nullptr)
            {
                if( dcm_img->getStatus() == EIS_Normal )
                {
                    int width = dcm_img->getWidth();
                    int height = dcm_img->getHeight();
                    unsigned char* pixelData = (unsigned char*)( dcm_img->getOutputData(8) );

                    if( pixelData != nullptr )
                    {
                        img = QImage(width,height,QImage::Format_Indexed8);

                        for( int y = 0; y < height; y++ )
                        {
                            for( int x = 0; x < width; x++ )
                            {
                                *( img.scanLine(y)+x ) = pixelData[x+y*width];
                            }
                        }

                    }
                }
                else
                {
                    std::cerr << "Error: cannot load DICOM image (" << DicomImage::getString(dcm_img->getStatus()) << ")" << std::endl;
                }
            }
            delete dcm_img;
        }*/
        //else
        //{
            img = QImage(fileName);
            if( img.isNull() )
            {
                QMessageBox::information(this, tr("Opening error - Ofeli"),
                                         tr("Cannot load %1.").arg(fileName));
                return;
            }
        //}

        image_format = img.format();

        if( image_format == 3 ) // si l'image est en niveau de gris
        {
            QVector<QRgb> table(256);
            for( int I = 0; I < 256; I++ )
            {
                table[I] = qRgb(I,I,I);
            }
            img.setColorTable(table);
        }

        imageLabel->set_qimage0(img);
        QApplication::processEvents();
        statusBar()->showMessage(tr("Push the left mouse button or click on the right mouse button in the window."));

        iteration1 = -1;

        QString name = fi.fileName();
        last_directory_used = fi.absolutePath();
        setCurrentFile(fileName);

        img_width = img.width();
        img_height = img.height();
        img_size = img_width*img_height;

        positionX = img_width/2;
        positionY = img_height/2;

        phi_text->setText("ϕ(x,y,t) ∈ { -3, -1, 1, 3 }");
        text->setToolTip(tr("must be strictly less than iteration_max = 5 × max(img_width,img_height) = ")+QString::number(5*std::max(img_width,img_height)));

        if( ac != nullptr )
        {
            delete ac;
            ac = nullptr;
        }

        image_disp = 1;

        QString string_lists_text;
        if( image_format == 3 )
        {
            string_lists_text = QString::number(img_width)+"×"+QString::number(img_height)+"×1";
        }
        else
        {
            string_lists_text = QString::number(img_width)+"×"+QString::number(img_height)+"×3";
        }

        if( operating_system == 2 )
        {
            setWindowTitle(name+" - "+string_lists_text);
        }
        if( operating_system == 1 || operating_system == 3 )
        {
            setWindowTitle(name+" - "+string_lists_text+" - Ofeli");
        }


        // Paramètres d'entrée du contour actif

        // Création de img1
        if( img1 != nullptr )
        {
            delete[] img1;
            img1 = nullptr;
        }

        if( image_format == 3 ) // si l'image est en niveau de gris
        {
            isRgb1 = false;

            img1 = new unsigned char[img_size];

            for( unsigned int y = 0; y < img_height; y++ )
            {
                for( unsigned int x = 0; x < img_width; x++ )
                {
                    img1[ find_offset(x,y) ] = *( img.scanLine(y)+x );
                }
            }
        }
        else // si l'image est en couleur
        {
            if( img.isGrayscale() ) // si les 3 composantes sont identiques
            {
                isRgb1 = false;

                img1 = new unsigned char[img_size];

                QRgb pix;

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        pix = img.pixel(x,y);

                        img1[ find_offset(x,y) ] = (unsigned char)( qRed(pix) );
                    }
                }
            }
            else
            {
                isRgb1 = true;

                img1 = new unsigned char[3*img_size];

                QRgb pix;

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        pix = img.pixel(x,y);

                        img1[ 3*find_offset(x,y) ] = (unsigned char)( qRed(pix) );
                        img1[ 3*find_offset(x,y)+1 ] = (unsigned char)( qGreen(pix) );
                        img1[ 3*find_offset(x,y)+2 ] = (unsigned char)( qBlue(pix) );
                    }
                }
            }
        }

        settings_window2->init(img1,img_width,img_height,isRgb1,img);

        if( image_result_uchar != nullptr )
        {
            delete[] image_result_uchar;
            image_result_uchar = nullptr;
        }
        if( image_result_uchar == nullptr )
        {
            image_result_uchar = new unsigned char[3*img_size];
        }
        image_result = QImage(image_result_uchar, img_width, img_height, 3*img_width, QImage::Format_RGB888);

        if( isRgb1 )
        {
            image_save_preprocess = QImage(img_width, img_height, QImage::Format_RGB888);
        }
        else
        {
            image_save_preprocess = QImage(img_width,img_height, QImage::Format_Indexed8);
        }

        auto mainwindow_settings = settings_window2->get_settings();

        if( isRgb1 && mainwindow_settings.model == 1 )
        {
            Cout_text->setText("Cout = (<font color=red>R<font color=black>,<font color=green>G<font color=black>,<font color=blue>B<font color=black>)");
            Cin_text->setText("Cin = (<font color=red>R<font color=black>,<font color=green>G<font color=black>,<font color=blue>B<font color=black>)");

            Cout_text->setToolTip(tr("outside average of the image in the (R,G,B) color space"));
            Cin_text->setToolTip(tr("inside average of the image in (R,G,B) color space"));
        }
        if( !isRgb1 && mainwindow_settings.model == 1 )
        {
            Cout_text->setText("Cout =");
            Cin_text->setText("Cin =");

            Cout_text->setToolTip(tr("outside average of the image"));
            Cin_text->setToolTip(tr("inside average of the image"));
        }

        if( isRgb1 )
        {
            pixel_text->setText(tr("img(x,y) = (<font color=red>R<font color=black>,<font color=green>G<font color=black>,<font color=blue>B<font color=black>)"));
            pixel_text->setToolTip(tr("pixel value at the position (x,y) in the (R,G,B) color space"));
        }
        else
        {
            pixel_text->setText(tr("img(x,y) = I"));
            pixel_text->setToolTip(tr("pixel value at the position (x,y)"));
        }

        lists_text->setText("Lin(t) & Lout(t)");

        disconnect(scale_spin0,SIGNAL(valueChanged(int)),this,SLOT(do_scale0(int)));

        scale_spin0->setMaximum(1000000/img_height);
        scale_spin0->setSingleStep(80000/(7*img_height));

        scale_slider0->setMaximum(160000/img_height);
        scale_slider0->setTickInterval(160000/(7*img_height));

        connect(scale_spin0,SIGNAL(valueChanged(int)),this,SLOT(do_scale0(int)));

        if( img1 != nullptr )
        {
            hasAlgoBreaking = false;
            hasStepByStep = false;
            hasClickStopping = false;
            printAct->setEnabled(true);
            zoomInAct->setEnabled(true);
            zoomOutAct->setEnabled(true);
            normalSizeAct->setEnabled(true);
            startAct->setEnabled(true);
            saveAct->setEnabled(true);
        }
    }
}

// Fonction appelée pour effectuer la segmentation
void MainWindow::start()
{
    disconnect( this, SIGNAL(signal_open()), this, SLOT(open()) );
    startAct->setEnabled(false);

    if( img1 != nullptr )
    {
        if( filters1 != nullptr )
        {
            delete filters1;
            filters1 = nullptr;
        }

        if( filters1 == nullptr )
        {
            if( isRgb1 )
            {
                filters1 = new ofeli::Filters(img1, img_width, img_height, 3);
            }
            else
            {
                filters1 = new ofeli::Filters(img1, img_width, img_height, 1);
            }
        }

        auto mainwindow_settings = settings_window2->get_settings();

        if( mainwindow_settings.hasPreprocess )
        {
            if( mainwindow_settings.hasGaussianNoise )
            {
                filters1->gaussian_white_noise(mainwindow_settings.std_noise);
            }
            if( mainwindow_settings.hasSaltNoise )
            {
                filters1->salt_pepper_noise(mainwindow_settings.proba_noise);
            }
            if( mainwindow_settings.hasSpeckleNoise )
            {
                filters1->speckle(mainwindow_settings.std_speckle_noise);
            }

            if( mainwindow_settings.hasMeanFilt )
            {
                filters1->mean_filtering(mainwindow_settings.kernel_mean_length1);
            }
            if( mainwindow_settings.hasGaussianFilt )
            {
                filters1->gaussian_filtering(mainwindow_settings.kernel_gaussian_length1, mainwindow_settings.sigma);
            }

            if( mainwindow_settings.hasMedianFilt )
            {
                if( mainwindow_settings.hasO1algo1 )
                {
                    filters1->median_filtering_o1(mainwindow_settings.kernel_median_length1);
                }
                else
                {
                    filters1->median_filtering_oNlogN(mainwindow_settings.kernel_median_length1);
                }
            }
            if( mainwindow_settings.hasAnisoDiff )
            {
                filters1->anisotropic_diffusion(mainwindow_settings.max_itera1, mainwindow_settings.lambda1, mainwindow_settings.kappa1, mainwindow_settings.aniso_option1);
            }
            if( mainwindow_settings.hasOpenFilt )
            {
                if( mainwindow_settings.hasO1morpho1 )
                {
                    filters1->opening_o1(mainwindow_settings.kernel_open_length1);
                }
                else
                {
                    filters1->opening(mainwindow_settings.kernel_open_length1);
                }
            }
            if( mainwindow_settings.hasCloseFilt )
            {
                if( mainwindow_settings.hasO1morpho1 )
                {
                    filters1->closing_o1(mainwindow_settings.kernel_close_length1);
                }
                else
                {
                    filters1->closing(mainwindow_settings.kernel_close_length1);
                }
            }
            if( mainwindow_settings.hasTophatFilt )
            {
                if( mainwindow_settings.isWhiteTophat )
                {
                    if( mainwindow_settings.hasO1morpho1 )
                    {
                        filters1->white_top_hat_o1(mainwindow_settings.kernel_tophat_length1);
                    }
                    else
                    {
                        filters1->white_top_hat(mainwindow_settings.kernel_tophat_length1);
                    }
                }
                else
                {
                    if( mainwindow_settings.hasO1morpho1 )
                    {
                        filters1->black_top_hat_o1(mainwindow_settings.kernel_tophat_length1);
                    }
                    else
                    {
                        filters1->black_top_hat(mainwindow_settings.kernel_tophat_length1);
                    }
                }
            }
        }

        const unsigned char* img1_filtered = nullptr;

        if( mainwindow_settings.model == 1 )
        {
            img1_filtered = filters1->get_filtered();
        }
        if( mainwindow_settings.model == 2 )
        {
            if( isRgb1 )
            {
                filters1->morphological_gradient_yuv(mainwindow_settings.kernel_gradient_length1,mainwindow_settings.alpha1,mainwindow_settings.beta1,mainwindow_settings.gamma1);
                img1_filtered = filters1->get_gradient();
            }
            else
            {
                filters1->morphological_gradient(mainwindow_settings.kernel_gradient_length1);
                img1_filtered = filters1->get_filtered();
            }
        }

        if( ac != nullptr )
        {
            delete ac;
            ac = nullptr;
        }

        // Création d'un objet contour actif
        // objets crees en dynamique et appel du 2eme constructeur
        ofeli::Matrix<const signed char> phi_init1(mainwindow_settings.phi_init,img_width,img_height);
        ofeli::Matrix<const unsigned char> matrix_img1(img1_filtered,img_width,img_height);

        if( mainwindow_settings.model == 1 )
        {
            if( isRgb1 )
            {
                ac = new ofeli::RegionBasedActiveContourYUV(matrix_img1,
                                                  phi_init1,
                                                  mainwindow_settings.hasSmoothingCycle1, mainwindow_settings.kernel_curve1, mainwindow_settings.std_curve1, mainwindow_settings.Na1, mainwindow_settings.Ns1,
                                                  mainwindow_settings.lambda_out1, mainwindow_settings.lambda_in1, mainwindow_settings.alpha1, mainwindow_settings.beta1, mainwindow_settings.gamma1);
            }
            else
            {
                ac = new ofeli::RegionBasedActiveContour(matrix_img1,
                                               phi_init1,
                                               mainwindow_settings.hasSmoothingCycle1, mainwindow_settings.kernel_curve1, mainwindow_settings.std_curve1, mainwindow_settings.Na1, mainwindow_settings.Ns1,
                                               mainwindow_settings.lambda_out1, mainwindow_settings.lambda_in1);
            }
        }

        if( mainwindow_settings.model == 2 )
        {
            ac = new ofeli::EdgeBasedActiveContour(matrix_img1,
                                       phi_init1,
                                       mainwindow_settings.hasSmoothingCycle1, mainwindow_settings.kernel_curve1, mainwindow_settings.std_curve1, mainwindow_settings.Na1, mainwindow_settings.Ns1);
        }

        show_phi_list_value();

        // Conditions de la boucle do-while

        iteration1 = ac->get_iteration();

        if( hasAlgoBreaking )
        {
            hasAlgoBreaking = false;
            startAct->setEnabled(true);
            connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
            emit signal_open();
            return;
        }

        time_text->setText(tr("time ="));

        if( mainwindow_settings.model == 1 )
        {
            stackedWidget->setCurrentIndex(0);
        }

        if( mainwindow_settings.model == 2 )
        {
            stackedWidget->setCurrentIndex(1);
        }

        //////////////////////////////////////////////////////////////////////
        // affiche une fois le contour actif avec l'image filtrée en dessous
        //////////////////////////////////////////////////////////////////////

        unsigned char I, max, min;
        // assignment in order to prevent a warning in GCC
        max = 0;
        min = 255;

        unsigned int offset;

        if( !isRgb1 || mainwindow_settings.model == 2 )
        {
            if( mainwindow_settings.model == 2 && mainwindow_settings.hasHistoNormaliz )
            {
                max = 0;
                min = 255;

                for( offset = 0; offset < img_size; offset++ )
                {
                    if( img1_filtered[offset] > max )
                    {
                        max = img1_filtered[offset];
                    }
                    if( img1_filtered[offset] < min )
                    {
                        min = img1_filtered[offset];
                    }
                }

                for( offset = 0; offset < img_size; offset++ )
                {
                    I = (unsigned char)(255.0*double(img1_filtered[offset]-min)/double(max-min));

                    image_result_uchar[3*offset] = I;
                    image_result_uchar[3*offset+1] = I;
                    image_result_uchar[3*offset+2] = I;
                }
            }

            else
            {
                for( offset = 0; offset < img_size; offset++ )
                {
                    I = img1_filtered[offset];

                    image_result_uchar[3*offset] = I;
                    image_result_uchar[3*offset+1] = I;
                    image_result_uchar[3*offset+2] = I;
                }
            }
        }
        else
        {
            std::memcpy(image_result_uchar,img1_filtered,3*img_size);
        }

        if( mainwindow_settings.hasGaussianNoise || mainwindow_settings.hasSaltNoise || mainwindow_settings.hasSpeckleNoise || mainwindow_settings.hasMeanFilt || mainwindow_settings.hasGaussianFilt || mainwindow_settings.hasMedianFilt || mainwindow_settings.hasAnisoDiff || mainwindow_settings.hasOpenFilt || mainwindow_settings.hasCloseFilt || mainwindow_settings.hasTophatFilt || mainwindow_settings.model == 2 )
        {
            if( isRgb1 )
            {
                image_save_preprocess = image_result.copy(0,0,img_width,img_height);
            }
            else
            {
                QVector<QRgb> table(256);
                for( int I = 0; I < 256; I++ )
                {
                    table[I] = qRgb(I,I,I);
                }
                image_save_preprocess.setColorTable(table);

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        *( image_save_preprocess.scanLine(y)+x ) = img1_filtered[ find_offset(x,y) ];
                    }
                }

            }
        }

        // marque les contours
        if( mainwindow_settings.outside_combo != 9 )
        {
            for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
            {
                offset = *it*3;

                image_result_uchar[offset] = mainwindow_settings.Rout1;
                image_result_uchar[offset+1] = mainwindow_settings.Gout1;
                image_result_uchar[offset+2] = mainwindow_settings.Bout1;
            }
        }

        if( mainwindow_settings.inside_combo != 9 )
        {
            for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
            {
                offset = *it*3;

                image_result_uchar[offset] = mainwindow_settings.Rin1;
                image_result_uchar[offset+1] = mainwindow_settings.Gin1;
                image_result_uchar[offset+2] = mainwindow_settings.Bin1;
            }
        }

        image_disp = 2;

        QApplication::processEvents();
        // Affichage
        imageLabel->set_qimage(image_result);
        QApplication::processEvents();
        imageLabel->update();
        QApplication::processEvents();

        if( hasStepByStep )
        {
            hasClickStopping = true;
        }
        infinite_loop();

        // efface les contours de la QImage
        if( !isRgb1 || mainwindow_settings.model == 2 )
        {
            if( mainwindow_settings.model == 2 && mainwindow_settings.hasHistoNormaliz )
            {
                if( mainwindow_settings.outside_combo != 9 )
                {
                    for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                    {
                        offset = *it;

                        I = (unsigned char)(255.0*double(img1_filtered[offset]-min)/double(max-min));

                        image_result_uchar[3*offset] = I;
                        image_result_uchar[3*offset+1] = I;
                        image_result_uchar[3*offset+2] = I;
                    }
                }

                if( mainwindow_settings.inside_combo != 9 )
                {
                    for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                    {
                        offset = *it;

                        I = (unsigned char)(255.0*double(img1_filtered[offset]-min)/double(max-min));

                        image_result_uchar[3*offset] = I;
                        image_result_uchar[3*offset+1] = I;
                        image_result_uchar[3*offset+2] = I;
                    }
                }
            }

            else
            {
                if( mainwindow_settings.outside_combo != 9 )
                {
                    for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                    {
                        offset = *it;

                        I = img1_filtered[offset];

                        image_result_uchar[3*offset] = I;
                        image_result_uchar[3*offset+1] = I;
                        image_result_uchar[3*offset+2] = I;
                    }
                }

                if( mainwindow_settings.inside_combo != 9 )
                {
                    for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                    {
                        offset = *it;

                        I = img1_filtered[offset];

                        image_result_uchar[3*offset] = I;
                        image_result_uchar[3*offset+1] = I;
                        image_result_uchar[3*offset+2] = I;
                    }
                }
            }
        }
        else
        {
            if( mainwindow_settings.outside_combo != 9 )
            {
                for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                {
                    offset = *it*3;

                    image_result_uchar[offset] = img1_filtered[offset];
                    image_result_uchar[offset+1] = img1_filtered[offset+1];
                    image_result_uchar[offset+2] = img1_filtered[offset+2];
                }
            }

            if( mainwindow_settings.inside_combo != 9 )
            {
                for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                {
                    offset = *it*3;

                    image_result_uchar[offset] = img1_filtered[offset];
                    image_result_uchar[offset+1] = img1_filtered[offset+1];
                    image_result_uchar[offset+2] = img1_filtered[offset+2];
                }
            }
        }

        //////////////////////////////////////////////////////////////////////
        // fin de
        //////////////////////////////////////////////////////////////////////

        imageLabel->setMouseTracking(false);

        double elapsedTime;
        std::clock_t startTime, stopTime;

        statusBar()->showMessage(tr("Active contour evolving."));
        time_text->setText("<font color=green>"+tr("time = _._____ s"));

        if( mainwindow_settings.hasDisplayEach )
        {
            // Pour calculer le temps d'exécution
            startTime = std::clock();

            //////////////////////////////////////////////////////////////////////
            // Boucle do-while, évolution de l'algorithme
            //////////////////////////////////////////////////////////////////////
            while( !ac->get_isStopped() )
            {
                if( hasAlgoBreaking )
                {
                    hasAlgoBreaking = false;
                    startAct->setEnabled(true);
                    connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
                    emit signal_open();
                    return;
                }

                if( hasStepByStep )
                {
                    hasClickStopping = true;
                }

                // efface les contours de la QImage
                if( !isRgb1 || mainwindow_settings.model == 2 )
                {
                    if( mainwindow_settings.model == 2 && mainwindow_settings.hasHistoNormaliz )
                    {
                        if( mainwindow_settings.outside_combo != 9 )
                        {
                            for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                            {
                                offset = *it;

                                I = (unsigned char)(255.0*double(img1_filtered[offset]-min)/double(max-min));

                                image_result_uchar[3*offset] = I;
                                image_result_uchar[3*offset+1] = I;
                                image_result_uchar[3*offset+2] = I;
                            }
                        }

                        if( mainwindow_settings.inside_combo != 9 )
                        {
                            for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                            {
                                offset = *it;

                                I = (unsigned char)(255.0*double(img1_filtered[offset]-min)/double(max-min));

                                image_result_uchar[3*offset] = I;
                                image_result_uchar[3*offset+1] = I;
                                image_result_uchar[3*offset+2] = I;
                            }
                        }

                    }

                    else
                    {
                        if( mainwindow_settings.outside_combo != 9 )
                        {
                            for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                            {
                                offset = *it;

                                I = img1_filtered[offset];

                                image_result_uchar[3*offset] = I;
                                image_result_uchar[3*offset+1] = I;
                                image_result_uchar[3*offset+2] = I;
                            }
                        }

                        if( mainwindow_settings.inside_combo != 9 )
                        {
                            for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                            {
                                offset = *it;

                                I = img1_filtered[offset];

                                image_result_uchar[3*offset] = I;
                                image_result_uchar[3*offset+1] = I;
                                image_result_uchar[3*offset+2] = I;
                            }
                        }
                    }
                }

                else
                {
                    if( mainwindow_settings.outside_combo != 9 )
                    {
                        for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                        {
                            offset = *it*3;

                            image_result_uchar[offset] = img1_filtered[offset];
                            image_result_uchar[offset+1] = img1_filtered[offset+1];
                            image_result_uchar[offset+2] = img1_filtered[offset+2];
                        }
                    }

                    if( mainwindow_settings.inside_combo != 9 )
                    {
                        for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                        {
                            offset = *it*3;

                            image_result_uchar[offset] = img1_filtered[offset];
                            image_result_uchar[offset+1] = img1_filtered[offset+1];
                            image_result_uchar[offset+2] = img1_filtered[offset+2];
                        }
                    }
                }

                ac->evolve_one_iteration();

                // marque les contours dans la QImage
                if( mainwindow_settings.outside_combo != 9 )
                {
                    for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                    {
                        offset = *it*3;

                        image_result_uchar[offset] = mainwindow_settings.Rout1;
                        image_result_uchar[offset+1] = mainwindow_settings.Gout1;
                        image_result_uchar[offset+2] = mainwindow_settings.Bout1;
                    }
                }

                if( mainwindow_settings.inside_combo != 9 )
                {
                    for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                    {
                        offset = *it*3;

                        image_result_uchar[offset] = mainwindow_settings.Rin1;
                        image_result_uchar[offset+1] = mainwindow_settings.Gin1;
                        image_result_uchar[offset+2] = mainwindow_settings.Bin1;
                    }
                }

                // Pour afficher plusieurs fois dans une boucle
                QApplication::processEvents();
                // Affichage
                imageLabel->update();

                if( hasClickStopping )
                {
                    show_phi_list_value();
                }
                infinite_loop();
            }

            // Temps d'exécution de la boucle do-while
            stopTime = std::clock();
            elapsedTime = double(stopTime - startTime) / double(CLOCKS_PER_SEC);
        }
        // on affiche rien du tout pour evaluer le temps de calcul
        // if( !mainwindow_settings.hasDisplayEach )
        else
        {
            startTime = std::clock();
            ac->evolve();
            stopTime = std::clock();

            elapsedTime = double(stopTime - startTime) / double(CLOCKS_PER_SEC);

            // marque les contours dans la QImage
            if( mainwindow_settings.outside_combo != 9 )
            {
                for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
                {
                    offset = *it*3;

                    image_result_uchar[offset] = mainwindow_settings.Rout1;
                    image_result_uchar[offset+1] = mainwindow_settings.Gout1;
                    image_result_uchar[offset+2] = mainwindow_settings.Bout1;
                }
            }
            if( mainwindow_settings.inside_combo != 9 )
            {
                for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
                {
                    offset = *it*3;

                    image_result_uchar[offset] = mainwindow_settings.Rin1;
                    image_result_uchar[offset+1] = mainwindow_settings.Gin1;
                    image_result_uchar[offset+2] = mainwindow_settings.Bin1;
                }
            }

            // Pour afficher plusieurs fois dans une boucle
            QApplication::processEvents();
            // Affichage
            imageLabel->update();

            if( hasClickStopping )
            {
                show_phi_list_value();
            }
            infinite_loop();
        }

        time_text->setText("<font color=red>"+tr("time = ")+QString::number(elapsedTime)+" s");
        show_phi_list_value();
        statusBar()->clearMessage();

        imageLabel->setMouseTracking(true);

        statusBar()->showMessage(tr("Push the left mouse button or click on the right mouse button in the window."));
    }

    hasStepByStep = false;
    startAct->setEnabled(true);

    connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
}

// Fonction appelée quand on clique sur save
void MainWindow::saveImage()
{
    QFileInfo fi(fileName);
    QString fileName_save;
    auto mainwindow_settings = settings_window2->get_settings();

    // selection du chemin à partir d'une boîte de dialogue
    if( iteration1 == -1 )
    {
        fileName_save = QFileDialog::getSaveFileName(this,
                                                     tr("Save the image displayed by the main window"),
                                                     last_directory_used + "/" + fi.baseName(),
                                                     "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;TIFF (*.tiff);;XBM (*.xbm);;XPM (*.xpm)");
    }
    else
    {
        show_phi_list_value();

        if( image_disp == 1 )
        {
            fileName_save = QFileDialog::getSaveFileName(this,
                                                         tr("Save the image displayed by the main window"),
                                                         last_directory_used + "/" + fi.baseName() + tr("_with_phi(t=")+QString::number(iteration1)+")",
                                                         "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;TIFF (*.tiff);;XBM (*.xbm);;XPM (*.xpm)");
        }
        if( image_disp == 2 )
        {
            if( mainwindow_settings.hasGaussianNoise || mainwindow_settings.hasSaltNoise || mainwindow_settings.hasSpeckleNoise || mainwindow_settings.hasMeanFilt || mainwindow_settings.hasGaussianFilt || mainwindow_settings.hasMedianFilt || mainwindow_settings.hasAnisoDiff || mainwindow_settings.hasOpenFilt || mainwindow_settings.hasCloseFilt || mainwindow_settings.hasTophatFilt || mainwindow_settings.model == 2 )
            {

                fileName_save = QFileDialog::getSaveFileName(this,
                                                            tr("Save the image displayed by the main window"),
                                                            last_directory_used + "/" + tr("preprocessed_") + fi.baseName() + tr("_with_phi(t=")+QString::number(iteration1)+")",
                                                            "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;TIFF (*.tiff);;XBM (*.xbm);;XPM (*.xpm)");

            }
            else
            {
                fileName_save = QFileDialog::getSaveFileName(this,
                                                             tr("Save the image displayed by the main window"),
                                                             last_directory_used + "/" + fi.baseName()+tr("_with_phi(t=") + QString::number(iteration1)+")",
                                                             "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;TIFF (*.tiff);;XBM (*.xbm);;XPM (*.xpm)");
            }
        }
    }

    QImage img_displayed = imageLabel->get_qimage();

    if( !fileName_save.isEmpty() )
    {
        QFileInfo fi1(fileName_save);
        last_directory_used = fi1.absolutePath();


        img_displayed.save(fileName_save);
    }

    if( (mainwindow_settings.hasGaussianNoise || mainwindow_settings.hasSaltNoise || mainwindow_settings.hasSpeckleNoise || mainwindow_settings.hasMeanFilt || mainwindow_settings.hasGaussianFilt || mainwindow_settings.hasMedianFilt || mainwindow_settings.hasAnisoDiff || mainwindow_settings.hasOpenFilt || mainwindow_settings.hasCloseFilt || mainwindow_settings.hasTophatFilt || mainwindow_settings.model == 2) && !image_save_preprocess.isNull() )
    {
        // sélection du chemin+nom de sauvegarde à partir d'une boîte de dialogue
        QString fileName_save2 = QFileDialog::getSaveFileName(this,
                                                              tr("Save the preprocessed image"),
                                                              last_directory_used + "/" + tr("preprocessed_") + fi.baseName(),
                                                              "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;TIFF (*.tiff);;XBM (*.xbm);;XPM (*.xpm)");


        if( !fileName_save2.isEmpty() )
        {
            QFileInfo fi2(fileName_save2);
            last_directory_used = fi2.absolutePath();

            image_save_preprocess.save(fileName_save2);
        }
    }
}

void MainWindow::print()
//! [5] //! [6]
{
    Q_ASSERT(imageLabel->pixmap());
#ifndef QT_NO_PRINTER
//! [6] //! [7]
    QPrintDialog dialog(&printer, this);
//! [7] //! [8]
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

// Fonction appelée lors de l'ouverture de la fenêtre de configuration et valider ou refuser les changements
void MainWindow::settings()
{
    settings_window2->update_visu();

    // on s'assure que la fenêtre de configuration n'est pas déja affichée
    if( settings_window2->isVisible() )
    {
        QMessageBox::critical(this, tr("Error"), tr("Close settings window before."));
    }
    else
    {
        if( settings_window2->exec() == QDialog::Accepted )
        {
            // Ok = les variables vont prendre leur valeur en fonction de l'état des widgets de la fenêtre de configuration
            settings_window2->apply_settings();

            if( camera_window->isVisible() )
            {
                camera_window->restart();
            }
        }
        else
        {
            // Annuler = les widgets vont reprendre leur état en fonction des valeurs des variables
            settings_window2->cancel_settings();
        }
    }
}

void MainWindow::zoomIn()
{
    scale_spin0->setValue( int(100.0f*imageLabel->get_zoomFactor()*1.25f) );
}

void MainWindow::zoomOut()
{
    scale_spin0->setValue( int(100.0f*imageLabel->get_zoomFactor()*0.8f) );
}

void MainWindow::normalSize()
{
    scale_spin0->setValue(100);
}

// Evénement clic souris dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( !startAct->isEnabled() )
        {
            if( event->button() == Qt::LeftButton )
            {
                statusBar()->showMessage(tr("Release the left mouse button to evolve the active contour."));
                hasStepByStep = false;
                hasClickStopping = true;
            }

            if( event->button() == Qt::RightButton )
            {
                hasClickStopping = false;
                hasStepByStep = true;
            }
        }
        else
        {
            if( event->button() == Qt::LeftButton )
            {
                statusBar()->showMessage(tr("Release the left mouse button to evolve the active contour."));
                hasStepByStep = false;
                hasClickStopping = true;
                emit( startAct->trigger() );
            }

            if( event->button() == Qt::RightButton )
            {
                hasStepByStep = true;
                emit( startAct->trigger() );
            }
        }
    }
}

// Evénement clic souris dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( event->button() == Qt::LeftButton )
        {
            statusBar()->showMessage(tr("Active contour evolving."));
            if( !startAct->isEnabled() )
            {
                hasStepByStep = false;
                hasClickStopping = false;
            }
        }
    }
}

// Evénement touche clavier enfoncé dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( !startAct->isEnabled() )
        {
            if( !event->isAutoRepeat() )
            {
                if( event->key() == Qt::Key_Space || event->key() == Qt::Key_Right || event->key() == Qt::Key_Pause )
                {
                    hasStepByStep = false;
                    hasClickStopping = true;
                }
            }

            if( event->key() == Qt::Key_Return || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down )
            {
                hasClickStopping = false;
                hasStepByStep = true;
            }
        }
        else
        {
            if( !event->isAutoRepeat() )
            {
                if( event->key() == Qt::Key_Space || event->key() == Qt::Key_Right || event->key() == Qt::Key_Pause )
                {
                    hasStepByStep = false;
                    hasClickStopping = true;
                    emit( startAct->trigger() );
                }
            }

            if( event->key() == Qt::Key_Return || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down )
            {
                hasStepByStep = true;
                emit( startAct->trigger() );
            }
        }
    }
}

// Evénement touche clavier relachée dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( !event->isAutoRepeat() )
        {
            if( event->key() == Qt::Key_Space || event->key() == Qt::Key_Right || event->key() == Qt::Key_Pause )
            {
                if( !startAct->isEnabled() )
                {
                    hasStepByStep = false;
                    hasClickStopping = false;
                }
            }
        }
    }
}

// Boucle infinie dont on peut rentrer ou sortir par un clic gauche ou droite (sur imageLabel, image de la fenêtre principale) lors de l'execution de l'algorithme et permettant de voir une étape particulière ou chaque étape, au rythme voulue
void MainWindow::infinite_loop()
{
    if( hasClickStopping )
    {
        bool temp = imageLabel->hasMouseTracking();
        imageLabel->setMouseTracking(true);

        if( hasStepByStep )
        {
            statusBar()->showMessage(tr("Click on the right mouse button to evolve the contour of one iteration."));
        }

        while( hasClickStopping )
        {
            QApplication::processEvents();
        }

        imageLabel->setMouseTracking(temp);
    }
}

// Evénement déplacement souris au niveau de imageLabel pour pouvoir donner les infos pixels
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        positionX = int(double(img_width)
                        * double( ( (event->pos()).x() - imageLabel->get_xoffset() )
                                  / double( imageLabel->getPixWidth() )
                                  )
                        );

        positionY = int(double(img_height)
                        * double( (  (event->pos()).y() - imageLabel->get_yoffset() )
                                  / double(imageLabel->getPixHeight() )
                                  )
                        );

        if( positionX < img_width && positionY < img_height )
        {
            if( image_disp == 1 )
            {
                if( !isRgb1 )
                {
                    int I = img1[ positionX+positionY*img_width ];
                    QColor rgb(I,I,I);
                    pixel_text->setText(tr("img(")+QString::number(positionX)+","+QString::number(positionY)+") = "+"<font color="+rgb.name()+">"+QString::number(I));

                }
                else
                {
                    QColor rgb(img1[ 3*(positionX+positionY*img_width) ], img1[ 3*(positionX+positionY*img_width)+1 ], img1[ 3*(positionX+positionY*img_width)+2 ]);
                    pixel_text->setText(tr("img(")+QString::number(positionX)+","+QString::number(positionY)+") = "+"<font color="+rgb.name()+">"+"("+QString::number(img1[ 3*(positionX+positionY*img_width) ])+","+QString::number(img1[ 3*(positionX+positionY*img_width)+1 ])+","+QString::number(img1[ 3*(positionX+positionY*img_width)+2 ])+")");
                }
            }

            if( image_disp == 2 )
            {
                auto mainwindow_settings = settings_window2->get_settings();

                const unsigned char* img1_filtered;

                QString img_str;
                if( mainwindow_settings.hasGaussianNoise || mainwindow_settings.hasSaltNoise || mainwindow_settings.hasSpeckleNoise || mainwindow_settings.hasMeanFilt || mainwindow_settings.hasGaussianFilt || mainwindow_settings.hasMedianFilt || mainwindow_settings.hasAnisoDiff || mainwindow_settings.hasOpenFilt || mainwindow_settings.hasCloseFilt || mainwindow_settings.hasTophatFilt || mainwindow_settings.model == 2 )
                {
                    img_str = tr("img'(");
                }
                else
                {
                    img_str = tr("img(");
                }

                if( !isRgb1 )
                {
                    img1_filtered = filters1->get_filtered();
                    int I = img1_filtered[positionX+positionY*img_width];
                    QColor rgb(I,I,I);
                    pixel_text->setText(img_str+QString::number(positionX)+","+QString::number(positionY)+") = "+"<font color="+rgb.name()+">"+QString::number(I));
                }

                else
                {
                    if( mainwindow_settings.model == 1 )
                    {
                        img1_filtered = filters1->get_filtered();

                        QColor rgb(img1_filtered[3*(positionX+positionY*img_width)],img1_filtered[3*(positionX+positionY*img_width)+1],img1_filtered[3*(positionX+positionY*img_width)+2]);

                        pixel_text->setText(img_str+QString::number(positionX)+","+QString::number(positionY)+") = "+"<font color="+rgb.name()+">"+"("+QString::number(img1_filtered[3*(positionX+positionY*img_width)])+","+QString::number(img1_filtered[3*(positionX+positionY*img_width)+1])+","+QString::number(img1_filtered[3*(positionX+positionY*img_width)+2])+")");
                    }
                    if( mainwindow_settings.model == 2 )
                    {
                        img1_filtered = filters1->get_gradient();

                        int I = img1_filtered[ positionX+positionY*img_width ];
                        QColor rgb(I,I,I);
                        pixel_text->setText(img_str+QString::number(positionX)+","+QString::number(positionY)+") = "+"<font color="+rgb.name()+">"+QString::number(I));
                    }
                }
            }
            show_phi_list_value();
        }
    }

}

void MainWindow::show_phi_list_value()
{
    QColor RGBout, RGBin;
    int Cout1, Cin1;
    const int* Cout_RGB;
    const int* Cin_RGB;
    auto mainwindow_settings = settings_window2->get_settings();
    QColor RGBout_list(mainwindow_settings.Rout1,mainwindow_settings.Gout1,mainwindow_settings.Bout1);
    QColor RGBin_list(mainwindow_settings.Rin1,mainwindow_settings.Gin1,mainwindow_settings.Bin1);

    QString Cout_str = QString("<font color="+RGBout_list.name()+">"+"Cout"+"<font color=black>"+" = ");
    QString Cin_str = QString("<font color="+RGBin_list.name()+">"+"Cin"+"<font color=black>"+" = ");

    unsigned char otsu_threshold1;
    unsigned int iteration_max1;

    if( ac != nullptr )
    {
        iteration1 = ac->get_iteration();
        iteration_max1 = ac->get_iteration_max();

        if( iteration1 < iteration_max1 )
        {
            text->setText("<font color=green>"+tr("iteration t = ")+QString::number(iteration1));
        }
        else
        {
            text->setText("<font color=red>"+tr("iteration t = ")+QString::number(iteration1));
        }

        if( ac->get_hasListsChanges() )
        {
            changes_text->setText("<font color=green>"+tr("lists changes = true"));
        }
        else
        {
            changes_text->setText("<font color=red>"+tr("lists changes = false"));
        }

        if( ac->get_hasOscillation() )
        {
            oscillation_text->setText("<font color=red>"+tr("oscillation = true"));
        }
        else
        {
            oscillation_text->setText("<font color=green>"+tr("oscillation = false"));
        }


        if( mainwindow_settings.model == 1 )
        {
            if( !isRgb1 )
            {
                // pour l'affichage
                if( dynamic_cast<ofeli::RegionBasedActiveContour*>(ac) != nullptr )
                {
                    Cout1 = dynamic_cast<ofeli::RegionBasedActiveContour*>(ac)->get_Cout();
                    Cin1 = dynamic_cast<ofeli::RegionBasedActiveContour*>(ac)->get_Cin();

                    RGBout = QColor(Cout1,Cout1,Cout1);
                    RGBin = QColor(Cin1,Cin1,Cin1);

                    Cout_text->setText(Cout_str+"<font color="+RGBout.name()+">"+QString::number(Cout1));
                    Cin_text->setText(Cin_str+"<font color="+RGBin.name()+">"+QString::number(Cin1));
                }
            }
            else
            {
                if( dynamic_cast<ofeli::RegionBasedActiveContourYUV*>(ac) != nullptr )
                {
                    Cout_RGB = dynamic_cast<ofeli::RegionBasedActiveContourYUV*>(ac)->get_Cout_RGB();
                    Cin_RGB = dynamic_cast<ofeli::RegionBasedActiveContourYUV*>(ac)->get_Cin_RGB();

                    RGBout = QColor(Cout_RGB[0],Cout_RGB[1],Cout_RGB[2]);
                    RGBin = QColor(Cin_RGB[0],Cin_RGB[1],Cin_RGB[2]);

                    Cout_text->setText(Cout_str+"<font color="+RGBout.name()+">"+"("+QString::number(Cout_RGB[0])+","+QString::number(Cout_RGB[1])+","+QString::number(Cout_RGB[2])+")");
                    Cin_text->setText(Cin_str+"<font color="+RGBin.name()+">"+"("+QString::number(Cin_RGB[0])+","+QString::number(Cin_RGB[1])+","+QString::number(Cin_RGB[2])+")");
                }
            }
        }

        if( mainwindow_settings.model == 2 )
        {
            if( dynamic_cast<ofeli::EdgeBasedActiveContour*>(ac) != nullptr )
            {
                // pour l'affichage
                otsu_threshold1 = dynamic_cast<ofeli::EdgeBasedActiveContour*>(ac)->get_otsu_threshold();
                threshold_text->setText(tr("Otsu's threshold of gradient = ")+QString::number(otsu_threshold1));
            }
        }

        const ofeli::Matrix<char>& phi = ac->get_phi();
        if( !phi.isNull() )
        {
            QColor RGBout(mainwindow_settings.Rout1,mainwindow_settings.Gout1,mainwindow_settings.Bout1);
            QColor RGBin(mainwindow_settings.Rin1,mainwindow_settings.Gin1,mainwindow_settings.Bin1);

            if( phi(positionX,positionY) == 3 )
            {
                phi_text->setText("ϕ(x,y,t) = <font color="+RGBout.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∉ <font color="+RGBout.name()+">"+"Lout"+"<font color=black>"+"(t) & ∉ "+"<font color="+RGBin.name()+">"+"Lin"+"<font color=black>"+"(t)");
            }
            if( phi(positionX,positionY) == -3 )
            {
                phi_text->setText("ϕ(x,y,t) = <font color="+RGBin.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∉ <font color="+RGBin.name()+">"+"Lin"+"<font color=black>"+"(t) & ∉ "+"<font color="+RGBout.name()+">"+"Lout"+"<font color=black>"+"(t)");
            }
            if( phi(positionX,positionY) == 1 )
            {
                phi_text->setText("ϕ(x,y,t) = <font color="+RGBout.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∈ <font color="+RGBout.name()+">"+"Lout"+"<font color=black>"+"(t)");
            }
            if( phi(positionX,positionY) == -1 )
            {
                phi_text->setText("ϕ(x,y,t) = <font color="+RGBin.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∈ <font color="+RGBin.name()+">"+"Lin"+"<font color=black>"+"(t)");
            }
        }
    }
}

// Pour arrêter manuellement l'algorithme, s'il y a un problème au niveau des conditions d'arrêts ou du temps de calcul sur les grandes images
void MainWindow::stop()
{
    hasClickStopping = false;
    hasAlgoBreaking = true;
}

// Filtre des événements pour avoir le tracking au niveau du widget image de la fenêtre principale et de la fenêtre de parametres et pour ne pas avoir le tracking/la position au niveau de l'ensemble de chaque fenêtre
bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    // deplacement uniquement en fonction de imageLabel et pas main_window
    if( object == imageLabel && event->type() == QEvent::MouseMove )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseEvent);
    }

    if( (object == scale_spin0 || object == scale_slider0) && event->type() == QEvent::MouseButtonPress )
    {
        if( img1 != nullptr )
        {
            imageLabel->set_hasText(false);
            imageLabel->setBackgroundRole(QPalette::Dark);
        }
        positionX = img_width/2;
        positionY = img_height/2;
    }

    if( object == imageLabel && event->type() == QEvent::DragEnter )
    {
        QDragEnterEvent* drag = static_cast<QDragEnterEvent*>(event);
        dragEnterEvent(drag);
    }
    if( object == imageLabel && event->type() == QEvent::DragMove )
    {
        QDragMoveEvent* drag = static_cast<QDragMoveEvent*>(event);
        dragMoveEvent(drag);
    }
    if( object == imageLabel && event->type() == QEvent::Drop )
    {
        QDropEvent* drag = static_cast<QDropEvent*>(event);
        dropEvent(drag);
    }
    if( object == imageLabel && event->type() == QEvent::DragLeave )
    {
        QDragLeaveEvent* drag = static_cast<QDragLeaveEvent*>(event);
        dragLeaveEvent(drag);
    }

    if( object == imageLabel && event->type() == QEvent::MouseMove && positionX < img_width && positionY < img_height )
    {
        setCursor(Qt::CrossCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
    return false;
}

// Fonction appelée pour le changement d'échelle dans la fenêtre viewer
void MainWindow::do_scale0(int value)
{
    if( img1 != nullptr )
    {
        imageLabel->setZoomFactor(float(value)/100.0);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if( !settings_window2->isVisible() )
    {
        QString text(tr("<drop image>"));
        imageLabel->set_text(text);
        imageLabel->setBackgroundRole(QPalette::Highlight);
        imageLabel->set_hasText(true);

        event->acceptProposedAction();
        emit changed(event->mimeData());
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    if( !settings_window2->isVisible() )
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if( !settings_window2->isVisible() )
    {
        const QMimeData* mimeData = event->mimeData();

        if( mimeData->hasUrls() )
        {
            QList<QUrl> urlList = mimeData->urls();
            fileName = urlList.first().toLocalFile();
        }
        imageLabel->setBackgroundRole(QPalette::Dark);
        emit signal_open();
        event->acceptProposedAction();
    }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    if( !settings_window2->isVisible() )
    {
        QString text(tr("<drag image>"));
        imageLabel->set_text(text);
        imageLabel->setBackgroundRole(QPalette::Dark);
        imageLabel->set_hasText(true);
        emit changed();
        event->accept();
    }
}

void MainWindow::adjustVerticalScroll(int min, int max)
{
    if( img_height != 0 )
    {
        scrollArea->verticalScrollBar()->setValue( (max-min)*positionY/img_height );
    }
}

void MainWindow::adjustHorizontalScroll(int min, int max)
{
    if( img_width != 0 )
    {
        scrollArea->horizontalScrollBar()->setValue( (max-min)*positionX/img_width );
    }
}

void MainWindow::language()
{
    if( language_window->exec() == QDialog::Accepted )
    {
        language_window->apply_setting();
    }
    else
    {
        language_window->cancel_setting();
    }
}

void MainWindow::doc()
{
    QDesktopServices::openUrl( QUrl("http://ofeli.googlecode.com/svn/doc/index.html", QUrl::TolerantMode) );
}

void MainWindow::setCurrentFile(const QString &fileName1)
{
    curFile = fileName1;
    //setWindowFilePath(curFile);

    QSettings settings("Bessy", "Ofeli");
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();
    files.removeAll(fileName1);
    files.prepend(fileName1);
    while( files.size() > MaxRecentFiles )
    {
        files.removeLast();
    }

    settings.setValue("Main/Name/recentFileList", files);

    foreach( QWidget* widget, QApplication::topLevelWidgets() )
    {
        MainWindow* mainWindow = qobject_cast<MainWindow*>(widget);
        if( mainWindow != nullptr )
        {
            mainWindow->updateRecentFileActions();
        }
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings("Bessy", "Ofeli");
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for( int i = 0; i < numRecentFiles; ++i )
    {
        QString text = tr("&%1").arg( strippedName(files[i]) );
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
        recentFileActs[i]->setStatusTip(files[i]);
    }

    for( int j = numRecentFiles; j < MaxRecentFiles; ++j )
    {
        recentFileActs[j]->setVisible(false);
    }

    separatorAct->setVisible(numRecentFiles > 0);

    if( files.isEmpty() )
    {
        deleteAct->setVisible(false);
    }
    else
    {
        deleteAct->setVisible(true);
    }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>( sender() );
    if( action != nullptr )
    {
        fileName = action->data().toString();
    }

    emit signal_open();
}

void MainWindow::deleteList()
{
    QStringList files;
    files.clear();

    QSettings settings("Bessy", "Ofeli");
    settings.setValue("Main/Name/recentFileList", files);

    updateRecentFileActions();
}

void MainWindow::wheel_zoom(int val, ofeli::ScrollAreaWidget* obj)
{
    if( obj == scrollArea && img1 != nullptr )
    {
        imageLabel->set_hasText(false);
        imageLabel->setBackgroundRole(QPalette::Dark);

        float value = 0.002f*float( val ) + imageLabel->get_zoomFactor();

        if( value < 32.0f/float( imageLabel->get_qimage().width() ) )
        {
            value = 32.0f/float( imageLabel->get_qimage().width() );
        }

        scale_spin0->setValue( int(100.0f*value) );
    }
}

void MainWindow::set_zoom_factor(int val)
{
    scale_spin0->setValue(val);
}

int MainWindow::get_zoom_factor() const
{
    return scale_spin0->value();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue( "Main/Window/size", size() );
    settings.setValue( "Main/Window/position", pos() );
    settings.setValue("Main/Name/last_directory_used", last_directory_used);

    settings_window2->save_settings();
    evaluation_window->save_settings();
    camera_window->save_settings();
    about_window->save_settings();
    language_window->save_settings();

    event->accept();
}

}
