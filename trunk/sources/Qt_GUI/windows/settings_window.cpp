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

#include "settings_window.hpp"
#include "main_window.hpp"
#include "filters.hpp"
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"
#include <QtWidgets>
#include <ctime>         // for std::clock_t, std::clock() and CLOCKS_PER_SEC
#include <cstring>       // for std::memcpy

namespace ofeli
{

SettingsWindow::SettingsWindow(QWidget* parent) :
    QDialog(parent),
    filters2(nullptr), phi_init2(nullptr), phi_init1_clean(nullptr), phi_init2_clean(nullptr),
    shape(nullptr), shape_points(nullptr), Lout_shape1(nullptr), Lin_shape1(nullptr), Lout_2(nullptr), Lin_2(nullptr),
    image_filter_uchar(nullptr), image_phi_uchar(nullptr), image_shape_uchar(nullptr), img1(nullptr)
{
    setWindowTitle(tr("Settings"));

    QSettings settings("Bessy", "Ofeli");

    switch( operating_system )
    {
    case 1 :
    {
        resize(settings.value("Settings/Window/size",
                              QSize(550+335,550+55)).toSize());
        move(settings.value("Settings/Window/position",
                            QPoint(200, 200)).toPoint());
        break;
    }
    case 2 :
    {
        resize(settings.value("Settings/Window/size",
                              QSize(550+477,550+82)).toSize());
        move(settings.value("Settings/Window/position",
                            QPoint(200, 200)).toPoint());
        break;
    }
    case 3 :
    {
        resize(settings.value("Settings/Window/size",
                              QSize(550+327,550+60)).toSize());
        move(settings.value("Settings/Window/position",
                            QPoint(200, 200)).toPoint());
    }
    }

    scrollArea_settings = new ofeli::ScrollAreaWidget(this);
    imageLabel_settings = new ofeli::PixmapWidget(scrollArea_settings);
    imageLabel_settings->setBackgroundRole(QPalette::Dark);
    imageLabel_settings->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel_settings->setScaledContents(true);
    imageLabel_settings->setMouseTracking(true);
    imageLabel_settings->installEventFilter(this);
    imageLabel_settings->setAcceptDrops(true);
    imageLabel_settings->setAlignment(Qt::AlignCenter);
    QString text(tr("<drag ϕ(t=0)>"));
    imageLabel_settings->set_text(text);
    imageLabel_settings->resize(200,200);
    scrollArea_settings->setBackgroundRole(QPalette::Dark);
    scrollArea_settings->setWidget(imageLabel_settings);
    scrollArea_settings->setAlignment(Qt::AlignCenter);
    scrollArea_settings->setWidgetResizable(true);
    connect(scrollArea_settings->verticalScrollBar(),
            SIGNAL(rangeChanged(int,int)),this,
            SLOT(adjustVerticalScroll_settings (int,int)));
    connect(scrollArea_settings->horizontalScrollBar(),
            SIGNAL(rangeChanged(int,int)),this,
            SLOT(adjustHorizontalScroll_settings(int,int)));

    dial_buttons = new QDialogButtonBox(this);
    dial_buttons->addButton(QDialogButtonBox::Ok);
    dial_buttons->addButton(QDialogButtonBox::Cancel);
    dial_buttons->addButton(QDialogButtonBox::Reset);
    connect( dial_buttons, SIGNAL(accepted()), this, SLOT(accept()) );
    connect( dial_buttons, SIGNAL(rejected()), this, SLOT(reject()) );
    connect( dial_buttons->button(QDialogButtonBox::Reset),
             SIGNAL(clicked()), this, SLOT(default_settings()) );

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Algorithm tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* externalspeed_groupbox = new QGroupBox(tr("Cycle 1 : data dependant evolution"));

    Na_spin = new QSpinBox;
    Na_spin->setSingleStep(1);
    Na_spin->setMinimum(1);
    Na_spin->setMaximum(999);
    Na_spin->setSuffix(tr(" iterations"));
    Na_spin->setToolTip(tr("iterations in the cycle 1, active contour penetrability"));
    QFormLayout *Na_layout = new QFormLayout;
    Na_layout->addRow("Na =", Na_spin);
    if( operating_system == 2 )
    {
        Na_layout->setFormAlignment(Qt::AlignLeft);
    }

    klength_gradient_spin = new QSpinBox;
    klength_gradient_spin->setSingleStep(2);
    klength_gradient_spin->setMinimum(1);
    klength_gradient_spin->setMaximum(499);

    chanvese_radio = new QRadioButton(tr("Chan-Vese model"));
    chanvese_radio->setToolTip(tr("region-based model for bimodal images"));

    lambda_out_spin = new QSpinBox;
    lambda_out_spin->setSingleStep(1);
    lambda_out_spin->setMinimum(0);
    lambda_out_spin->setMaximum(100000);
    lambda_out_spin->setToolTip(tr("weight of the outside homogeneity criterion"));

    lambda_in_spin = new QSpinBox;
    lambda_in_spin->setSingleStep(1);
    lambda_in_spin->setMinimum(0);
    lambda_in_spin->setMaximum(100000);
    lambda_in_spin->setToolTip(tr("weight of the inside homogeneity criterion"));

    QFormLayout* lambda_layout = new QFormLayout;

    QColor RGBout_list(mainwindow_settings.Rout1,
                       mainwindow_settings.Gout1,
                       mainwindow_settings.Bout1);

    QColor RGBin_list(mainwindow_settings.Rin1,
                      mainwindow_settings.Gin1,
                      mainwindow_settings.Bin1);

    lambda_layout->addRow("<font color="+RGBout_list.name()+">"+"λout"+"<font color=black>"+" =", lambda_out_spin);
    lambda_layout->addRow("<font color="+RGBin_list.name()+">"+"λin"+"<font color=black>"+" =", lambda_in_spin);
    if( operating_system == 2 )
    {
        lambda_layout->setFormAlignment(Qt::AlignLeft);
    }

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addWidget(chanvese_radio);
    chanvese_layout->addLayout(lambda_layout);

    geodesic_radio = new QRadioButton(tr("geodesic model"));
    geodesic_radio->setToolTip(tr("edge-based model for smoothed multimodal images"));

    klength_gradient_spin = new QSpinBox;
    klength_gradient_spin->setSingleStep(2);
    klength_gradient_spin->setMinimum(1);
    klength_gradient_spin->setMaximum(499);
    klength_gradient_spin->setSuffix("²");
    klength_gradient_spin->setToolTip(tr("morphological gradient structuring element size"));
    QFormLayout *gradient_layout = new QFormLayout;
    gradient_layout->addRow(tr("SE size ="), klength_gradient_spin);
    if( operating_system == 2 )
    {
        gradient_layout->setFormAlignment(Qt::AlignLeft);
    }
    connect(klength_gradient_spin,SIGNAL(valueChanged(int)),this,
            SLOT(filtering_visu()));

    QVBoxLayout* geodesic_layout = new QVBoxLayout;
    geodesic_layout->addWidget(geodesic_radio);
    geodesic_layout->addLayout(gradient_layout);

    QHBoxLayout* speed_layout = new QHBoxLayout;
    speed_layout->addLayout(chanvese_layout);
    speed_layout->addLayout(geodesic_layout);

    yuv_groupbox = new QGroupBox(tr("normalized (Y,U,V) color space weighting"));
    yuv_groupbox->setFlat(true);

    Yspin = new QSpinBox;
    Yspin->setSingleStep(1);
    Yspin->setMinimum(0);
    Yspin->setMaximum(100000);
    Yspin->setToolTip(tr("luminance weight"));

    Uspin = new QSpinBox;
    Uspin->setSingleStep(1);
    Uspin->setMinimum(0);
    Uspin->setMaximum(100000);
    Uspin->setToolTip(tr("chrominance weight"));

    Vspin = new QSpinBox;
    Vspin->setSingleStep(1);
    Vspin->setMinimum(0);
    Vspin->setMaximum(100000);
    Vspin->setToolTip(tr("chrominance weight"));

    connect(Yspin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    connect(Uspin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    connect(Vspin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));

    QFormLayout* yuv_layout = new QFormLayout;
    yuv_layout->addRow(tr("Y weight ="), Yspin);
    yuv_layout->addRow(tr("U weight ="), Uspin);
    yuv_layout->addRow(tr("V weight ="), Vspin);
    if( operating_system == 2 )
    {
        yuv_layout->setFormAlignment(Qt::AlignLeft);
    }
    yuv_groupbox->setLayout(yuv_layout);

    connect(chanvese_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(geodesic_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));

    QVBoxLayout* externalspeed_layout = new QVBoxLayout;

    externalspeed_layout->addLayout(Na_layout);
    externalspeed_layout->addLayout(speed_layout);
    externalspeed_layout->addWidget(yuv_groupbox);
    externalspeed_groupbox->setLayout(externalspeed_layout);

    ////////////////////////////////////////////

    internalspeed_groupbox = new QGroupBox(tr("Cycle 2 - smoothing via gaussian filtring"));
    internalspeed_groupbox->setCheckable(true);
    internalspeed_groupbox->setChecked(true);

    Ns_spin = new QSpinBox;
    Ns_spin->setSingleStep(1);
    Ns_spin->setMinimum(1);
    Ns_spin->setMaximum(999);
    Ns_spin->setSuffix(tr(" iterations"));
    Ns_spin->setToolTip(tr("iterations in the cycle 2, active contour regularization"));
    QFormLayout* Ns_layout = new QFormLayout;
    Ns_layout->addRow("Ns =", Ns_spin);
    if( operating_system == 2 )
    {
        Ns_layout->setFormAlignment(Qt::AlignLeft);
    }

    klength_spin = new QSpinBox;
    klength_spin->setSingleStep(2);
    klength_spin->setMinimum(1);
    klength_spin->setMaximum(499);
    klength_spin->setToolTip(tr("gaussian kernel size = Ng × Ng "));
    std_spin = new QDoubleSpinBox;
    std_spin->setSingleStep(0.1);
    std_spin->setMinimum(0.0);
    std_spin->setMaximum(1000000.0);
    std_spin->setToolTip(tr("standard deviation of the gaussian kernel"));

    QFormLayout* internalspeed_layout = new QFormLayout;
    internalspeed_layout->addRow("Ns =", Ns_spin);
    internalspeed_layout->addRow("Ng =", klength_spin);
    internalspeed_layout->addRow("σ =", std_spin);
    if( operating_system == 2 )
    {
        internalspeed_layout->setFormAlignment(Qt::AlignLeft);
    }
    internalspeed_groupbox->setLayout(internalspeed_layout);

    ////////////////////////////////////////////

    QVBoxLayout* algorithm_layout = new QVBoxLayout;
    algorithm_layout->addWidget(externalspeed_groupbox);
    algorithm_layout->addWidget(internalspeed_groupbox);
    algorithm_layout->addStretch(1);

    QWidget* page1 = new QWidget;
    page1->setLayout(algorithm_layout);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialization tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    open_phi_button = new QPushButton(tr("Open ϕ(t=0)"));
    open_phi_button->setToolTip(tr("or drag and drop an image of ϕ(t=0)"));
    save_phi_button = new QPushButton(tr("Save ϕ(t=0)"));
    connect(open_phi_button,SIGNAL(clicked()),this,SLOT(openFilenamePhi()));
    connect(save_phi_button,SIGNAL(clicked()),this,SLOT(save_phi()));
    open_phi_button->setEnabled(false);
    save_phi_button->setEnabled(false);


    QHBoxLayout* openSavePhi= new QHBoxLayout;
    openSavePhi->addWidget(open_phi_button);
    openSavePhi->addWidget(save_phi_button);

    ////////////////////////////////////////////////////

    QGroupBox* shape_groupbox = new QGroupBox(tr("Shape"));
    shape_groupbox->setFlat(true);

    rectangle_radio = new QRadioButton(tr("rectangle"));
    rectangle_radio->setToolTip(tr("or click on the middle mouse button when the cursor is in the image"));
    ellipse_radio = new QRadioButton(tr("ellipse"));
    ellipse_radio->setToolTip(tr("or click on the middle mouse button when the cursor is in the image"));
    connect(rectangle_radio,SIGNAL(clicked()),this,SLOT(shape_visu()));
    connect(ellipse_radio,SIGNAL(clicked()),this,SLOT(shape_visu()));

    QHBoxLayout* shape_layout = new QHBoxLayout;
    shape_layout->addWidget(rectangle_radio);
    shape_layout->addWidget(ellipse_radio);
    shape_groupbox->setLayout(shape_layout);

    ////////////////////////////////////////////

    QGroupBox* shape_size_groupbox = new QGroupBox(tr("Size"));
    shape_size_groupbox ->setToolTip(tr("or roll the mouse wheel when the cursor is in the image"));

    width_shape_spin = new QSpinBox;
    width_shape_spin->setSingleStep(15);
    width_shape_spin->setMinimum(0);
    width_shape_spin->setMaximum(1000);
    width_shape_spin->setSuffix(tr(" % image width"));

    QFormLayout* width_spin_layout = new QFormLayout;
    width_spin_layout->addRow(tr("width ="), width_shape_spin);
    if( operating_system == 2 )
    {
        width_spin_layout->setFormAlignment(Qt::AlignLeft);
    }

    width_slider = new QSlider(Qt::Horizontal, this);
    if( operating_system == 3 )
    {
        width_slider->setTickPosition(QSlider::TicksBelow);
    }
    else
    {
        width_slider->setTickPosition(QSlider::TicksAbove);
    }
    width_slider->setMinimum(0);
    width_slider->setMaximum(150);
    width_slider->setTickInterval(25);
    width_slider->setSingleStep(15);

    connect(width_shape_spin,SIGNAL(valueChanged(int)),this,
            SLOT(shape_visu(int)));
    connect(width_slider,SIGNAL(valueChanged(int)),width_shape_spin,
            SLOT(setValue(int)));
    connect(width_shape_spin,SIGNAL(valueChanged(int)),width_slider,
            SLOT(setValue(int)));

    height_shape_spin = new QSpinBox;
    height_shape_spin->setSingleStep(15);
    height_shape_spin->setMinimum(0);
    height_shape_spin->setMaximum(1000);
    height_shape_spin->setSuffix(tr((" % image height")));

    QFormLayout* height_spin_layout = new QFormLayout;
    height_spin_layout->addRow(tr("height ="), height_shape_spin);
    if( operating_system == 2 )
    {
        height_spin_layout->setFormAlignment(Qt::AlignLeft);
    }

    height_slider = new QSlider(Qt::Horizontal, this);
    if( operating_system == 3 )
    {
        height_slider->setTickPosition(QSlider::TicksBelow);
    }
    else
    {
        height_slider->setTickPosition(QSlider::TicksAbove);
    }
    height_slider->setMinimum(0);
    height_slider->setMaximum(150);
    height_slider->setTickInterval(25);
    height_slider->setSingleStep(15);

    connect(height_shape_spin,SIGNAL(valueChanged(int)),this,
            SLOT(shape_visu(int)));
    connect(height_slider,SIGNAL(valueChanged(int)),height_shape_spin,
            SLOT(setValue(int)));
    connect(height_shape_spin,SIGNAL(valueChanged(int)),height_slider,
            SLOT(setValue(int)));

    QVBoxLayout* shape_size_layout = new QVBoxLayout;
    shape_size_layout->addLayout(width_spin_layout);
    shape_size_layout->addWidget(width_slider);
    shape_size_layout->addLayout(height_spin_layout);
    shape_size_layout->addWidget(height_slider);
    shape_size_groupbox->setLayout(shape_size_layout);

    ////////////////////////////////////////////

    QGroupBox* position_groupbox = new QGroupBox(tr("Position (x,y)"));
    position_groupbox->setToolTip(tr("or move the mouse cursor in the image"));

    abscissa_spin = new QSpinBox;
    abscissa_spin->setSingleStep(15);
    abscissa_spin->setMinimum(-500);
    abscissa_spin->setMaximum(500);
    abscissa_spin->setSuffix(tr(" % image width"));

    QFormLayout* abscissa_spin_layout = new QFormLayout;
    abscissa_spin_layout->addRow("x = Xo +", abscissa_spin);
    if( operating_system == 2 )
    {
        abscissa_spin_layout->setFormAlignment(Qt::AlignLeft);
    }

    abscissa_slider = new QSlider(Qt::Horizontal, this);
    if( operating_system == 3 )
    {
        abscissa_slider->setTickPosition(QSlider::TicksBelow);
    }
    else
    {
        abscissa_slider->setTickPosition(QSlider::TicksAbove);
    }
    abscissa_slider->setMinimum(-75);
    abscissa_slider->setMaximum(75);
    abscissa_slider->setTickInterval(25);
    abscissa_slider->setSingleStep(15);

    connect(abscissa_spin,SIGNAL(valueChanged(int)),this,SLOT(shape_visu(int)));
    connect(abscissa_slider,SIGNAL(valueChanged(int)),abscissa_spin,SLOT(setValue(int)));
    connect(abscissa_spin,SIGNAL(valueChanged(int)),abscissa_slider,SLOT(setValue(int)));

    ordinate_spin = new QSpinBox;
    ordinate_spin->setSingleStep(15);
    ordinate_spin->setMinimum(-500);
    ordinate_spin->setMaximum(500);
    ordinate_spin->setSuffix(tr(" % image height"));

    QFormLayout* ordinate_spin_layout = new QFormLayout;
    ordinate_spin_layout->addRow("y = Yo +", ordinate_spin);
    if( operating_system == 2 )
    {
        ordinate_spin_layout->setFormAlignment(Qt::AlignLeft);
    }

    ordinate_slider = new QSlider(Qt::Horizontal, this);
    if( operating_system == 3 )
    {
        ordinate_slider->setTickPosition(QSlider::TicksBelow);
    }
    else
    {
        ordinate_slider->setTickPosition(QSlider::TicksAbove);
    }
    ordinate_slider->setMinimum(-75);
    ordinate_slider->setMaximum(75);
    ordinate_slider->setTickInterval(25);
    ordinate_slider->setSingleStep(15);

    connect(ordinate_spin,SIGNAL(valueChanged(int)),this,SLOT(shape_visu(int)));
    connect(ordinate_slider,SIGNAL(valueChanged(int)),ordinate_spin,SLOT(setValue(int)));
    connect(ordinate_spin,SIGNAL(valueChanged(int)),ordinate_slider,SLOT(setValue(int)));

    QVBoxLayout* position_layout = new QVBoxLayout;
    position_layout->addLayout(abscissa_spin_layout);
    position_layout->addWidget(abscissa_slider);
    position_layout->addLayout(ordinate_spin_layout);
    position_layout->addWidget(ordinate_slider);
    position_groupbox->setLayout(position_layout);

    ////////////////////////////////////////////

    QGroupBox* modify_groupbox = new QGroupBox(tr("Active contour modification"));
    modify_groupbox->setFlat(true);

    add_button = new QPushButton(tr("Add"));
    add_button->setToolTip(tr("or click on the left mouse button when the cursor is in the image"));
    connect(add_button,SIGNAL(clicked()),this,SLOT(add_visu()));

    subtract_button = new QPushButton(tr("Subtract"));
    subtract_button->setToolTip(tr("or click on the right mouse button when the cursor is in the image"));
    connect(subtract_button,SIGNAL(clicked()),this,SLOT(subtract_visu()));

    clean_button = new QPushButton(tr("Clean"));
    connect(clean_button,SIGNAL(clicked()),this,SLOT(clean_phi_visu()));

    QHBoxLayout* modify_layout = new QHBoxLayout;
    modify_layout->addWidget(clean_button);
    modify_layout->addWidget(subtract_button);
    modify_layout->addWidget(add_button);
    modify_groupbox->setLayout(modify_layout);

    ////////////////////////////////////////////

    QVBoxLayout* initialization_layout = new QVBoxLayout;
    initialization_layout->addLayout(openSavePhi);
    initialization_layout->addWidget(shape_groupbox);
    initialization_layout->addWidget(shape_size_groupbox);
    initialization_layout->addWidget(position_groupbox);
    initialization_layout->addWidget(modify_groupbox);
    initialization_layout->addStretch(1);

    QWidget* page2 = new QWidget;
    page2->setLayout(initialization_layout);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Preprocessing tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    gaussian_noise_groupbox = new QGroupBox(tr("Gaussian white noise"));
    gaussian_noise_groupbox->setCheckable(true);
    gaussian_noise_groupbox->setChecked(false);
    std_noise_spin = new QDoubleSpinBox;
    std_noise_spin->setSingleStep(5.0);
    std_noise_spin->setMinimum(0.0);
    std_noise_spin->setMaximum(10000.0);
    std_noise_spin->setToolTip(tr("standard deviation"));
    connect(gaussian_noise_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(std_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(filtering_visu()));

    QFormLayout* gaussian_noise_layout = new QFormLayout;
    gaussian_noise_layout->addRow("σ =", std_noise_spin);

    salt_noise_groupbox = new QGroupBox(tr("Impulsional noise (salt and pepper)"));
    salt_noise_groupbox->setCheckable(true);
    salt_noise_groupbox->setChecked(false);
    proba_noise_spin = new QDoubleSpinBox;
    proba_noise_spin->setSingleStep(1.0);
    proba_noise_spin->setMinimum(0.0);
    proba_noise_spin->setMaximum(100.0);
    proba_noise_spin->setSuffix(" %");
    proba_noise_spin->setToolTip(tr("impulsional noise probability for each pixel"));
    connect(salt_noise_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(proba_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(filtering_visu()));

    QFormLayout* salt_noise_layout = new QFormLayout;
    salt_noise_layout->addRow(tr("d ="), proba_noise_spin);

    speckle_noise_groupbox = new QGroupBox(tr("Speckle noise"));
    speckle_noise_groupbox->setCheckable(true);
    speckle_noise_groupbox->setChecked(false);
    std_speckle_noise_spin = new QDoubleSpinBox;
    std_speckle_noise_spin->setSingleStep(0.02);
    std_speckle_noise_spin->setMinimum(0.0);
    std_speckle_noise_spin->setMaximum(1000.0);
    std_speckle_noise_spin->setToolTip(tr("standard deviation"));
    connect(speckle_noise_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(std_speckle_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(filtering_visu()));

    QFormLayout* speckle_noise_layout = new QFormLayout;
    speckle_noise_layout->addRow("σ =", std_speckle_noise_spin);

    if( operating_system == 2 )
    {
        gaussian_noise_layout->setFormAlignment(Qt::AlignLeft);
        salt_noise_layout->setFormAlignment(Qt::AlignLeft);
        speckle_noise_layout->setFormAlignment(Qt::AlignLeft);
    }

    gaussian_noise_groupbox->setLayout(gaussian_noise_layout);
    salt_noise_groupbox->setLayout(salt_noise_layout);
    speckle_noise_groupbox->setLayout(speckle_noise_layout);

    QVBoxLayout* noise_layout = new QVBoxLayout;
    noise_layout->addWidget(gaussian_noise_groupbox);
    noise_layout->addWidget(salt_noise_groupbox);
    noise_layout->addWidget(speckle_noise_groupbox);
    noise_layout->addStretch(1);

    ////////////////////////////////////////////

    mean_groupbox = new QGroupBox(tr("Mean filter"));
    mean_groupbox->setCheckable(true);
    mean_groupbox->setChecked(false);
    klength_mean_spin = new QSpinBox;
    klength_mean_spin->setSingleStep(2);
    klength_mean_spin->setMinimum(1);
    klength_mean_spin->setMaximum(499);
    klength_mean_spin->setSuffix("²");
    connect(mean_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(klength_mean_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    QFormLayout* mean_layout = new QFormLayout;
    mean_layout->addRow(tr("kernel size ="), klength_mean_spin);

    gaussian_groupbox = new QGroupBox(tr("Gaussian filter"));
    gaussian_groupbox->setCheckable(true);
    gaussian_groupbox->setChecked(false);
    klength_gaussian_spin = new QSpinBox;
    klength_gaussian_spin->setSingleStep(2);
    klength_gaussian_spin->setMinimum(1);
    klength_gaussian_spin->setMaximum(499);
    klength_gaussian_spin->setSuffix("²");
    std_filter_spin = new QDoubleSpinBox;
    std_filter_spin->setSingleStep(0.1);
    std_filter_spin->setMinimum(0.0);
    std_filter_spin->setMaximum(1000000.0);
    std_filter_spin->setToolTip(tr("standard deviation"));
    connect(gaussian_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(klength_gaussian_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    connect(std_filter_spin,SIGNAL(valueChanged(double)),this,SLOT(filtering_visu()));
    QFormLayout* gaussian_layout = new QFormLayout;
    gaussian_layout->addRow(tr("kernel size ="), klength_gaussian_spin);
    gaussian_layout->addRow("σ =", std_filter_spin);

    ////////////////////////////////////////////

    median_groupbox = new QGroupBox(tr("Median filter"));
    median_groupbox->setCheckable(true);
    median_groupbox->setChecked(false);
    klength_median_spin = new QSpinBox;
    klength_median_spin->setSingleStep(2);
    klength_median_spin->setMinimum(1);
    klength_median_spin->setMaximum(499);
    klength_median_spin->setSuffix("²");
    complex_radio1= new QRadioButton("O(r log r)×O(n)");
    complex_radio2= new QRadioButton("O(1)×O(n)");
    connect(median_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(klength_median_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    connect(complex_radio1,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(complex_radio2,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    QFormLayout* median_layout = new QFormLayout;
    median_layout->addRow(tr("kernel size ="), klength_median_spin);
    median_layout->addRow(tr("quick sort algorithm"), complex_radio1);
    median_layout->addRow(tr("Perreault's algorithm"), complex_radio2);

    aniso_groupbox = new QGroupBox(tr("Perona-Malik anisotropic diffusion"));
    aniso_groupbox->setCheckable(true);
    aniso_groupbox->setChecked(false);
    aniso1_radio = new QRadioButton("g(∇I) = exp(-(|∇I|/κ)²)");
    aniso2_radio = new QRadioButton("g(∇I) = 1/(1+(1+(|∇I|/κ)²)");
    iteration_filter_spin = new QSpinBox;
    iteration_filter_spin->setSingleStep(1);
    iteration_filter_spin->setMinimum(0);
    iteration_filter_spin->setMaximum(5000);
    lambda_spin = new QDoubleSpinBox;
    lambda_spin->setSingleStep(0.01);
    lambda_spin->setMinimum(0.0);
    lambda_spin->setMaximum(1.0/4.0);
    kappa_spin = new QDoubleSpinBox;
    kappa_spin->setSingleStep(1.0);
    kappa_spin->setMinimum(0.0);
    kappa_spin->setMaximum(10000.0);
    connect(aniso_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(aniso1_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(aniso2_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(iteration_filter_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    connect(lambda_spin,SIGNAL(valueChanged(double)),this,SLOT(filtering_visu()));
    connect(kappa_spin,SIGNAL(valueChanged(double)),this,SLOT(filtering_visu()));
    QFormLayout* aniso_layout = new QFormLayout;
    aniso_layout->addRow(tr("iterations ="), iteration_filter_spin);
    aniso_layout->addRow("λ =", lambda_spin);
    aniso_layout->addRow("κ =", kappa_spin);
    aniso_layout->addRow(tr("function 1 :"), aniso1_radio);
    aniso_layout->addRow(tr("function 2 :"), aniso2_radio);

    ////////////////////////////////////////////

    open_groupbox = new QGroupBox(tr("Opening"));
    open_groupbox->setCheckable(true);
    open_groupbox->setChecked(false);
    klength_open_spin = new QSpinBox;
    klength_open_spin->setSingleStep(2);
    klength_open_spin->setMinimum(1);
    klength_open_spin->setMaximum(499);
    klength_open_spin->setSuffix("²");
    klength_open_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));
    connect(open_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(klength_open_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    QFormLayout* open_layout = new QFormLayout;
    open_layout->addRow(tr("SE size ="), klength_open_spin);

    close_groupbox = new QGroupBox(tr("Closing"));
    close_groupbox->setCheckable(true);
    close_groupbox->setChecked(false);
    klength_close_spin = new QSpinBox;
    klength_close_spin->setSingleStep(2);
    klength_close_spin->setMinimum(1);
    klength_close_spin->setMaximum(499);
    klength_close_spin->setSuffix("²");
    klength_close_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));
    connect(close_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(klength_close_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    QFormLayout* close_layout = new QFormLayout;
    close_layout->addRow(tr("SE size ="), klength_close_spin);

    ////////////////////////////////////////////

    tophat_groupbox = new QGroupBox(tr("Top-hat transform"));
    tophat_groupbox->setCheckable(true);
    tophat_groupbox->setChecked(false);
    whitetophat_radio = new QRadioButton(tr("white top-hat"));
    whitetophat_radio->setToolTip(tr("difference between the input image the opened"));
    blacktophat_radio = new QRadioButton(tr("black top-hat"));
    blacktophat_radio->setToolTip(tr("difference between the closed and the input image"));
    klength_tophat_spin = new QSpinBox;
    klength_tophat_spin->setSingleStep(2);
    klength_tophat_spin->setMinimum(1);
    klength_tophat_spin->setMaximum(499);
    klength_tophat_spin->setSuffix("²");
    klength_tophat_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));
    connect(tophat_groupbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(whitetophat_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(blacktophat_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(klength_tophat_spin,SIGNAL(valueChanged(int)),this,SLOT(filtering_visu()));
    QFormLayout* tophat_layout = new QFormLayout;
    tophat_layout->addRow(" ", whitetophat_radio);
    tophat_layout->addRow(" ", blacktophat_radio);
    tophat_layout->addRow(tr("SE size ="), klength_tophat_spin);

    algo_groupbox = new QGroupBox(tr("Algorithm"));
    complex1_morpho_radio = new QRadioButton(tr("naïve algorithm in O(r)×O(n)"));
    complex2_morpho_radio = new QRadioButton(tr("Perreault's algorithm in O(1)×O(n)"));
    QVBoxLayout* algo_layout = new QVBoxLayout;
    algo_layout->addWidget(complex1_morpho_radio);
    algo_layout->addWidget(complex2_morpho_radio);
    connect(complex1_morpho_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));
    connect(complex2_morpho_radio,SIGNAL(clicked()),this,SLOT(filtering_visu()));

    if( operating_system == 2 )
    {
        median_layout->setFormAlignment(Qt::AlignLeft);
        mean_layout->setFormAlignment(Qt::AlignLeft);
        gaussian_layout->setFormAlignment(Qt::AlignLeft);
        aniso_layout->setFormAlignment(Qt::AlignLeft);
        open_layout->setFormAlignment(Qt::AlignLeft);
        close_layout->setFormAlignment(Qt::AlignLeft);
        tophat_layout->setFormAlignment(Qt::AlignLeft);
    }

    mean_groupbox->setLayout(mean_layout);
    gaussian_groupbox->setLayout(gaussian_layout);

    median_groupbox->setLayout(median_layout);
    aniso_groupbox->setLayout(aniso_layout);

    open_groupbox->setLayout(open_layout);
    close_groupbox->setLayout(close_layout);
    tophat_groupbox->setLayout(tophat_layout);
    algo_groupbox->setLayout(algo_layout);

    QVBoxLayout* filter_layout_linear = new QVBoxLayout;
    filter_layout_linear->addWidget(mean_groupbox);
    filter_layout_linear->addWidget(gaussian_groupbox);

    QVBoxLayout* filter_layout_edge_preserv = new QVBoxLayout;
    filter_layout_edge_preserv->addWidget(median_groupbox);
    filter_layout_edge_preserv->addWidget(aniso_groupbox);

    QVBoxLayout* filter_layout_mm = new QVBoxLayout;
    filter_layout_mm->addWidget(open_groupbox);
    filter_layout_mm->addWidget(close_groupbox);
    filter_layout_mm->addWidget(tophat_groupbox);
    filter_layout_mm->addWidget(algo_groupbox);

    ////////////////////////////////////////////

    preprocess_tabs = new QTabWidget(this);

    QWidget* page_noise = new QWidget;
    QWidget* page_filter_iso = new QWidget;
    QWidget* page_filter_ansio = new QWidget;
    QWidget* page_filter_morpho = new QWidget;

    noise_layout->addStretch(1);
    page_noise->setLayout(noise_layout);

    filter_layout_linear->addStretch(1);
    page_filter_iso->setLayout(filter_layout_linear);

    filter_layout_edge_preserv->addStretch(1);
    page_filter_ansio->setLayout(filter_layout_edge_preserv);

    filter_layout_mm->addStretch(1);
    page_filter_morpho->setLayout(filter_layout_mm);

    QTabWidget* filter_tabs = new QTabWidget(this);
    filter_tabs->addTab(page_filter_iso, tr("Linear"));
    filter_tabs->addTab(page_filter_ansio, tr("Edge preserving"));
    filter_tabs->addTab(page_filter_morpho, tr("Math morpho"));

    preprocess_tabs->addTab(page_noise, tr("Noise generators"));
    preprocess_tabs->addTab(filter_tabs, tr("Filters"));

    page3 = new QGroupBox(tr("Preprocessing"));
    page3->setCheckable(true);
    page3->setChecked(false);
    connect(page3,SIGNAL(clicked()),this,SLOT(filtering_visu()));

    time_filt = new QLabel(this);
    time_filt->setText(tr("time = "));
    QGroupBox* time_filt_groupbox = new QGroupBox(tr("Processing time"));
    QVBoxLayout* elapsed_filt_layout = new QVBoxLayout;
    elapsed_filt_layout->addWidget(time_filt);
    time_filt_groupbox->setLayout(elapsed_filt_layout);

    QVBoxLayout* page3_layout = new QVBoxLayout;
    page3_layout->addWidget(preprocess_tabs);
    page3_layout->addWidget(time_filt_groupbox);
    page3->setLayout(page3_layout);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Display tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* size_groupbox = new QGroupBox(tr("Image"));

    scale_spin = new QSpinBox;
    scale_spin->setSingleStep(25);
    scale_spin->setMinimum(1);
    scale_spin->setMaximum(5000);
    scale_spin->setSuffix(" %");
    scale_spin->setValue(100);

    scale_slider = new QSlider(Qt::Horizontal, this);
    if( operating_system == 3 )
    {
        scale_slider->setTickPosition(QSlider::TicksBelow);
    }
    else
    {
        scale_slider->setTickPosition(QSlider::TicksAbove);
    }
    scale_slider->setMinimum(1);
    scale_slider->setMaximum(1000);
    scale_slider->setTickInterval(100);
    scale_slider->setSingleStep(25);
    scale_slider->setValue(100);

    connect(scale_spin,SIGNAL(valueChanged(int)),this,SLOT(do_scale(int)));
    connect(scale_slider,SIGNAL(valueChanged(int)),scale_spin,SLOT(setValue(int)));
    connect(scale_spin,SIGNAL(valueChanged(int)),scale_slider,SLOT(setValue(int)));

    scale_spin->installEventFilter(this);
    scale_slider->installEventFilter(this);
    scale_spin->setMouseTracking(true);
    scale_slider->setMouseTracking(true);

    histo_checkbox = new QCheckBox(tr("histogram normalization for the gradient"));
    connect(histo_checkbox,SIGNAL(clicked()),this,SLOT(filtering_visu()));

    QFormLayout* size_layout = new QFormLayout;
    size_layout->addRow(tr("scale ="), scale_spin);
    size_layout->addRow(scale_slider);
    size_layout->addRow(" ",histo_checkbox);
    if( operating_system == 2 )
    {
        size_layout->setFormAlignment(Qt::AlignLeft);
    }

    size_groupbox->setLayout(size_layout);

    ////////////////////////////////////////////

    step_checkbox = new QCheckBox(tr("display"));
    step_checkbox->setToolTip(tr("to see the calculation time if not checked"));

    QGroupBox* active_contour_groupbox = new QGroupBox(tr("Active contour"));

    outsidecolor_combobox = new QComboBox;
    insidecolor_combobox = new QComboBox;

    // QPixmap pm : petite image affichant la couleur devant le nom de la couleur dans le combobox
    QPixmap pm(12,12);
    pm.fill(Qt::red);
    outsidecolor_combobox->addItem (pm, tr("Red"));
    insidecolor_combobox->addItem (pm, tr("Red"));
    pm.fill(Qt::green);
    outsidecolor_combobox->addItem (pm, tr("Green"));
    insidecolor_combobox->addItem (pm, tr("Green"));
    pm.fill(Qt::blue);
    outsidecolor_combobox->addItem (pm, tr("Blue"));
    insidecolor_combobox->addItem (pm, tr("Blue"));
    pm.fill(Qt::cyan);
    outsidecolor_combobox->addItem (pm, tr("Cyan"));
    insidecolor_combobox->addItem (pm, tr("Cyan"));
    pm.fill(Qt::magenta);
    outsidecolor_combobox->addItem (pm, tr("Magenta"));
    insidecolor_combobox->addItem (pm, tr("Magenta"));
    pm.fill(Qt::yellow);
    outsidecolor_combobox->addItem (pm, tr("Yellow"));
    insidecolor_combobox->addItem (pm, tr("Yellow"));
    pm.fill(Qt::black);
    outsidecolor_combobox->addItem (pm, tr("Black"));
    insidecolor_combobox->addItem (pm, tr("Black"));
    pm.fill(Qt::white);
    outsidecolor_combobox->addItem (pm, tr("White"));
    insidecolor_combobox->addItem (pm, tr("White"));
    pm.fill(Qt::transparent);
    outsidecolor_combobox->addItem (pm,tr("Selected"));
    insidecolor_combobox->addItem (pm,tr("Selected"));
    outsidecolor_combobox->addItem (pm, tr("No"));
    insidecolor_combobox->addItem (pm, tr("No"));


    connect(outsidecolor_combobox,SIGNAL(activated(int)),this,SLOT(phi_visu(int)));
    connect(insidecolor_combobox,SIGNAL(activated(int)),this,SLOT(phi_visu(int)));

    QPushButton* outsidecolor_select = new QPushButton(tr("Select"));
    connect(outsidecolor_select,SIGNAL(clicked()),this,SLOT(color_out()));

    QPushButton* insidecolor_select = new QPushButton(tr("Select"));
    connect(insidecolor_select,SIGNAL(clicked()),this,SLOT(color_in()));

    QFormLayout* active_contour_form = new QFormLayout;
    active_contour_form->addRow(tr("after each iteration :"), step_checkbox);
    if( operating_system == 2 )
    {
        active_contour_form->setFormAlignment(Qt::AlignLeft);
    }

    QFormLayout* Loutcolor_form = new QFormLayout;
    Loutcolor_form->addRow(tr("Lout :"), outsidecolor_combobox);
    if( operating_system == 2 )
    {
        Loutcolor_form->setFormAlignment(Qt::AlignLeft);
    }

    QFormLayout* Lincolor_form = new QFormLayout;
    Lincolor_form->addRow(tr("  Lin :"), insidecolor_combobox);
    if( operating_system == 2 )
    {
        Lincolor_form->setFormAlignment(Qt::AlignLeft);
    }

    QHBoxLayout* Loutcolor_hlay = new QHBoxLayout;
    Loutcolor_hlay->addLayout(Loutcolor_form);
    Loutcolor_hlay->addWidget(outsidecolor_select);

    QHBoxLayout* Lincolor_hlay = new QHBoxLayout;
    Lincolor_hlay->addLayout(Lincolor_form);
    Lincolor_hlay->addWidget(insidecolor_select);

    QVBoxLayout* color_layout = new QVBoxLayout;
    color_layout->addLayout(Loutcolor_hlay);
    color_layout->addLayout(Lincolor_hlay);

    QGroupBox* color_groupbox = new QGroupBox(tr("Boundaries colors"));
    color_groupbox->setFlat(true);
    color_groupbox->setLayout(color_layout);

    QVBoxLayout* active_contour_layout = new QVBoxLayout;
    active_contour_layout->addLayout(active_contour_form);
    active_contour_layout->addWidget(color_groupbox);

    active_contour_groupbox->setLayout(active_contour_layout);

    ////////////////////////////////////////////

    QVBoxLayout* display_layout = new QVBoxLayout;
    display_layout->addWidget(size_groupbox);
    display_layout->addWidget(active_contour_groupbox);
    display_layout->addStretch(1);

    QWidget* page4 = new QWidget;
    page4->setLayout( display_layout );

    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////

    tabs = new QTabWidget(this);
    tabs->addTab( page1, tr("Algorithm") );
    tabs->addTab( page2, tr("Initialization") );
    tabs->addTab( page3, tr("Preprocessing") );
    tabs->addTab( page4, tr("Display") );
    connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_visu(int)) );

    QGridLayout *settings_grid = new QGridLayout;
    settings_grid->addWidget(tabs,0,0);
    settings_grid->addWidget(scrollArea_settings,0,1);
    settings_grid->addWidget(dial_buttons,1,1);

    settings_grid->setColumnStretch(1,1);
    setLayout(settings_grid);

    last_directory_used = settings.value("Main/Name/last_directory_used",QDir().homePath()).toString();
    hasEllipse1 = settings.value("Settings/Initialization/hasEllipse", true).toBool();
    init_width1 = settings.value("Settings/Initialization/init_width", 0.65).toDouble();
    init_height1 = settings.value("Settings/Initialization/init_height", 0.65).toDouble();
    center_x1 = settings.value("Settings/Initialization/center_x", 0.0).toDouble();
    center_y1 = settings.value("Settings/Initialization/center_y", 0.0).toDouble();

    hasContoursHidden = true;
    scale_spin->setValue(settings.value("Settings/Display/zoom_factor", 100).toInt());
    imageLabel_settings->set_zoomFactor(float(scale_spin->value())/100.0f);

    cancel_settings();
    scrollArea_settings->setFocus(Qt::OtherFocusReason);
}

SettingsWindow::Settings::Settings()
{
    QSettings settings("Bessy", "Ofeli");

    //////////////////////////////////////////////////
    // Algorithm

    Na1 = settings.value("Settings/Algorithm/Na", 30).toUInt();
    Ns1 = settings.value("Settings/Algorithm/Ns", 3).toUInt();
    model = settings.value("Settings/Algorithm/model", 1).toUInt();
    lambda_out1 = settings.value("Settings/Algorithm/lambda_out", 1).toUInt();
    lambda_in1 = settings.value("Settings/Algorithm/lambda_in", 1).toUInt();
    kernel_gradient_length1 = settings.value("Settings/Algorithm/kernel_gradient_length", 5).toUInt();
    alpha1 = settings.value("Settings/Algorithm/alpha", 1).toUInt();
    beta1 = settings.value("Settings/Algorithm/beta", 10).toUInt();
    gamma1 = settings.value("Settings/Algorithm/gamma", 10).toUInt();

    hasSmoothingCycle1 = settings.value("Settings/Algorithm/hasSmoothingCycle", true).toBool();
    kernel_curve1 = settings.value("Settings/Algorithm/kernel_curve", 5).toUInt();
    std_curve1 = settings.value("Settings/Algorithm/std_curve", 2.0).toDouble();

    //////////////////////////////////////////////////
    // Initialization

    phi_init = nullptr;



    //////////////////////////////////////////////////
    // Preprocessing

    hasPreprocess = settings.value("Settings/Preprocessing/hasPreprocess", false).toBool();

    hasGaussianNoise = settings.value("Settings/Preprocessing/hasGaussianNoise", false).toBool();
    std_noise = settings.value("Settings/Preprocessing/std_noise", 20.0).toDouble();
    hasSaltNoise = settings.value("Settings/Preprocessing/hasSaltNoise", false).toBool();
    proba_noise = settings.value("Settings/Preprocessing/proba_noise", 0.05).toDouble();
    hasSpeckleNoise = settings.value("Settings/Preprocessing/hasSpeckleNoise", false).toBool();
    std_speckle_noise = settings.value("Settings/Preprocessing/std_speckle_noise", 0.16).toDouble();

    hasMedianFilt = settings.value("Settings/Preprocessing/hasMedianFilt", false).toBool();
    kernel_median_length1 = settings.value("Settings/Preprocessing/kernel_median_length", 5).toUInt();
    hasO1algo1 = settings.value("Settings/Preprocessing/hasO1algo", true).toBool();
    hasMeanFilt = settings.value("Settings/Preprocessing/hasMeanFilt", false).toBool();
    kernel_mean_length1 = settings.value("Settings/Preprocessing/kernel_mean_length", 5).toUInt();
    hasGaussianFilt = settings.value("Settings/Preprocessing/hasGaussianFilt", false).toBool();
    kernel_gaussian_length1 = settings.value("Settings/Preprocessing/kernel_gaussian_length", 5).toUInt();
    sigma = settings.value("Settings/Preprocessing/sigma", 2.0).toDouble();

    hasAnisoDiff = settings.value("Settings/Preprocessing/hasAnisoDiff", false).toBool();
    aniso_option1 = settings.value("Settings/Preprocessing/aniso_option", 1).toUInt();
    max_itera1 = settings.value("Settings/Preprocessing/max_itera", 10).toUInt();
    lambda1 = settings.value("Settings/Preprocessing/lambda", 1.0/7.0).toDouble();
    kappa1 = settings.value("Settings/Preprocessing/kappa", 30.0).toDouble();

    hasOpenFilt = settings.value("Settings/Preprocessing/hasOpenFilt", false).toBool();
    kernel_open_length1 = settings.value("Settings/Preprocessing/kernel_open_length", 5).toUInt();
    hasCloseFilt = settings.value("Settings/Preprocessing/hasCloseFilt", false).toBool();
    kernel_close_length1 = settings.value("Settings/Preprocessing/kernel_close_length", 5).toUInt();
    hasTophatFilt = settings.value("Settings/Preprocessing/hasTophatFilt", false).toBool();
    isWhiteTophat = settings.value("Settings/Preprocessing/isWhiteTophat", true).toBool();
    kernel_tophat_length1 = settings.value("Settings/Preprocessing/kernel_tophat_length", 5).toUInt();

    hasO1morpho1 = settings.value("Settings/Preprocessing/hasO1morpho", true).toBool();

    ////////////////////////////////////////////
    // Display

    hasHistoNormaliz = settings.value("Settings/Display/hasHistoNormaliz", true).toBool();

    hasDisplayEach = settings.value("Settings/Display/hasDisplayEach", true).toBool();

    outside_combo = settings.value("Settings/Display/outside_combo", 2).toInt();
    inside_combo = settings.value("Settings/Display/inside_combo", 0).toInt();

    Rout_selected1 = (unsigned char)(settings.value("Settings/Display/Rout_selected", 128).toUInt());
    Gout_selected1 = (unsigned char)(settings.value("Settings/Display/Gout_selected", 0).toUInt());
    Bout_selected1 = (unsigned char)(settings.value("Settings/Display/Bout_selected", 255).toUInt());
    Rin_selected1 = (unsigned char)(settings.value("Settings/Display/Rin_selected", 255).toUInt());
    Gin_selected1 = (unsigned char)(settings.value("Settings/Display/Gin_selected", 128).toUInt());
    Bin_selected1 = (unsigned char)(settings.value("Settings/Display/Bin_selected", 0).toUInt());

    if( outside_combo == 8 )
    {
        Rout1 = Rout_selected1;
        Gout1 = Gout_selected1;
        Bout1 = Bout_selected1;
    }
    else
    {
       get_color(outside_combo,Rout1,Gout1,Bout1);
    }

    if( inside_combo == 8 )
    {
        Rin1 = Rin_selected1;
        Gin1 = Gin_selected1;
        Bin1 = Bin_selected1;
    }
    else
    {
        get_color(inside_combo,Rin1,Gin1,Bin1);
    }
}

void SettingsWindow::init(const unsigned char* img0, unsigned int img0_width, unsigned int img0_height, bool isRgb0, const QImage& qimg0)
{
    imageLabel_settings->set_qimage0(qimg0);

    img1 = img0;
    img_width = img0_width;
    img_height = img0_height;
    img_size = img0_width*img0_height;
    isRgb1 = isRgb0;
    img = qimg0;

    if( shape != nullptr )
    {
        delete[] shape;
        shape = nullptr;
    }
    if( shape == nullptr )
    {
        shape = new signed char[img_size];
    }

    if( shape_points != nullptr )
    {
        delete[] shape_points;
        shape_points = nullptr;
    }
    if( shape_points == nullptr )
    {
        shape_points = new unsigned int[2*img_size+1];
        shape_points[0] = list_end;
    }

    if( Lout_shape1 != nullptr )
    {
        delete[] Lout_shape1;
        Lout_shape1 = nullptr;
    }
    if( Lout_shape1 == nullptr )
    {
        Lout_shape1 = new unsigned int[img_size+1];
        Lout_shape1[0] = list_end;
    }

    if( Lin_shape1 != nullptr )
    {
        delete[] Lin_shape1;
        Lin_shape1 = nullptr;
    }
    if( Lin_shape1 == nullptr )
    {
        Lin_shape1 = new unsigned int[img_size+1];
        Lin_shape1[0] = list_end;
    }

    if( Lout_2 != nullptr )
    {
        delete[] Lout_2;
        Lout_2 = nullptr;
    }
    if( Lout_2 == nullptr )
    {
        Lout_2 = new unsigned int[img_size+1];
        Lout_2[0] = list_end;
    }

    if( Lin_2 != nullptr )
    {
        delete[] Lin_2;
        Lin_2 = nullptr;
    }
    if( Lin_2 == nullptr )
    {
        Lin_2 = new unsigned int[img_size+1];
        Lin_2[0] = list_end;
    }

    if( phi_init2 != nullptr )
    {
        delete[] phi_init2;
        phi_init2 = nullptr;
    }
    if( phi_init2 == nullptr )
    {
        phi_init2 = new signed char[img_size];
    }

    if( phi_init1_clean != nullptr )
    {
        delete[] phi_init1_clean;
        phi_init1_clean = nullptr;
    }
    if( phi_init1_clean == nullptr )
    {
        phi_init1_clean = new signed char[img_size];
    }
    mainwindow_settings.phi_init = phi_init1_clean;
    mainwindow_settings.phi_width = img_width;
    mainwindow_settings.phi_height = img_height;

    if( phi_init2_clean != nullptr )
    {
        delete[] phi_init2_clean;
        phi_init2_clean = nullptr;
    }
    if( phi_init2_clean == nullptr )
    {
        phi_init2_clean = new signed char[img_size];
    }

    if( image_filter_uchar != nullptr )
    {
        delete[] image_filter_uchar;
        image_filter_uchar = nullptr;
    }
    if( image_filter_uchar == nullptr )
    {
        image_filter_uchar = new unsigned char[3*img_size];
    }
    image_filter = QImage(image_filter_uchar, img_width, img_height, 3*img_width, QImage::Format_RGB888);

    if( image_phi_uchar != nullptr )
    {
        delete[] image_phi_uchar;
        image_phi_uchar = nullptr;
    }
    if( image_phi_uchar == nullptr )
    {
        image_phi_uchar = new unsigned char[3*img_size];
    }
    image_phi = QImage(image_phi_uchar, img_width, img_height, 3*img_width, QImage::Format_RGB888);

    if( image_shape_uchar != nullptr )
    {
        delete[] image_shape_uchar;
        image_shape_uchar = nullptr;
    }
    if( image_shape_uchar == nullptr )
    {
        image_shape_uchar = new unsigned char[3*img_size];
    }
    image_shape = QImage(image_shape_uchar, img_width, img_height, 3*img_width, QImage::Format_RGB888);

    QImage img_phi0(100,100,QImage::Format_Indexed8);

    for( unsigned int y = 0; y < 100; y++ )
    {
        for( unsigned int x = 0; x < 100; x++ )
        {
            if( x > 5 && x < 95 && y > 5 && y < 95 )
            {
                *( img_phi0.scanLine(y)+x ) = 255;
            }
            else
            {
                *( img_phi0.scanLine(y)+x ) = 0;
            }
        }
    }

    QSettings settings("Bessy","Ofeli");

    if( img_phi.isNull() )
    {
        img_phi = settings.value("Settings/Initialization/img_phi",img_phi0).value<QImage>().copy();
    }

    if( img_phi.isNull() )
    {
        img_phi = img_phi0.copy().scaled(img_width, img_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    else
    {
        img_phi = img_phi.scaled(img_width, img_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    imgPhi2phiInit();

    unsigned int n_out = 0;
    unsigned int n_in = 0;
    for( unsigned int offset = 0; offset < img_size; offset++ )
    {
        if( phi_init1_clean[offset] == 1 )
        {
            Lout_2[n_out++] = offset;
        }
        if( phi_init1_clean[offset] == -1 )
        {
            Lin_2[n_in++] = offset;
        }
    }
    Lout_2[n_out] = list_end;
    Lin_2[n_in] = list_end;

    if( filters2 != nullptr )
    {
        delete filters2;
        filters2 = nullptr;
    }

    if( filters2 == nullptr )
    {
        if( isRgb1 )
        {
            filters2 = new ofeli::Filters(img1,img_width,img_height,3);
        }
        else
        {
            filters2 = new ofeli::Filters(img1,img_width,img_height,1);
        }
    }

    tab_visu(tabs->currentIndex());

    disconnect(scale_spin,SIGNAL(valueChanged(int)),this,SLOT(do_scale(int)));

    scale_spin->setMaximum(1000000/img_height);
    scale_spin->setSingleStep(80000/(7*img_height));

    scale_slider->setMaximum(160000/img_height);
    scale_slider->setTickInterval(160000/(7*img_height));

    connect(scale_spin,SIGNAL(valueChanged(int)),this,SLOT(do_scale(int)));

    open_phi_button->setEnabled(true);
    save_phi_button->setEnabled(true);

    if( isRgb1 )
    {
        yuv_groupbox->setHidden(false);
    }
    else
    {
        yuv_groupbox->setHidden(true);
    }
}

void SettingsWindow::update_visu()
{
    if( parent() != nullptr )
    {
        scale_spin->setValue(static_cast<ofeli::MainWindow*>(parent())->get_zoom_factor());

        if( tabs->currentIndex() == 1 )
        {
            filtering_visu();
            phi_visu(true);
            shape_visu();
        }
        else
        {
            filtering_visu();
            phi_visu(false);
        }
    }
}

void SettingsWindow::apply_settings()
{

    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

    mainwindow_settings.Na1 = Na_spin->value();

    if( chanvese_radio->isChecked() )
    {
        mainwindow_settings.model = 1;
    }
    if( geodesic_radio->isChecked() )
    {
        mainwindow_settings.model = 2;
    }

    mainwindow_settings.lambda_out1 = lambda_out_spin->value();
    mainwindow_settings.lambda_in1 = lambda_in_spin->value();

    mainwindow_settings.kernel_gradient_length1 = klength_gradient_spin->value();

    mainwindow_settings.alpha1 = Yspin->value();
    mainwindow_settings.beta1 = Uspin->value();
    mainwindow_settings.gamma1 = Vspin->value();

    if( internalspeed_groupbox->isChecked() )
    {
        mainwindow_settings.hasSmoothingCycle1 = true;
    }
    else
    {
        mainwindow_settings.hasSmoothingCycle1 = false;
    }

    mainwindow_settings.Ns1 = Ns_spin->value();

    mainwindow_settings.kernel_curve1 = klength_spin->value();
    mainwindow_settings.std_curve1 = std_spin->value();


    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    if( rectangle_radio->isChecked() )
    {
        hasEllipse1 = false;
    }
    if( ellipse_radio->isChecked() )
    {
        hasEllipse1 = true;
    }

    init_width1 = double(width_shape_spin->value())/100.0;
    init_height1 = double(height_shape_spin->value())/100.0;

    center_x1 = double(abscissa_spin->value())/100.0;
    center_y1 = double(ordinate_spin->value())/100.0;

    if( phi_init1_clean != nullptr && phi_init2_clean != nullptr )
    {
        std::memcpy(phi_init1_clean,phi_init2_clean,img_size);
        phiInit2imgPhi();
    }

    ///////////////////////////////////
    //        Preprocessing          //
    ///////////////////////////////////

    if( page3->isChecked() )
    {
        mainwindow_settings.hasPreprocess = true;
    }
    else
    {
        mainwindow_settings.hasPreprocess = false;
    }

    if( gaussian_noise_groupbox->isChecked() )
    {
        mainwindow_settings.hasGaussianNoise = true;
    }
    else
    {
        mainwindow_settings.hasGaussianNoise = false;
    }
    mainwindow_settings.std_noise = std_noise_spin->value();

    if( salt_noise_groupbox->isChecked() )
    {
        mainwindow_settings.hasSaltNoise = true;
    }
    else
    {
        mainwindow_settings.hasSaltNoise = false;
    }
    mainwindow_settings.proba_noise = proba_noise_spin->value()/100.0;

    if( speckle_noise_groupbox->isChecked() )
    {
        mainwindow_settings.hasSpeckleNoise = true;
    }
    else
    {
        mainwindow_settings.hasSpeckleNoise = false;
    }
    mainwindow_settings.std_speckle_noise = std_speckle_noise_spin->value();

    if( median_groupbox->isChecked() )
    {
        mainwindow_settings.hasMedianFilt = true;
    }
    else
    {
        mainwindow_settings.hasMedianFilt = false;
    }
    mainwindow_settings.kernel_median_length1 = klength_median_spin->value();
    if( complex_radio2->isChecked() )
    {
        mainwindow_settings.hasO1algo1 = true;
    }
    else
    {
        mainwindow_settings.hasO1algo1 = false;
    }

    if( mean_groupbox->isChecked() )
    {
        mainwindow_settings.hasMeanFilt = true;
    }
    else
    {
        mainwindow_settings.hasMeanFilt = false;
    }
    mainwindow_settings.kernel_mean_length1 = klength_mean_spin->value();

    if( gaussian_groupbox->isChecked() )
    {
        mainwindow_settings.hasGaussianFilt = true;
    }
    else
    {
        mainwindow_settings.hasGaussianFilt = false;
    }
    mainwindow_settings.kernel_gaussian_length1 = klength_gaussian_spin->value();
    mainwindow_settings.sigma = std_filter_spin->value();

    if( aniso_groupbox->isChecked() )
    {
        mainwindow_settings.hasAnisoDiff = true;
    }
    else
    {
        mainwindow_settings.hasAnisoDiff = false;
    }

    mainwindow_settings.max_itera1 = iteration_filter_spin->value();
    mainwindow_settings.lambda1 = lambda_spin->value();
    mainwindow_settings.kappa1 = kappa_spin->value();
    if( aniso1_radio->isChecked() )
    {
        mainwindow_settings.aniso_option1 = 1;
    }
    if( aniso2_radio->isChecked() )
    {
        mainwindow_settings.aniso_option1 = 2;
    }

    if( open_groupbox->isChecked() )
    {
        mainwindow_settings.hasOpenFilt = true;
    }
    else
    {
        mainwindow_settings.hasOpenFilt = false;
    }
    mainwindow_settings.kernel_open_length1 = klength_open_spin->value();

    if( close_groupbox->isChecked() )
    {
        mainwindow_settings.hasCloseFilt = true;
    }
    else
    {
        mainwindow_settings.hasCloseFilt = false;
    }
    mainwindow_settings.kernel_close_length1 = klength_close_spin->value();

    if( tophat_groupbox->isChecked() )
    {
        mainwindow_settings.hasTophatFilt = true;
    }
    else
    {
        mainwindow_settings.hasTophatFilt = false;
    }
    if( whitetophat_radio->isChecked() )
    {
        mainwindow_settings.isWhiteTophat = true;
    }
    else
    {
        mainwindow_settings.isWhiteTophat = false;
    }
    mainwindow_settings.kernel_tophat_length1 = klength_tophat_spin->value();

    if( complex2_morpho_radio->isChecked() )
    {
        mainwindow_settings.hasO1morpho1 = true;
    }
    else
    {
        mainwindow_settings.hasO1morpho1 = false;
    }

    /////////////////////////////
    //        Display          //
    /////////////////////////////

    if( parent() != nullptr )
    {
        static_cast<ofeli::MainWindow*>(parent())->set_zoom_factor(scale_spin->value());
    }

    if( histo_checkbox->isChecked() )
    {
        mainwindow_settings.hasHistoNormaliz = true;
    }
    else
    {
        mainwindow_settings.hasHistoNormaliz = false;
    }



    if( step_checkbox->isChecked() )
    {
        mainwindow_settings.hasDisplayEach = true;
    }
    else
    {
        mainwindow_settings.hasDisplayEach = false;
    }


    mainwindow_settings.inside_combo = insidecolor_combobox->currentIndex();
    mainwindow_settings.outside_combo = outsidecolor_combobox->currentIndex();

    mainwindow_settings.Rin_selected1 = Rin_selected2;
    mainwindow_settings.Gin_selected1 = Gin_selected2;
    mainwindow_settings.Bin_selected1 = Bin_selected2;

    mainwindow_settings.Rout_selected1 = Rout_selected2;
    mainwindow_settings.Gout_selected1 = Gout_selected2;
    mainwindow_settings.Bout_selected1 = Bout_selected2;

    if( mainwindow_settings.outside_combo == 8 )
    {
        mainwindow_settings.Rout1 = mainwindow_settings.Rout_selected1;
        mainwindow_settings.Gout1 = mainwindow_settings.Gout_selected1;
        mainwindow_settings.Bout1 = mainwindow_settings.Bout_selected1;
    }
    else
    {
       get_color(mainwindow_settings.outside_combo,
                 mainwindow_settings.Rout1,mainwindow_settings.Gout1,mainwindow_settings.Bout1);
    }

    if( mainwindow_settings.inside_combo == 8 )
    {
        mainwindow_settings.Rin1 = mainwindow_settings.Rin_selected1;
        mainwindow_settings.Gin1 = mainwindow_settings.Gin_selected1;
        mainwindow_settings.Bin1 = mainwindow_settings.Bin_selected1;
    }
    else
    {
        get_color(mainwindow_settings.inside_combo,
                  mainwindow_settings.Rin1,mainwindow_settings.Gin1,mainwindow_settings.Bin1);
    }
}

// si on clique sur le boutton Cancel de settings_window
// les widgets reprennent leur état, correspondant aux valeurs des paramètres
void SettingsWindow::cancel_settings()
{
    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

    Na_spin->setValue(mainwindow_settings.Na1);

    if( mainwindow_settings.model == 1 )
    {
        chanvese_radio->setChecked(true);
    }
    if( mainwindow_settings.model == 2 )
    {
        geodesic_radio->setChecked(true);
    }

    lambda_out_spin->setValue(mainwindow_settings.lambda_out1);
    lambda_in_spin->setValue(mainwindow_settings.lambda_in1);

    klength_gradient_spin->setValue(mainwindow_settings.kernel_gradient_length1);

    Yspin->setValue(mainwindow_settings.alpha1);
    Uspin->setValue(mainwindow_settings.beta1);
    Vspin->setValue(mainwindow_settings.gamma1);

    Ns_spin->setValue(mainwindow_settings.Ns1);

    if( mainwindow_settings.hasSmoothingCycle1 )
    {
        internalspeed_groupbox->setChecked(true);
    }
    else
    {
        internalspeed_groupbox->setChecked(false);
    }

    klength_spin->setValue(mainwindow_settings.kernel_curve1);
    std_spin->setValue(mainwindow_settings.std_curve1);

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    if( !hasEllipse1 )
    {
        rectangle_radio->setChecked(true);
    }
    else
    {
        ellipse_radio->setChecked(true);
    }

    width_shape_spin->setValue(int(init_width1*100.0));
    height_shape_spin->setValue(int(init_height1*100.0));

    abscissa_spin->setValue(int(center_x1*100.0));
    ordinate_spin->setValue(int(center_y1*100.0));

    if( phi_init1_clean != nullptr && phi_init2_clean != nullptr )
    {
        std::memcpy(phi_init2_clean,phi_init1_clean,img_size);
    }

    // Lin_2 mis à jour pour la prochaine visualisation
    int n_out = 0;
    int n_in = 0;
    if( phi_init1_clean != nullptr && phi_init2 != nullptr && Lin_2 != nullptr )
    {
        for( unsigned int offset = 0; offset < img_size; offset++ )
        {
            if( phi_init1_clean[offset] < 0 )
            {
                phi_init2[offset] = -1;

                if( phi_init1_clean[offset] == -1 )
                {
                    Lin_2[n_in++] = offset;
                }
            }
            else
            {
                phi_init2[offset] = 1;

                if( phi_init1_clean[offset] == 1 )
                {
                    Lout_2[n_out++] = offset;
                }
            }
        }
        Lout_2[n_out] = list_end;
        Lin_2[n_in] = list_end;
    }

    ///////////////////////////////////
    //        Preprocessing          //
    ///////////////////////////////////

    if( mainwindow_settings.hasPreprocess )
    {
        page3->setChecked(true);
    }
    else
    {
       page3->setChecked(false);
    }

    if( mainwindow_settings.hasGaussianNoise )
    {
        gaussian_noise_groupbox->setChecked(true);
    }
    else
    {
        gaussian_noise_groupbox->setChecked(false);
    }


    std_noise_spin->setValue(mainwindow_settings.std_noise);

    if( mainwindow_settings.hasSaltNoise )
    {
        salt_noise_groupbox->setChecked(true);
    }
    else
    {
        salt_noise_groupbox->setChecked(false);
    }

    proba_noise_spin->setValue(100.0*mainwindow_settings.proba_noise);

    if( mainwindow_settings.hasSpeckleNoise )
    {
        speckle_noise_groupbox->setChecked(true);
    }
    else
    {
        speckle_noise_groupbox->setChecked(false);
    }

    std_speckle_noise_spin->setValue(mainwindow_settings.std_speckle_noise);

    if( mainwindow_settings.hasMedianFilt )
    {
        median_groupbox->setChecked(true);
    }
    else
    {
        median_groupbox->setChecked(false);
    }

    klength_median_spin->setValue(mainwindow_settings.kernel_median_length1);

    if( mainwindow_settings.hasO1algo1 )
    {
        complex_radio2->setChecked(true);
    }
    else
    {
        complex_radio1->setChecked(true);
    }

    if( mainwindow_settings.hasMeanFilt )
    {
        mean_groupbox->setChecked(true);
    }
    else
    {
        mean_groupbox->setChecked(false);
    }
    klength_mean_spin->setValue(mainwindow_settings.kernel_mean_length1);

    if( mainwindow_settings.hasGaussianFilt )
    {
        gaussian_groupbox->setChecked(true);
    }
    else
    {
        gaussian_groupbox->setChecked(false);
    }
    klength_gaussian_spin->setValue(mainwindow_settings.kernel_gaussian_length1);
    std_filter_spin->setValue(mainwindow_settings.sigma);

    if( mainwindow_settings.hasAnisoDiff )
    {
        aniso_groupbox->setChecked(true);
    }
    else
    {
        aniso_groupbox->setChecked(false);
    }

    iteration_filter_spin->setValue(mainwindow_settings.max_itera1);
    lambda_spin->setValue(mainwindow_settings.lambda1);
    kappa_spin->setValue(mainwindow_settings.kappa1);
    if( mainwindow_settings.aniso_option1 == 1 )
    {
        aniso1_radio->setChecked(true);
    }
    if( mainwindow_settings.aniso_option1 == 2 )
    {
        aniso2_radio->setChecked(true);
    }

    if( mainwindow_settings.hasOpenFilt )
    {
        open_groupbox->setChecked(true);
    }
    else
    {
        open_groupbox->setChecked(false);
    }
    klength_open_spin->setValue(mainwindow_settings.kernel_open_length1);

    if( mainwindow_settings.hasCloseFilt )
    {
        close_groupbox->setChecked(true);
    }
    else
    {
        close_groupbox->setChecked(false);
    }
    klength_close_spin->setValue(mainwindow_settings.kernel_close_length1);

    if( mainwindow_settings.hasTophatFilt )
    {
        tophat_groupbox->setChecked(true);
    }
    else
    {
        tophat_groupbox->setChecked(false);
    }
    if( mainwindow_settings.isWhiteTophat )
    {
        whitetophat_radio->setChecked(true);
    }
    else
    {
        blacktophat_radio->setChecked(true);
    }
    klength_tophat_spin->setValue(mainwindow_settings.kernel_tophat_length1);

    if( mainwindow_settings.hasO1morpho1 )
    {
        complex2_morpho_radio->setChecked(true);
    }
    else
    {
        complex1_morpho_radio->setChecked(true);
    }

    /////////////////////////////
    //        Display          //
    /////////////////////////////

    if ( parent() != nullptr )
    {
        scale_spin->setValue(static_cast<ofeli::MainWindow*>(parent())->get_zoom_factor());
    }

    if( mainwindow_settings.hasHistoNormaliz )
    {
        histo_checkbox->setChecked(true);
    }
    else
    {
        histo_checkbox->setChecked(false);
    }

    if( mainwindow_settings.hasDisplayEach )
    {
        step_checkbox->setChecked(true);
    }
    else
    {
        step_checkbox->setChecked(false);
    }


    insidecolor_combobox->setCurrentIndex(mainwindow_settings.inside_combo);
    outsidecolor_combobox->setCurrentIndex(mainwindow_settings.outside_combo);

    Rin_selected2 = mainwindow_settings.Rin_selected1;
    Gin_selected2 = mainwindow_settings.Gin_selected1;
    Bin_selected2 = mainwindow_settings.Bin_selected1;

    Rout_selected2 = mainwindow_settings.Rout_selected1;
    Gout_selected2 = mainwindow_settings.Gout_selected1;
    Bout_selected2 = mainwindow_settings.Bout_selected1;

    unsigned char R, G, B;

    if( mainwindow_settings.inside_combo == 8 )
    {
        R = Rin_selected2;
        G = Gin_selected2;
        B = Bin_selected2;
    }
    else
    {
        get_color(mainwindow_settings.inside_combo,R,G,B);
    }
    QPixmap pm1(12,12);
    pm1.fill(QColor(R,G,B));
    insidecolor_combobox->setItemIcon(8,pm1);

    if( mainwindow_settings.outside_combo == 8 )
    {
        R = Rout_selected2;
        G = Gout_selected2;
        B = Bout_selected2;
    }
    else
    {
        get_color(mainwindow_settings.outside_combo,R,G,B);
    }
    QPixmap pm2(12,12);
    pm2.fill(QColor(R,G,B));
    outsidecolor_combobox->setItemIcon(8,pm2);
}

void SettingsWindow::default_settings()
{
    ///////////////////////////////////////////////////

    // paramètres par défaut de l'onglet Algorithm

    ////////////////////////////////////////////////////

    mainwindow_settings.Na1 = 30;
    mainwindow_settings.Ns1 = 3;
    mainwindow_settings.model = 1;
    mainwindow_settings.lambda_out1 = 1;
    mainwindow_settings.lambda_in1 = 1;
    mainwindow_settings.kernel_gradient_length1 = 5;
    mainwindow_settings.alpha1 = 1;
    mainwindow_settings.beta1 = 10;
    mainwindow_settings.gamma1 = 10;
    mainwindow_settings.hasSmoothingCycle1 = true;
    mainwindow_settings.kernel_curve1 = 5;
    mainwindow_settings.std_curve1 = 2.0;

    //////////////////////////////////////////////////

    ///////////////////////////////////////////////////

    // paramètres par défaut de l'onglet Initialisation

    ////////////////////////////////////////////////////

    hasEllipse1 = false;
    init_width1 = 0.95;
    init_height1 = 0.95;
    center_x1 = 0.0;
    center_y1 = 0.0;

    ////////////////////////////////////////////////////

    ///////////////////////////////////////////////////

    // paramètres par défaut de l'onglet Preprocessing

    ////////////////////////////////////////////////////

    mainwindow_settings.hasPreprocess = false;
    mainwindow_settings.hasGaussianNoise = false;
    mainwindow_settings.std_noise = 20.0;
    mainwindow_settings.hasSaltNoise = false;
    mainwindow_settings.proba_noise = 0.05;
    mainwindow_settings.hasSpeckleNoise = false;
    mainwindow_settings.std_speckle_noise = 0.16;

    mainwindow_settings.hasMedianFilt = false;
    mainwindow_settings.kernel_median_length1 = 5;
    mainwindow_settings.hasO1algo1 = true;

    mainwindow_settings.hasMeanFilt = false;
    mainwindow_settings.kernel_mean_length1 = 5;

    mainwindow_settings.hasGaussianFilt = false;
    mainwindow_settings.kernel_gaussian_length1 = 5;
    mainwindow_settings.sigma = 2.0;

    mainwindow_settings.hasAnisoDiff = false;
    mainwindow_settings.aniso_option1 = 1;
    mainwindow_settings.max_itera1 = 10;
    mainwindow_settings.lambda1 = 1.0/7.0;
    mainwindow_settings.kappa1 = 30.0;

    mainwindow_settings.hasOpenFilt = false;
    mainwindow_settings.kernel_open_length1 = 5;

    mainwindow_settings.hasCloseFilt = false;
    mainwindow_settings.kernel_close_length1 = 5;

    mainwindow_settings.hasTophatFilt = false;
    mainwindow_settings.isWhiteTophat = true;
    mainwindow_settings.kernel_tophat_length1 = 5;

    mainwindow_settings.hasO1morpho1 = true;

    ////////////////////////////////////////////

    // paramètres par défaut de l'onglet Display

    ///////////////////////////////////////////

    mainwindow_settings.hasHistoNormaliz = true;

    mainwindow_settings.hasDisplayEach = true;
    mainwindow_settings.inside_combo = 0;
    mainwindow_settings.outside_combo = 2;

    mainwindow_settings.Rout_selected1 = 128;
    mainwindow_settings.Gout_selected1 = 0;
    mainwindow_settings.Bout_selected1 = 255;

    mainwindow_settings.Rin_selected1 = 255;
    mainwindow_settings.Gin_selected1 = 128;
    mainwindow_settings.Bin_selected1 = 0;


    hasEllipse1 = true;
    init_width1 = 0.65;
    init_height1 = 0.65;
    center_x1 = 0.0;
    center_y1 = 0.0;
    cancel_settings();

    scale_spin->setValue(100);

    QImage img_phi0(100,100,QImage::Format_Indexed8);

    for( unsigned int y = 0; y < 100; y++ )
    {
        for( unsigned int x = 0; x < 100; x++ )
        {
            if( x > 5 && x < 95 && y > 5 && y < 95 )
            {
                *( img_phi0.scanLine(y)+x ) = 255;
            }
            else
            {
                *( img_phi0.scanLine(y)+x ) = 0;
            }
        }
    }

    img_phi = img_phi0.scaled( img_width, img_height, Qt::IgnoreAspectRatio, Qt::FastTransformation).copy();

    imgPhi2phiInit();

    if( img1 != nullptr )
    {
        // pour afficher l'image dans la fenetre settings
        unsigned int n_out = 0;
        unsigned int n_in = 0;
        for( unsigned int offset = 0; offset < img_size; offset++ )
        {
            if( phi_init1_clean[offset] == 1 )
            {
                Lout_2[n_out++] = offset;
            }
            if( phi_init1_clean[offset] == -1 )
            {
                Lin_2[n_in++] = offset;
            }
        }
        Lout_2[n_out] = list_end;
        Lin_2[n_in] = list_end;
    }

    filtering_visu();
    tab_visu(tabs->currentIndex());
}

// Fonction appelée pour le changement d 'echelle de l'image dans la fenêtre paramètre
void SettingsWindow::do_scale(int value)
{
    if( img1 != nullptr )
    {
        double scale_factor = value/100.0;
        imageLabel_settings->setZoomFactor(scale_factor);
    }
}

// Nettoyage des frontières
void SettingsWindow::clean_boundaries(signed char* phi, signed char* phi_clean)
{
    if( phi != nullptr && phi_clean != nullptr )
    {
        for( unsigned int y = 0; y < img_height; y++ )
        {
            for( unsigned int x = 0; x < img_width; x++ )
            {
                if( phi[ find_offset(x,y) ] == 1 )
                {
                    if( isRedundantPointOfLout(phi,x,y) )
                    {
                        phi_clean[ find_offset(x,y) ] = 3;
                    }
                    else
                    {
                        phi_clean[ find_offset(x,y) ] = 1;
                    }
                }

                if( phi[ find_offset(x,y) ] == -1 )
                {
                    if( isRedundantPointOfLin(phi,x,y) )
                    {
                        phi_clean[ find_offset(x,y) ] = -3;
                    }
                    else
                    {
                        phi_clean[ find_offset(x,y) ] = -1;
                    }
                }
            }
        }

        unsigned int offset;

        for( unsigned int n_out = 0; Lout_2[n_out] != list_end; n_out++ )
        {
            offset = Lout_2[n_out]*3;

            image_phi_uchar[offset] = image_filter_uchar[offset];
            image_phi_uchar[offset+1] = image_filter_uchar[offset+1];
            image_phi_uchar[offset+2] = image_filter_uchar[offset+2];

            image_shape_uchar[offset] = image_filter_uchar[offset];
            image_shape_uchar[offset+1] = image_filter_uchar[offset+1];
            image_shape_uchar[offset+2] = image_filter_uchar[offset+2];
        }

        for( unsigned int n_in = 0; Lin_2[n_in] != list_end; n_in++ )
        {
            offset = Lin_2[n_in]*3;

            image_phi_uchar[offset] = image_filter_uchar[offset];
            image_phi_uchar[offset+1] = image_filter_uchar[offset+1];
            image_phi_uchar[offset+2] = image_filter_uchar[offset+2];

            image_shape_uchar[offset] = image_filter_uchar[offset];
            image_shape_uchar[offset+1] = image_filter_uchar[offset+1];
            image_shape_uchar[offset+2] = image_filter_uchar[offset+2];
        }

        unsigned int n_out = 0;
        unsigned int n_in = 0;
        for( offset = 0; offset < img_size; offset++ )
        {
            if( phi_clean[offset] == 1 )
            {
                Lout_2[n_out++] = offset;
            }
            if( phi_clean[offset] == -1 )
            {
                Lin_2[n_in++] = offset;
            }
        }
        Lout_2[n_out] = list_end;
        Lin_2[n_in] = list_end;

    }
}

bool SettingsWindow::isRedundantPointOfLin(signed char* phi, unsigned int x, unsigned int y)
{
    // if ∃ a neighbor ∈ Lout | ∈ Rout

    if( y > 0 )
    {
        if( phi[ x+(y-1)*img_width ] >= 0 )
        {
            return false; // is not redundant point of Lin
        }
    }
    if( x > 0 )
    {
        if( phi[ (x-1)+y*img_width ] >= 0 )
        {
            return false; // is not redundant point of Lin
        }
    }
    if( x < img_width-1 )
    {
        if( phi[ (x+1)+y*img_width ] >= 0 )
        {
            return false; // is not redundant point of Lin
        }
    }
    if( y < img_height-1 )
    {
        if( phi[ x+(y+1)*img_width ] >= 0 )
        {
            return false; // is not redundant point of Lin
        }
    }

    // ==> ∀ neighbors ∈ Lin | ∈ Rin
    return true; // is redundant point of Lin
}

bool SettingsWindow::isRedundantPointOfLout(signed char* phi, unsigned int x, unsigned int y)
{
    // if ∃ a neighbor ∈ Lin | ∈ Rin

    if( y > 0 )
    {
        if( phi[ x+(y-1)*img_width ] <= 0 )
        {
            return false; // is not redundant point of Lout
        }
    }
    if( x > 0 )
    {
        if( phi[ (x-1)+y*img_width ] <= 0 )
        {
            return false; // is not redundant point of Lout
        }
    }
    if( x < img_width-1 )
    {
        if( phi[ (x+1)+y*img_width ] <= 0 )
        {
            return false; // is not redundant point of Lout
        }
    }
    if( y < img_height-1 )
    {
        if( phi[ x+(y+1)*img_width ] <= 0 )
        {
            return false; // is not redundant point of Lout
        }
    }

    // ==> ∀ neighbors ∈ Lout | ∈ Rout
    return true; // is redundant point of Lout
}

// Fonction appelée dans l'onglet initalisation pour calculer et afficher l'image+phi(couleur foncé)+forme(couleur clair)
void SettingsWindow::shape_visu()
{
    if( shape != nullptr )
    {
        init_width2 = double(width_shape_spin->value())/100.0;
        init_height2 = double(height_shape_spin->value())/100.0;
        center_x2 = double(abscissa_spin->value())/100.0;
        center_y2 = double(ordinate_spin->value())/100.0;

        if( rectangle_radio->isChecked() )
        {
            hasEllipse2 = false;
        }
        if( ellipse_radio->isChecked() )
        {
            hasEllipse2 = true;
        }


        unsigned int offset, x, y; // position

        // efface les listes de la QImage
        for( unsigned int n_out = 0; Lout_shape1[n_out] != list_end; n_out++ )
        {
            offset = Lout_shape1[n_out]*3;

            image_shape_uchar[offset] = image_phi_uchar[offset];
            image_shape_uchar[offset+1] = image_phi_uchar[offset+1];
            image_shape_uchar[offset+2] = image_phi_uchar[offset+2];
        }

        for( unsigned int n_in = 0; Lin_shape1[n_in] != list_end; n_in++ )
        {
            offset = Lin_shape1[n_in]*3;

            image_shape_uchar[offset] = image_phi_uchar[offset];
            image_shape_uchar[offset+1] = image_phi_uchar[offset+1];
            image_shape_uchar[offset+2] = image_phi_uchar[offset+2];
        }

        if( hasEllipse2 )
        {
            // Thanks to Alois Zingl for his ellipse Bresenham's Algorithm implementation
            // http://free.pages.at/easyfilter/bresenham.html

            unsigned int n = 0;
            int x0 = int((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width));
            int y0 = int((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height));
            int x1 = int((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width));
            int y1 = int((double(img_height)-(1.0-init_height2)*double(img_height)/2.0)+ center_y2*double(img_height));

            // values of diameter
            double a = abs(x1-x0);
            int b = abs(y1-y0);

            double b1 = b&1;
            double dx = double(4*(1-a)*b*b);
            double dy = double(4*(b1+1)*a*a);
            double err = dx+dy+double(b1*a*a); // error increment
            double e2; // error of 1.step

            if( x0 > x1 )
            {
                x0 = x1; // if called with swapped points
                x1 += int(a);
            }

            if( y0 > y1 )
            {
                y0 = y1; // .. exchange them
            }

            y0 += (b+1)/2;
            y1 = y0-int(b1); // starting pixel
            a *= 8.0*a;
            b1 = 8.0*double(b)*double(b);

            do
            {
                if( n < 2*img_size-20 )
                {
                    for( int ddy = -2; ddy <= 2; ddy++ )
                    {
                        for( int ddx = -2; ddx <= 2; ddx++ )
                        {
                            shape_points[n++] = x1+ddx+(y0+ddy)*img_width; //   I. Quadrant
                            shape_points[n++] = x0+ddx+(y0+ddy)*img_width; //  II. Quadrant
                            shape_points[n++] = x0+ddx+(y1+ddy)*img_width; // III. Quadrant
                            shape_points[n++] = x1+ddx+(y1+ddy)*img_width; //  IV. Quadrant
                        }
                    }
                }

                e2 = 2.0*err;

                // x step
                if( e2 >= dx )
                {
                    x0++;
                    x1--;
                    dx += b1;
                    err += dx;
                }

                // y step
                if( e2 <= dy )
                {
                    y0++;
                    y1--;
                    dy += a;
                    err += dy;
                }

            }
            while( x0 <= x1 );

            // too early stop of flat ellipses a = 1
            // -> finish tip of ellipse
            while( y0-y1 < b )
            {
                if( n < 2*img_size-20 )
                {
                    for( int ddy = -2; ddy <= 2; ddy++ )
                    {
                        for( int ddx = -2; ddx <= 2; ddx++ )
                        {
                            shape_points[n++] = x0-1+ddx+(y0+ddy)*img_width;
                            shape_points[n++] = x1+1+ddx+(y0+ddy)*img_width;
                            shape_points[n++] = x0-1+ddx+(y1+ddy)*img_width;
                            shape_points[n++] = x1+1+ddx+(y1+ddy)*img_width;
                        }
                    }
                    y0++;
                    y1--;
                }
            }

            // to mark the end;
            shape_points[n] = list_end;

            unsigned int n_out = 0;
            unsigned int n_in = 0;
            for( unsigned int n = 0; shape_points[n] != list_end; n++ )
            {
                offset = shape_points[n];

                y = offset/img_width;
                x = offset-y*img_width;

                if( x < img_width && y < img_height )
                {
                    if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                    {
                        if( (double(y-1)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y-1)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) <= 1.0 )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }

                        if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x-1)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x-1)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) <= 1.0 )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }

                        if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x+1)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x+1)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) <= 1.0 )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }

                        if( (double(y+1)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y+1)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) <= 1.0 )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }
                    }
                    else
                    {
                        if( (double(y-1)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y-1)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }

                        if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x-1)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x-1)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }

                        if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x+1)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x+1)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }

                        if( (double(y+1)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y+1)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }
                    }
                }

            }

            Lout_shape1[n_out] = list_end;
            Lin_shape1[n_in] = list_end;
        }
        else
        {
            int x1, y1;

            unsigned int n = 0;
            for( x = int((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)); x < int((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)); x++ )
            {
                y = int((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height));
                y1 = int((double(img_height)-(1.0-init_height2)*double(img_height)/2.0)+ center_y2*double(img_height));

                if( n < 2*img_size-10 )
                {
                    for( int dy = -2; dy <= 2; dy++ )
                    {
                        for( int dx = -2; dx <= 2; dx++ )
                        {
                            shape_points[n++] = x+dx+(y+dy)*img_width;
                            shape_points[n++] = x+dx+(y1+dy)*img_width;
                        }
                    }
                }
            }

            for( y = int((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)); y < int((double(img_height)-(1.0-init_height2)*double(img_height)/2.0)+ center_y2*double(img_height)); y++ )
            {
                x = int((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width));

                x1 = int((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width));

                if( n < 2*img_size-10 )
                {
                    for( int dy = -2; dy <= 2; dy++ )
                    {
                        for( int dx = -2; dx <= 2; dx++ )
                        {
                            shape_points[n++] = x+dx+(y+dy)*img_width;
                            shape_points[n++] = x1+dx+(y+dy)*img_width;
                        }
                    }
                }
            }

            // to mark the end;
            shape_points[n] = list_end;

            unsigned int n_out = 0;
            unsigned int n_in = 0;
            for( unsigned int n = 0; shape_points[n] != list_end; n++ )
            {
                offset = shape_points[n];

                y = offset/img_width;
                x = offset-y*img_width;

                if( x < img_width && y < img_height )
                {
                    if( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                    {
                        if( !( double(y-1) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y-1) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) ) )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }

                        if( !( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x-1) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x-1) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) ) )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }

                        if( !( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x+1) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x+1) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) ) )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }

                        if( !( double(y+1) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y+1) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) ) )
                        {
                            if( Lin_shape1[n_in] != offset )
                            {
                                Lin_shape1[n_in++] = offset;
                            }
                        }
                    }
                    else
                    {
                        if( double(y-1) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y-1) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }

                        if( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x-1) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x-1) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }

                        if( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x+1) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x+1) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }

                        if( double(y+1) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y+1) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                        {
                            if( Lout_shape1[n_out] != offset )
                            {
                                Lout_shape1[n_out++] = offset;
                            }
                        }
                    }
                }

            }

            Lout_shape1[n_out] = list_end;
            Lin_shape1[n_in] = list_end;
        }

        if( outsidecolor_combobox->currentIndex() != 9 )
        {
            for( int n_out = 0; Lout_shape1[n_out] != list_end; n_out++ )
            {
                offset = Lout_shape1[n_out]*3;

                image_shape_uchar[offset] = Rout2;
                image_shape_uchar[offset+1] = Gout2;
                image_shape_uchar[offset+2] = Bout2;
            }
        }

        if( insidecolor_combobox->currentIndex() != 9 )
        {
            for( int n_in = 0; Lin_shape1[n_in] != list_end; n_in++ )
            {
                offset = Lin_shape1[n_in]*3;

                image_shape_uchar[offset] = Rin2;
                image_shape_uchar[offset+1] = Gin2;
                image_shape_uchar[offset+2] = Bin2;
            }
        }

        imageLabel_settings->set_qimage(image_shape);
    }
}

// Surcharge pour avoir une signature identique (memes paramètres) entre signaux et slots de Qt
void SettingsWindow::shape_visu(int)
{
    shape_visu();
}


// Remet phi_init2_clean a zéro, c'est à dire tout correspond à l'extérieur
void SettingsWindow::clean_phi_visu()
{
    if( phi_init2 != nullptr && phi_init2_clean != nullptr )
    {
        clean_boundaries(phi_init2, phi_init2_clean);
        for( unsigned int offset = 0; offset < img_size; offset++ )
        {
            phi_init2[offset] = 1;
            phi_init2_clean[offset] = 3;
        }
        clean_boundaries(phi_init2, phi_init2_clean);

        shape_visu();

    }
}

// Soustrait une forme à phi_init2
void SettingsWindow::phi_subtract_shape()
{
    if( shape != nullptr && phi_init2 != nullptr )
    {

        unsigned int offset, x, y;

        if( hasEllipse2 )
        {
            for( offset = 0; offset < img_size; offset++ )
            {
                y = offset/img_width;
                x = offset-y*img_width;

                // inéquation de l'ellipse
                // grand axe et petit axe proportionnel à la largeur et la hauteur de l'image
                // pour une image carrée, l'ellipse est un cercle
                // variable init_shape_size par défaut a 0.95 (entre 0 et 1 sinon)
                if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                {
                    shape[offset] = 1;
                }
                else
                {
                    shape[offset] = -1;
                }
            }

        }

        else
        {
            for( offset = 0; offset < img_size; offset++ )
            {
                y = offset/img_width;
                x = offset-y*img_width;

                // pour faire un rectangle
                // largeur et hauteur du rectangle proportionnel à la largeur et la hauteur de l'image
                // pour une image carrée, le rectangle est un carré
                if( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                {
                    shape[offset] = -1;
                }
                else
                {
                    shape[offset] = 1;
                }
            }

        }

        bool phi_init2_modif = false;

        for( offset = 0; offset < img_size; offset++ )
        {
            if( shape[offset] == -1 && phi_init2[offset] == -1 )
            {
                phi_init2[offset] = 1;
                phi_init2_modif = true;
            }
        }

        clean_boundaries(phi_init2, phi_init2_clean);

        if( !phi_init2_modif )
        {
            bool hasOne = false;
            bool hasMinusOne = false;

            for( offset = 0; offset < img_size; offset++ )
            {
                if( shape[offset] == 1 )
                {
                    hasOne = true;
                }
                if( shape[offset] == -1 )
                {
                    hasMinusOne = true;
                }
            }

            if( hasOne && hasMinusOne )
            {
                phi_add_shape();
            }
        }
    }
}

// Ajoute une forme à phi_init2
void SettingsWindow::phi_add_shape()
{
    if( shape != nullptr && phi_init2 != nullptr )
    {
        unsigned int offset, x, y;

        if( hasEllipse2 )
        {
            for( offset = 0; offset < img_size; offset++ )
            {
                y = offset/img_width;
                x = offset-y*img_width;

                // inéquation de l'ellipse
                // grand axe et petit axe proportionnel à la largeur et la hauteur de l'image
                // pour une image carrée, l'ellipse est un cercle
                // variable init_shape_size par défaut a 0.95 (entre 0 et 1 sinon)
                if( (double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)*(double(y)-(1.0+2.0*center_y2)*double(img_height)/2.0)/(init_height2*double(img_height)/2.0*init_height2*double(img_height)/2.0)+(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)*(double(x)-(1.0+2.0*center_x2)*double(img_width)/2.0)/(init_width2*double(img_width)/2.0*init_width2*double(img_width)/2.0) > 1.0 )
                {
                    shape[offset] = 1;
                }
                else
                {
                    shape[offset] = -1;
                }
            }
        }

        else
        {
            for( offset = 0; offset < img_size; offset++ )
            {
                y = offset/img_width;
                x = offset-y*img_width;

                // pour faire un rectangle
                // largeur et hauteur du rectangle proportionnel à la largeur et la hauteur de l'image
                // pour une image carrée, le rectangle est un carré
                if( double(y) > ((1.0-init_height2)*double(img_height)/2.0 + center_y2*double(img_height)) && double(y) < ((double(img_height)-(1.0-init_height2)*double(img_height)/2.0) + center_y2*double(img_height)) && double(x) > ((1.0-init_width2)*double(img_width)/2.0 + center_x2*double(img_width)) && double(x) < ((double(img_width)-(1.0-init_width2)*double(img_width)/2.0) + center_x2*double(img_width)) )
                {
                    shape[offset] = -1;
                }
                else
                {
                    shape[offset] = 1;
                }
            }
        }

        bool phi_init2_modif = false;

        for( offset = 0; offset < img_size; offset++ )
        {
            if( shape[offset] == -1 && phi_init2[offset] == 1 )
            {
                phi_init2[offset] = -1;
                phi_init2_modif = true;
            }
        }

        clean_boundaries(phi_init2, phi_init2_clean);

        if( !phi_init2_modif )
        {
            bool hasOne = false;
            bool hasMinusOne = false;

            for( offset = 0; offset < img_size; offset++ )
            {
                if( shape[offset] == 1 )
                {
                    hasOne = true;
                }
                if( shape[offset] == -1 )
                {
                    hasMinusOne = true;
                }
            }

            if( hasOne && hasMinusOne )
            {
                phi_subtract_shape();
            }
        }
    }
}

// affiche l'image+phi en clair dans la fenêtre de configuration
// dans l'onglet initialisation, affiche l'image+phi en foncé (booléen d'entrée a true) lorsque qu'on clique sur l'image
// ou qu'on appuie sur les bouttons add subtract
void SettingsWindow::phi_visu(bool dark_color)
{
    if( img1 != nullptr )
    {
        if( outsidecolor_combobox->currentIndex() == 8 )
        {
            Rout2 = Rout_selected2;
            Gout2 = Gout_selected2;
            Bout2 = Bout_selected2;
        }
        else
        {
            get_color(outsidecolor_combobox->currentIndex(),Rout2,Gout2,Bout2);
        }

        if( insidecolor_combobox->currentIndex() == 8 )
        {
            Rin2 = Rin_selected2;
            Gin2 = Gin_selected2;
            Bin2 = Bin_selected2;
        }
        else
        {
            get_color(insidecolor_combobox->currentIndex(),Rin2,Gin2,Bin2);
        }


        unsigned int offset;

        if( dark_color )
        {
            if( outsidecolor_combobox->currentIndex() != 9 )
            {
                for( unsigned int n_out = 0; Lout_2[n_out] != list_end; n_out++ )
                {
                    offset = Lout_2[n_out]*3;

                    image_phi_uchar[offset] = Rout2/2;
                    image_phi_uchar[offset+1] = Gout2/2;
                    image_phi_uchar[offset+2] = Bout2/2;

                    image_shape_uchar[offset] = Rout2/2;
                    image_shape_uchar[offset+1] = Gout2/2;
                    image_shape_uchar[offset+2] = Bout2/2;
                }
            }

            if( insidecolor_combobox->currentIndex() != 9 )
            {
                for( unsigned int n_in = 0; Lin_2[n_in] != list_end; n_in++ )
                {
                    offset = Lin_2[n_in]*3;

                    image_phi_uchar[offset] = Rin2/2;
                    image_phi_uchar[offset+1] = Gin2/2;
                    image_phi_uchar[offset+2] = Bin2/2;

                    image_shape_uchar[offset] = Rin2/2;
                    image_shape_uchar[offset+1] = Gin2/2;
                    image_shape_uchar[offset+2] = Bin2/2;
                }
            }
        }


        else
        {
            if( outsidecolor_combobox->currentIndex() != 9 )
            {
                for( unsigned int n_out = 0; Lout_2[n_out] != list_end; n_out++ )
                {
                    offset = Lout_2[n_out]*3;

                    image_phi_uchar[offset] = Rout2;
                    image_phi_uchar[offset+1] = Gout2;
                    image_phi_uchar[offset+2] = Bout2;

                    image_shape_uchar[offset] = Rout2/2;
                    image_shape_uchar[offset+1] = Gout2/2;
                    image_shape_uchar[offset+2] = Bout2/2;
                }
            }
            else
            {
                // efface l'affichage des listes
                for( unsigned int n_out = 0; Lout_2[n_out] != list_end; n_out++ )
                {
                    offset = Lout_2[n_out]*3;

                    image_phi_uchar[offset] = image_filter_uchar[offset];
                    image_phi_uchar[offset+1] = image_filter_uchar[offset+1];
                    image_phi_uchar[offset+2] = image_filter_uchar[offset+2];

                    image_shape_uchar[offset] = image_filter_uchar[offset];
                    image_shape_uchar[offset+1] = image_filter_uchar[offset+1];
                    image_shape_uchar[offset+2] = image_filter_uchar[offset+2];
                }
            }

            if( insidecolor_combobox->currentIndex() != 9 )
            {
                for( unsigned int n_in = 0; Lin_2[n_in] != list_end; n_in++ )
                {
                    offset = Lin_2[n_in]*3;

                    image_phi_uchar[offset] = Rin2;
                    image_phi_uchar[offset+1] = Gin2;
                    image_phi_uchar[offset+2] = Bin2;

                    image_shape_uchar[offset] = Rin2/2;
                    image_shape_uchar[offset+1] = Gin2/2;
                    image_shape_uchar[offset+2] = Bin2/2;
                }
            }
            else
            {
                for( unsigned int n_in = 0; Lin_2[n_in] != list_end; n_in++ )
                {
                    offset = Lin_2[n_in]*3;

                    image_phi_uchar[offset] = image_filter_uchar[offset];
                    image_phi_uchar[offset+1] = image_filter_uchar[offset+1];
                    image_phi_uchar[offset+2] = image_filter_uchar[offset+2];

                    image_shape_uchar[offset] = image_filter_uchar[offset];
                    image_shape_uchar[offset+1] = image_filter_uchar[offset+1];
                    image_shape_uchar[offset+2] = image_filter_uchar[offset+2];
                }
            }
        }

        if( hasContoursHidden && tabs->currentIndex() == 2 )
        {
            // affiche l'ellipse ou le rectangle
            imageLabel_settings->set_qimage(image_filter);
        }
        else
        {
           // affiche l'ellipse ou le rectangle
           imageLabel_settings->set_qimage(image_phi);
        }

        // efface l'ellipse ou le recangle de la QImage
        // efface les listes de la QImage
        for( unsigned int n_out = 0; Lout_shape1[n_out] != list_end; n_out++ )
        {
            offset = Lout_shape1[n_out]*3;

            image_shape_uchar[offset] = image_phi_uchar[offset];
            image_shape_uchar[offset+1] = image_phi_uchar[offset+1];
            image_shape_uchar[offset+2] = image_phi_uchar[offset+2];
        }

        for( unsigned int n_in = 0; Lin_shape1[n_in] != list_end; n_in++ )
        {
            offset = Lin_shape1[n_in]*3;

            image_shape_uchar[offset] = image_phi_uchar[offset];
            image_shape_uchar[offset+1] = image_phi_uchar[offset+1];
            image_shape_uchar[offset+2] = image_phi_uchar[offset+2];
        }
    }
}

void SettingsWindow::get_color(unsigned int index, unsigned char& R, unsigned char& G, unsigned char& B)
{

    if( index == 0 )
    {
        R = 255;
        G = 0;
        B = 0;
    }
    if( index == 1 )
    {
        R = 0;
        G = 255;
        B = 0;
    }
    if( index == 2 )
    {
        R = 0;
        G = 0;
        B = 255;
    }
    if( index == 3 )
    {
        R = 0;
        G = 255;
        B = 255;
    }
    if( index == 4 )
    {
        R = 255;
        G = 0;
        B = 255;
    }
    if( index == 5 )
    {
        R = 255;
        G = 255;
        B = 0;
    }
    if( index == 6 || index == 9 )
    {
        R = 0;
        G = 0;
        B = 0;
    }
    if( index == 7 )
    {
        R = 255;
        G = 255;
        B = 255;
    }
}

// Surcharge de phi_visu pour que le signal et le slot de Qt aient la même signature (les memes types de paramètres)
// pour les combo box couleurs
void SettingsWindow::phi_visu(int)
{
    phi_visu(false);
}

// Fonction appelée quand on clique sur le boutton add dans l'onglet initialization
// ou par clic gauche
void SettingsWindow::add_visu()
{
    // ajoute
    phi_add_shape();
    // visualisation de phi en foncé (booléen true)
    phi_visu(true);
}

// Fonction appelée quand on clique sur le boutton subtract dans l'onglet initialization
// ou par clic droit
void SettingsWindow::subtract_visu()
{
    // soustrait
    phi_subtract_shape();
    // visualisation de phi en foncé (booléen true)
    phi_visu(true);
}


// Fonction appelée par le boutton selected de boundaries outside
// sélection et affichage d'une couleur particulière
void SettingsWindow::color_out()
{
    // Sélection d'une QColor a partir d'une boite de dialogue couleur
    QColor color_out = QColorDialog::getColor(Qt::white, this, tr("Select Lout color"));
    if( color_out.isValid() )
    {
        Rout_selected2 = (unsigned char)(color_out.red());
        Gout_selected2 = (unsigned char)(color_out.green());
        Bout_selected2 = (unsigned char)(color_out.blue());

        QPixmap pm(12,12);
        pm.fill(color_out);
        outsidecolor_combobox->setItemIcon(8,pm);

        outsidecolor_combobox->setCurrentIndex(8);
    }
    // affichage de l'image avec la nouvelle couleur sélectionnée
    phi_visu(false);

}

// Fonction appelée par le boutton selected de boundaries inside
// sélection et affichage d'une couleure particulière
void SettingsWindow::color_in()
{
    // Selection d'une QColor à partir d'une boîte de dialogue couleur
    QColor color_in = QColorDialog::getColor(Qt::white, this, tr("Select Lin color"));
    if( color_in.isValid() )
    {
        Rin_selected2 = (unsigned char)(color_in.red());
        Gin_selected2 = (unsigned char)(color_in.green());
        Bin_selected2 = (unsigned char)(color_in.blue());

        QPixmap pm(12,12);
        pm.fill(color_in);
        insidecolor_combobox->setItemIcon(8,pm);

        insidecolor_combobox->setCurrentIndex(8);
    }
    // affichage de l'image avec la nouvelle couleur sélectionnée
    phi_visu(false);
}

// Fonction appelée par tous les widgets de l'onglet Preprocessing
// Calcule et affiche l'image prétraitéé
void SettingsWindow::filtering_visu()
{
    if( img1 != nullptr && filters2 != nullptr )
    {
        // récupération de tous les états des widgets de l'onglet preprocessing

        if( page3->isChecked() )
        {
            hasPreprocess2 = true;
        }
        else
        {
            hasPreprocess2 = false;
        }

        if( gaussian_noise_groupbox->isChecked() )
        {
            hasGaussianNoise2 = true;
        }
        else
        {
            hasGaussianNoise2 = false;
        }
        std_noise2 = std_noise_spin->value();

        if( salt_noise_groupbox->isChecked() )
        {
            hasSaltNoise2 = true;
        }
        else
        {
            hasSaltNoise2 = false;
        }
        proba_noise2 = proba_noise_spin->value()/100.0;

        if( speckle_noise_groupbox->isChecked() )
        {
            hasSpeckleNoise2 = true;
        }
        else
        {
            hasSpeckleNoise2 = false;
        }
        std_speckle_noise2 = std_speckle_noise_spin->value();

        if( median_groupbox->isChecked() )
        {
            hasMedianFilt2 = true;
        }
        else
        {
            hasMedianFilt2 = false;
        }
        kernel_median_length2 = klength_median_spin->value();
        if( complex_radio2->isChecked() )
        {
            hasO1algo2 = true;
        }
        else
        {
            hasO1algo2 = false;
        }

        if( mean_groupbox->isChecked() )
        {
            hasMeanFilt2 = true;
        }
        else
        {
            hasMeanFilt2 = false;
        }
        kernel_mean_length2 = klength_mean_spin->value();

        if( gaussian_groupbox->isChecked() )
        {
            hasGaussianFilt2 = true;
        }
        else
        {
            hasGaussianFilt2 = false;
        }
        kernel_gaussian_length2 = klength_gaussian_spin->value();
        sigma2 = std_filter_spin->value();


        if( aniso_groupbox->isChecked() )
        {
            hasAnisoDiff2 = true;
        }
        else
        {
            hasAnisoDiff2 = false;
        }

        max_itera2 = iteration_filter_spin->value();
        lambda2 = lambda_spin->value();
        kappa2 = kappa_spin->value();
        if( aniso1_radio->isChecked() )
        {
            aniso_option2 = 1;
        }
        if( aniso2_radio->isChecked() )
        {
            aniso_option2 = 2;
        }

        if( open_groupbox->isChecked() )
        {
            hasOpenFilt2 = true;
        }
        else
        {
            hasOpenFilt2 = false;
        }
        kernel_open_length2 = klength_open_spin->value();

        if( close_groupbox->isChecked() )
        {
            hasCloseFilt2 = true;
        }
        else
        {
            hasCloseFilt2 = false;
        }
        kernel_close_length2 = klength_close_spin->value();

        if( tophat_groupbox->isChecked() )
        {
            hasTophatFilt2 = true;
        }
        else
        {
            hasTophatFilt2 = false;
        }
        if( whitetophat_radio->isChecked() )
        {
            isWhiteTophat2 = true;
        }
        else
        {
            isWhiteTophat2 = false;
        }
        kernel_tophat_length2 = klength_tophat_spin->value();

        if( complex2_morpho_radio->isChecked() )
        {
            hasO1morpho2 = true;
        }
        else
        {
            hasO1morpho2 = false;
        }

        if( chanvese_radio->isChecked() )
        {
            model2 = 1;
        }
        if( geodesic_radio->isChecked() )
        {
            model2 = 2;
        }
        kernel_gradient_length2 = klength_gradient_spin->value();

        alpha2 = Yspin->value();
        beta2 = Uspin->value();
        gamma2 = Vspin->value();

        if( histo_checkbox->isChecked() )
        {
            hasHistoNormaliz2 = true;
        }
        else
        {
            hasHistoNormaliz2 = false;
        }


        if( hasOpenFilt2 || hasCloseFilt2 || hasTophatFilt2 )
        {
            algo_groupbox->setEnabled(true);
        }
        else
        {
            algo_groupbox->setEnabled(false);
        }



        // pour refiltrer à partir de l'image de départ
        filters2->initialyze_filtered();

        double elapsedTime;
        std::clock_t startTime, stopTime;

        startTime = std::clock();

        if( hasPreprocess2 )
        {
            if( hasGaussianNoise2 )
            {
                filters2->gaussian_white_noise(std_noise2);
            }
            if( hasSaltNoise2 )
            {
                filters2->salt_pepper_noise(proba_noise2);
            }
            if( hasSpeckleNoise2 )
            {
                filters2->speckle(std_speckle_noise2);
            }

            if( hasMeanFilt2 )
            {
                filters2->mean_filtering(kernel_mean_length2);
            }
            if( hasGaussianFilt2 )
            {
                filters2->gaussian_filtering(kernel_gaussian_length2, sigma2);
            }

            if( hasMedianFilt2 )
            {
                if( hasO1algo2 )
                {
                    filters2->median_filtering_o1(kernel_median_length2);
                }
                else
                {
                    filters2->median_filtering_oNlogN(kernel_median_length2);
                }
            }
            if( hasAnisoDiff2 )
            {
                filters2->anisotropic_diffusion(max_itera2, lambda2, kappa2, aniso_option2);
            }

            if( hasOpenFilt2 )
            {
                if( hasO1morpho2 )
                {
                    filters2->opening_o1(kernel_open_length2);
                }
                else
                {
                    filters2->opening(kernel_open_length2);
                }
            }

            if( hasCloseFilt2 )
            {
                if( hasO1morpho2 )
                {
                    filters2->closing_o1(kernel_close_length2);
                }
                else
                {
                    filters2->closing(kernel_close_length2);
                }
            }

            if( hasTophatFilt2 )
            {
                if( isWhiteTophat2 )
                {
                    if( hasO1morpho2 )
                    {
                        filters2->white_top_hat_o1(kernel_tophat_length2);
                    }
                    else
                    {
                        filters2->white_top_hat(kernel_tophat_length2);
                    }
                }
                else
                {
                    if( hasO1morpho2 )
                    {
                        filters2->black_top_hat_o1(kernel_tophat_length2);
                    }
                    else
                    {
                        filters2->black_top_hat(kernel_tophat_length2);
                    }
                }
            }

        }

        // assignment in order to prevent a warning in GCC
        const unsigned char* img2_filtered = nullptr;

        if( model2 == 1 )
        {
            img2_filtered = filters2->get_filtered();
        }
        if( model2 == 2 )
        {
            if( isRgb1 )
            {
                filters2->morphological_gradient_yuv(kernel_gradient_length2,alpha2,beta2,gamma2);
                img2_filtered = filters2->get_gradient();
            }
            else
            {
                filters2->morphological_gradient(kernel_gradient_length2);
                img2_filtered = filters2->get_filtered();
            }
        }

        stopTime = std::clock();
        elapsedTime = double(stopTime - startTime) / double(CLOCKS_PER_SEC);
        time_filt->setText(tr("time = ")+QString::number(elapsedTime)+" s");

        if( isRgb1 )
        {


            if( model2 == 1 )
            {
                std::memcpy(image_filter_uchar,img2_filtered,3*img_size);
                std::memcpy(image_phi_uchar,img2_filtered,3*img_size);
                std::memcpy(image_shape_uchar,img2_filtered,3*img_size);
            }
            if( model2 == 2 )
            {
                unsigned char I;

                if( hasHistoNormaliz2 )
                {
                    unsigned char max = 0;
                    unsigned char min = 255;

                    for( unsigned int offset = 0; offset < img_size; offset++ )
                    {
                        if( img2_filtered[offset] > max )
                        {
                            max = img2_filtered[offset];
                        }
                        if( img2_filtered[offset] < min )
                        {
                            min = img2_filtered[offset];
                        }
                    }

                    for( unsigned int offset = 0; offset < img_size; offset++ )
                    {
                        I = (unsigned char)(255.0*double(img2_filtered[offset]-min)/double(max-min));

                        image_filter_uchar[3*offset] = I;
                        image_filter_uchar[3*offset+1] = I;
                        image_filter_uchar[3*offset+2] = I;

                        image_phi_uchar[3*offset] = I;
                        image_phi_uchar[3*offset+1] = I;
                        image_phi_uchar[3*offset+2] = I;

                        image_shape_uchar[3*offset] = I;
                        image_shape_uchar[3*offset+1] = I;
                        image_shape_uchar[3*offset+2] = I;
                    }
                }

                else
                {
                    for( unsigned int offset = 0; offset < img_size; offset++ )
                    {
                        I = img2_filtered[offset];

                        image_filter_uchar[3*offset] = I;
                        image_filter_uchar[3*offset+1] = I;
                        image_filter_uchar[3*offset+2] = I;

                        image_phi_uchar[3*offset] = I;
                        image_phi_uchar[3*offset+1] = I;
                        image_phi_uchar[3*offset+2] = I;

                        image_shape_uchar[3*offset] = I;
                        image_shape_uchar[3*offset+1] = I;
                        image_shape_uchar[3*offset+2] = I;
                    }
                }
            }


        }
        else
        {


            unsigned char I;

            if( model2 == 2 && hasHistoNormaliz2 )
            {
                unsigned char max = 0;
                unsigned char min = 255;

                for( unsigned int offset = 0; offset < img_size; offset++ )
                {
                    if( img2_filtered[offset] > max )
                    {
                        max = img2_filtered[offset];
                    }
                    if( img2_filtered[offset] < min )
                    {
                        min = img2_filtered[offset];
                    }
                }

                for( unsigned int offset = 0; offset < img_size; offset++ )
                {
                    I = (unsigned char)(255.0*double(img2_filtered[offset]-min)/double(max-min));

                    image_filter_uchar[3*offset] = I;
                    image_filter_uchar[3*offset+1] = I;
                    image_filter_uchar[3*offset+2] = I;

                    image_phi_uchar[3*offset] = I;
                    image_phi_uchar[3*offset+1] = I;
                    image_phi_uchar[3*offset+2] = I;

                    image_shape_uchar[3*offset] = I;
                    image_shape_uchar[3*offset+1] = I;
                    image_shape_uchar[3*offset+2] = I;
                }
            }

            else
            {
                for( unsigned int offset = 0; offset < img_size; offset++ )
                {
                    I = img2_filtered[offset];

                    image_filter_uchar[3*offset] = I;
                    image_filter_uchar[3*offset+1] = I;
                    image_filter_uchar[3*offset+2] = I;

                    image_phi_uchar[3*offset] = I;
                    image_phi_uchar[3*offset+1] = I;
                    image_phi_uchar[3*offset+2] = I;

                    image_shape_uchar[3*offset] = I;
                    image_shape_uchar[3*offset+1] = I;
                    image_shape_uchar[3*offset+2] = I;
                }
            }


        }

        phi_visu(false);
    }
}

// Fonction appelée lorsque on change de taille d'image dans l'onglet Display
void SettingsWindow::change_display_size()
{
    if( img1 != nullptr )
    {
        phi_visu(false);
    }
}

// Fonction appelée à chaque changement d'onglet
void SettingsWindow::tab_visu(int value)
{
    if( img1 != nullptr )
    {
        // si onglet initilisation
        if( value == 1 )
        {
            imageLabel_settings->set_doWheelEvent(false);
            // visualisation de la forme
            phi_visu(true);
            shape_visu();
        }
        else
        {
            imageLabel_settings->set_doWheelEvent(true);
            // visualisation de phi
            phi_visu(false);
        }
    }
}

// enlever des trucs de mainwindow
// Filtre des événements pour avoir le tracking au niveau du widget image de la fenêtre principale et de la fenêtre de parametres et pour ne pas avoir le tracking/la position au niveau de l'ensemble de chaque fenêtre
bool SettingsWindow::eventFilter(QObject* object, QEvent* event)
{

    if( (object == scale_spin || object == scale_slider) && event->type() == QEvent::MouseButtonPress )
    {
        if( img1 != nullptr )
        {
            imageLabel_settings->set_hasText(false);
            imageLabel_settings->setBackgroundRole(QPalette::Dark);
        }
        positionX = img_width/2;
        positionY = img_height/2;
    }


    // deplacement uniquement en fonction de imageLabel_settings et pas settings_window
    if( object == imageLabel_settings && event->type() == QEvent::MouseMove )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        mouse_move_event_settings(mouseEvent);
    }

    // clic pris en compte que sur la zone de imageLabel_settings
    if( object == imageLabel_settings && event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        mouse_press_event_settings(mouseEvent);
    }

    if( object == imageLabel_settings && event->type() == QEvent::DragEnter )
    {
        QDragEnterEvent* drag = static_cast<QDragEnterEvent*>(event);
        drag_enter_event_phi(drag);
    }
    if( object == imageLabel_settings && event->type() == QEvent::DragMove )
    {
        QDragMoveEvent* drag = static_cast<QDragMoveEvent*>(event);
        drag_move_event_phi(drag);
    }
    if( object == imageLabel_settings && event->type() == QEvent::Drop )
    {
        QDropEvent* drag = static_cast<QDropEvent*>(event);
        drop_event_phi(drag);
    }
    if( object == imageLabel_settings && event->type() == QEvent::DragLeave )
    {
        QDragLeaveEvent* drag = static_cast<QDragLeaveEvent*>(event);
        drag_leave_event_phi(drag);
    }

    return false;
}

// Evénement de déplacement de la souris dans le widget image de la fenêtre de paramètres
void SettingsWindow::mouse_move_event_settings(QMouseEvent* event)
{
    // si l'image est chargée et si on est dans l'onglet initialisation
    if( img1 != nullptr )
    {
        // position de la souris dans l'image
        positionX = (unsigned int)(double(img_width)*double(   (  (event->pos()).x() -imageLabel_settings->get_xoffset()   ) /double(imageLabel_settings->getPixWidth())));
        positionY = (unsigned int)(double(img_height)*double(   (  (event->pos()).y() -imageLabel_settings->get_yoffset()   ) /double(imageLabel_settings->getPixHeight())));

        if( tabs->currentIndex() == 1 )
        {
            // on en deduit les valeurs relatives des sliders en %

            double a = (double(positionX)-double(img_width/2))/double(img_width);
            double b = (double(positionY)-double(img_height/2))/double(img_height);

            abscissa_spin->setValue(int(a*100.0));
            ordinate_spin->setValue(int(b*100.0));
        }
    }

}

// Evénement clic souris dans le widget image de la fenêtre de paramètres
void SettingsWindow::mouse_press_event_settings(QMouseEvent* event)
{
    // si on est dans l'onglet d'initilisation
    if( (img1 != nullptr) && tabs->currentIndex() == 1 )
    {
        // clic gauche, on ajoute une forme
        if( event->button() == Qt::LeftButton )
        {
            add_visu();
        }
        // clic droit, on soustraie
        if( event->button() == Qt::RightButton )
        {
            subtract_visu();
        }
        // clic du milieu, on change un rectangle en ellipse ou vice versa
        if( event->button() == Qt::MidButton )
        {
            if( !rectangle_radio->isChecked() )
            {
                rectangle_radio->setChecked(true);
            }
            else
            {
                ellipse_radio->setChecked(true);
            }
            shape_visu();
        }
    }

    // pour cacher ou non au clic gauche le contour actif initial quand on est dans l'onglet Preprocessing
    if( (img1 != nullptr) && tabs->currentIndex() == 2 )
    {
        if( event->button() == Qt::RightButton )
        {
            if( hasContoursHidden )
            {
                hasContoursHidden = false;
            }
            else
            {
                hasContoursHidden = true;
            }
            phi_visu(false);
        }
        if( event->button() == Qt::LeftButton )
        {
            hasContoursHidden = true;
            if( hasShowImg1 )
            {
                hasShowImg1 = false;
                img1_visu();
            }
            else
            {
                hasShowImg1 = true;
                phi_visu(false);
            }
        }
    }

 }

// Fonction pour afficher l'image de départ par clic gauche dans l'onglet preprocessing
void SettingsWindow::img1_visu()
{
    if( img1 != nullptr )
    {
        imageLabel_settings->set_qimage(img);
    }
}

void SettingsWindow::drag_enter_event_phi(QDragEnterEvent* event)
{
    if( tabs->currentIndex() != 1 )
    {
        tabs->setCurrentIndex(1);
    }
    QString text(tr("<drop ϕ(t=0)>"));
    imageLabel_settings->set_text(text);
    imageLabel_settings->setBackgroundRole(QPalette::Highlight);
    imageLabel_settings->set_hasText(true);

    event->acceptProposedAction();
    emit changed(event->mimeData());
}

void SettingsWindow::drag_move_event_phi(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void SettingsWindow::drop_event_phi(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if( mimeData->hasUrls() )
    {
        QList<QUrl> urlList = mimeData->urls();
        fileName_phi = urlList.first().toLocalFile();
    }
    imageLabel_settings->setBackgroundRole(QPalette::Dark);
    open_phi();
    event->acceptProposedAction();
}

void SettingsWindow::drag_leave_event_phi(QDragLeaveEvent* event)
{
    QString text(tr("<drag ϕ(t=0)>"));
    imageLabel_settings->set_text(text);
    imageLabel_settings->setBackgroundRole(QPalette::Dark);
    imageLabel_settings->set_hasText(true);

    emit changed();
    event->accept();
}

void SettingsWindow::openFilenamePhi()
{
    fileName_phi = QFileDialog::getOpenFileName(this,
                                            tr("Open File"),
                                            last_directory_used,
                                            tr("Image Files (%1)").arg(nameFilters.join(" ")));

    open_phi();
}

void SettingsWindow::open_phi()
{
    if( img1 != nullptr )
    {
        if( !fileName_phi.isEmpty() )
        {
            QFileInfo fi(fileName_phi);
            last_directory_used = fi.absolutePath();

            QImage img_phi1(fileName_phi);

            if( img_phi1.isNull() )
            {
                QMessageBox::information(this, tr("Opening error - Ofeli"),
                                         tr("Cannot load %1.").arg(fileName_phi));
                return;
            }

            img_phi1 = img_phi1.scaled(img_width, img_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);

            int histogram[256];
            for( unsigned char I = 0; I <= 255; I++ )
            {
                histogram[I] = 0;
            }

            if( img_phi1.format() == 3 )
            {

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        histogram[ *( img_phi1.scanLine(y)+x ) ]++;
                    }
                }

                const unsigned char threshold = otsu_method(histogram,img_size);

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        if( *( img_phi1.scanLine(y)+x ) <= threshold )
                        {
                            phi_init2[ find_offset(x,y) ] = 1;
                        }
                        else
                        {
                            phi_init2[ find_offset(x,y) ] = -1;
                        }
                    }
                }

            }
            else
            {
                QRgb pix;

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        pix = img_phi1.pixel(x,y);

                        histogram[ (unsigned char) ( 0.2989*(double)(qRed(pix))
                                                     + 0.5870*(double)(qGreen(pix))
                                                     + 0.1140*(double)(qBlue(pix)) ) ]++;
                    }
                }

                const unsigned char threshold = otsu_method(histogram,img_size);

                for( unsigned int y = 0; y < img_height; y++ )
                {
                    for( unsigned int x = 0; x < img_width; x++ )
                    {
                        pix = img_phi1.pixel(x,y);

                        if( (unsigned char) ( 0.2989*(double)(qRed(pix))
                                              + 0.5870*(double)(qGreen(pix))
                                              + 0.1140*(double)(qBlue(pix)) ) <= threshold )
                        {
                            phi_init2[ find_offset(x,y) ] = 1;
                        }
                        else
                        {
                            phi_init2[ find_offset(x,y) ] = -1;
                        }
                    }
                }
            }
            clean_boundaries(phi_init2,phi_init2_clean);
        }

        tab_visu(tabs->currentIndex());
    }
}

void SettingsWindow::save_phi()
{
    if( phi_init2_clean != nullptr )
    {
        QImage img_phi_save((unsigned char*)(phi_init2_clean),img_width,img_height, img_width, QImage::Format_Indexed8);
        QVector<QRgb> table(256);
        for( int I = 0; I < 256; I++ )
        {
            table[I] = qRgb(I,I,I);
        }
        img_phi_save.setColorTable(table);

        QString fileName_save = QFileDialog::getSaveFileName(this,
                                                             tr("Save ϕ(t=0)"),
                                                             last_directory_used + QString(tr("/initial_phi")),
                                                             "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;TIFF (*.tiff);;XBM (*.xbm);;XPM (*.xpm)");
        // sauvegarde
        img_phi_save.save(fileName_save);
    }
}

void SettingsWindow::phiInit2imgPhi()
{
    if( phi_init1_clean != nullptr )
    {
        img_phi = QImage(img_width,img_height, QImage::Format_Indexed8);
        QVector<QRgb> table(256);
        for( int I = 0; I < 256; I++ )
        {
            table[I] = qRgb(I,I,I);
        }
        img_phi.setColorTable(table);

        for( unsigned int y = 0; y < img_height; y++ )
        {
            for( unsigned int x = 0; x < img_width; x++ )
            {
                if( phi_init1_clean[ find_offset(x,y) ] < 0 )
                {
                    *( img_phi.scanLine(y)+x ) = 255;
                }
                else
                {
                    *( img_phi.scanLine(y)+x ) = 0;
                }
            }
        }

    }
}

void SettingsWindow::imgPhi2phiInit()
{
    if( phi_init1_clean != nullptr && phi_init2 != nullptr && !img_phi.isNull() )
    {
        if( img_phi.format() == 3 )
        {

            for( unsigned int y = 0; y < img_height; y++ )
            {
                for( unsigned int x = 0; x < img_width; x++ )
                {
                    if( *( img_phi.scanLine(y)+x ) < 128 )
                    {
                        phi_init1_clean[ find_offset(x,y) ] = 1;
                    }
                    else
                    {
                        phi_init1_clean[ find_offset(x,y) ] = -1;
                    }
                }
            }

        }
        else
        {
            QRgb pix;

            for( unsigned int y = 0; y < img_height; y++ )
            {
                for( unsigned int x = 0; x < img_width; x++ )
                {
                    pix = img_phi.pixel(x,y);

                    if( (0.2989*(double)(qRed(pix))
                         + 0.5870*(double)(qGreen(pix))
                         + 0.1140*(double)(qBlue(pix)) ) < 128.0 )
                    {
                        phi_init1_clean[ find_offset(x,y) ] = 1;
                    }
                    else
                    {
                        phi_init1_clean[ find_offset(x,y) ] = -1;
                    }
                }
            }

        }

        std::memcpy(phi_init2,phi_init1_clean,img_size);
        clean_boundaries(phi_init2,phi_init1_clean);
        std::memcpy(phi_init2_clean,phi_init1_clean,img_size);
    }
}

void SettingsWindow::adjustVerticalScroll_settings(int min, int max)
{
    if( img_height != 0 )
    {
        scrollArea_settings->verticalScrollBar()->setValue( (max-min)*positionY/img_height );
    }
}

void SettingsWindow::adjustHorizontalScroll_settings(int min, int max)
{
    if( img_width != 0 )
    {
        scrollArea_settings->horizontalScrollBar()->setValue( (max-min)*positionX/img_width );
    }
}

unsigned char SettingsWindow::otsu_method(const int histogram[], unsigned int img_size)
{
    unsigned int sum = 0;
    for( unsigned char I = 0; I <= 255; I++ )
    {
        sum += I*histogram[I];
    }

    unsigned int weight1, weight2, sum1;
    double mean1, mean2, var_t, var_max;

    unsigned char threshold = 127; // value returned in the case of an totally homogeneous image

    weight1 = 0;
    sum1 = 0;
    var_max = -1.0;

    // 256 values ==> 255 thresholds t evaluated
    // class1 <= t and class2 > t
    for( unsigned char t = 0; t < 255; t++ )
    {
        weight1 += histogram[t];
        if( weight1 == 0 )
        {
            continue;
        }

        weight2 = img_size-weight1;
        if( weight2 == 0 )
        {
            break;
        }

        sum1 += t*histogram[t];

        mean1 = double(sum1/weight1);
        mean2 = double( (sum-sum1)/weight2 ); // sum2 = sum-sum1

        var_t = double(weight1)*double(weight2)*(mean1-mean2)*(mean1-mean2);

        if( var_t > var_max )
        {
            var_max = var_t;
            threshold = t;
        }
    }

    return threshold;
}

void SettingsWindow::wheel_zoom(int val, ofeli::ScrollAreaWidget* obj)
{
    if( obj == scrollArea_settings && img1 != nullptr )
    {
        imageLabel_settings->set_hasText(false);
        imageLabel_settings->setBackgroundRole(QPalette::Dark);

        if ( tabs->currentIndex() != 1 )
        {
            float value = 0.002f*float( val ) + imageLabel_settings->get_zoomFactor();

            if( value < 32.0f/float( imageLabel_settings->get_qimage().width() ) )
            {
                value = 32.0f/float( imageLabel_settings->get_qimage().width() );
            }

            scale_spin->setValue( int(100.0f*value) );
        }

        // si une image est déja chargée et si on est dans l'onglet d'initialisation
        if( tabs->currentIndex() == 1 )
        {
            // nombre de degré et de pas de la molette
            int numDegrees = val / 8;
            int numSteps = numDegrees / 15;

            // récupération des valeurs des sliders concernant la taille de la forme
            int width_ratio = width_shape_spin->value();
            int height_ratio = height_shape_spin->value();

            // correpondance par rapport à la taille réelle de la forme
            double width = double(img_width)*double(width_ratio)/100.0;
            double height = double(img_height)*double(height_ratio)/100.0;

            // augmentation ou diminution de 15% à chaque pas de molette
            width += 0.15*double(numSteps)*width;
            height += 0.15*double(numSteps)*height;

            width_ratio = int( 100.0*width/double(img_width) );
            height_ratio = int( 100.0*height/double(img_height) );

            if( (width_ratio > 3) && (height_ratio > 3) && (width_ratio < 150) && (height_ratio < 150) )
            {
                width_shape_spin->setValue(width_ratio);
                height_shape_spin->setValue(height_ratio);
            }
        }
    }
}

void SettingsWindow::save_settings() const
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue("Settings/Window/size", size());
    settings.setValue("Settings/Window/position", pos());

    settings.setValue("Settings/Algorithm/Na", mainwindow_settings.Na1);
    settings.setValue("Settings/Algorithm/Ns", mainwindow_settings.Ns1);
    settings.setValue("Settings/Algorithm/model", mainwindow_settings.model);
    settings.setValue("Settings/Algorithm/isRgb", isRgb1);
    settings.setValue("Settings/Algorithm/lambda_out", mainwindow_settings.lambda_out1);
    settings.setValue("Settings/Algorithm/lambda_in", mainwindow_settings.lambda_in1);
    settings.setValue("Settings/Algorithm/kernel_gradient_length", mainwindow_settings.kernel_gradient_length1);
    settings.setValue("Settings/Algorithm/alpha", mainwindow_settings.alpha1);
    settings.setValue("Settings/Algorithm/beta", mainwindow_settings.beta1);
    settings.setValue("Settings/Algorithm/gamma", mainwindow_settings.gamma1);
    settings.setValue("Settings/Algorithm/hasSmoothingCycle", mainwindow_settings.hasSmoothingCycle1);
    settings.setValue("Settings/Algorithm/kernel_curve", mainwindow_settings.kernel_curve1);
    settings.setValue("Settings/Algorithm/std_curve", mainwindow_settings.std_curve1);

    settings.setValue("Settings/Initialization/img_phi", img_phi);
    settings.setValue("Settings/Initialization/hasEllipse", hasEllipse1);
    settings.setValue("Settings/Initialization/init_width", init_width1);
    settings.setValue("Settings/Initialization/init_height", init_height1);
    settings.setValue("Settings/Initialization/center_x", center_x1);
    settings.setValue("Settings/Initialization/center_y", center_y1);

    settings.setValue("Settings/Preprocessing/hasPreprocess", mainwindow_settings.hasPreprocess);
    settings.setValue("Settings/Preprocessing/hasGaussianNoise", mainwindow_settings.hasGaussianNoise);
    settings.setValue("Settings/Preprocessing/std_noise", mainwindow_settings.std_noise);
    settings.setValue("Settings/Preprocessing/hasSaltNoise", mainwindow_settings.hasSaltNoise);
    settings.setValue("Settings/Preprocessing/proba_noise", mainwindow_settings.proba_noise);
    settings.setValue("Settings/Preprocessing/hasSpeckleNoise", mainwindow_settings.hasSpeckleNoise);
    settings.setValue("Settings/Preprocessing/std_speckle_noise", mainwindow_settings.std_speckle_noise);

    settings.setValue("Settings/Preprocessing/hasMedianFilt", mainwindow_settings.hasMedianFilt);
    settings.setValue("Settings/Preprocessing/kernel_median_length", mainwindow_settings.kernel_median_length1);
    settings.setValue("Settings/Preprocessing/hasO1algo", mainwindow_settings.hasO1algo1);
    settings.setValue("Settings/Preprocessing/hasMeanFilt", mainwindow_settings.hasMeanFilt);
    settings.setValue("Settings/Preprocessing/kernel_mean_length", mainwindow_settings.kernel_mean_length1);
    settings.setValue("Settings/Preprocessing/hasGaussianFilt", mainwindow_settings.hasGaussianFilt);
    settings.setValue("Settings/Preprocessing/kernel_gaussian_length", mainwindow_settings.kernel_gaussian_length1);
    settings.setValue("Settings/Preprocessing/sigma", mainwindow_settings.sigma);

    settings.setValue("Settings/Preprocessing/hasAnisoDiff", mainwindow_settings.hasAnisoDiff);
    settings.setValue("Settings/Preprocessing/aniso_option", mainwindow_settings.aniso_option1);
    settings.setValue("Settings/Preprocessing/max_itera", mainwindow_settings.max_itera1);
    settings.setValue("Settings/Preprocessing/lambda", mainwindow_settings.lambda1);
    settings.setValue("Settings/Preprocessing/kappa", mainwindow_settings.kappa1);

    settings.setValue("Settings/Preprocessing/hasOpenFilt", mainwindow_settings.hasOpenFilt);
    settings.setValue("Settings/Preprocessing/kernel_open_length", mainwindow_settings.kernel_open_length1);

    settings.setValue("Settings/Preprocessing/hasCloseFilt", mainwindow_settings.hasCloseFilt);
    settings.setValue("Settings/Preprocessing/kernel_close_length", mainwindow_settings.kernel_close_length1);

    settings.setValue("Settings/Preprocessing/hasTophatFilt", mainwindow_settings.hasTophatFilt);
    settings.setValue("Settings/Preprocessing/isWhiteTophat", mainwindow_settings.isWhiteTophat);
    settings.setValue("Settings/Preprocessing/kernel_tophat_length", mainwindow_settings.kernel_tophat_length1);

    settings.setValue("Settings/Preprocessing/hasO1morpho", mainwindow_settings.hasO1morpho1);

    if( parent() != nullptr)
    {
        settings.setValue("Settings/Display/zoom_factor", static_cast<ofeli::MainWindow*>(parent())->get_zoom_factor());
    }
    settings.setValue("Settings/Display/hasHistoNormaliz", mainwindow_settings.hasHistoNormaliz);
    settings.setValue("Settings/Display/hasDisplayEach", mainwindow_settings.hasDisplayEach);

    settings.setValue("Settings/Display/inside_combo", mainwindow_settings.inside_combo);
    settings.setValue("Settings/Display/outside_combo", mainwindow_settings.outside_combo);

    settings.setValue("Settings/Display/Rout_selected", (unsigned int)(mainwindow_settings.Rout_selected1));
    settings.setValue("Settings/Display/Gout_selected", (unsigned int)(mainwindow_settings.Gout_selected1));
    settings.setValue("Settings/Display/Bout_selected", (unsigned int)(mainwindow_settings.Bout_selected1));
    settings.setValue("Settings/Display/Rin_selected", (unsigned int)(mainwindow_settings.Rin_selected1));
    settings.setValue("Settings/Display/Gin_selected", (unsigned int)(mainwindow_settings.Gin_selected1));
    settings.setValue("Settings/Display/Bin_selected", (unsigned int)(mainwindow_settings.Bin_selected1));
}

}
