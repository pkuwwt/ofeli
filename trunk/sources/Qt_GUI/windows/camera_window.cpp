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

#include "camera_window.hpp"
#include "camera_interface.hpp"
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"
#include "settings_window.hpp"

#include <QCamera>
#include <QCameraViewfinder>
#include <QtWidgets>

namespace ofeli
{

CameraWindow::CameraWindow(QWidget* parent, ofeli::SettingsWindow* settings_window1) :
    QDialog(parent), camera(nullptr), imageLabel(nullptr)
{
    QSettings settings("Bessy", "Ofeli");

    setWindowTitle( tr("Camera") );
    resize( settings.value("Camera/Window/size",
                           QSize(650, 400)).toSize() );

    move( settings.value("Camera/Window/position",
                         QPoint(200, 200)).toPoint() );

    scrollArea = new ofeli::ScrollAreaWidget(this);
    imageLabel = new ofeli::PixmapWidget(scrollArea);
    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    imageLabel->setFocusPolicy(Qt::StrongFocus);
    imageLabel->setMouseTracking(true);
    imageLabel->installEventFilter(this);
    imageLabel->resize(280, 487);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->set_text(tr(" "));

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->installEventFilter(this);
    connect(scrollArea->verticalScrollBar(),
            SIGNAL(rangeChanged(int,int)), this,
            SLOT(adjustVerticalScroll(int,int)));

    connect(scrollArea->horizontalScrollBar(),
            SIGNAL(rangeChanged(int,int)), this,
            SLOT(adjustHorizontalScroll(int,int)));

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
    QHBoxLayout* scale_layout = new QHBoxLayout;
    scale_layout->addWidget(scale_slider0,1);
    scale_layout->addLayout(scale_spin_layout,0);
    QGroupBox* scale_group = new QGroupBox(tr("Display"));
    scale_group->setLayout(scale_layout);

    QVBoxLayout* layout_this = new QVBoxLayout;
    layout_this->addWidget(scrollArea,1);
    layout_this->addWidget(scale_group,0);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(layout_this);

    connect(scale_spin0, SIGNAL(valueChanged(int)), this,
            SLOT(do_scale0(int)));
    connect(scale_slider0, SIGNAL(valueChanged(int)), scale_spin0,
            SLOT(setValue(int)));
    connect(scale_spin0, SIGNAL(valueChanged(int)), scale_slider0,
            SLOT(setValue(int)));

    scale_spin0->setValue(settings.value("Camera/Display/zoom_factor", 100).toInt());
    imageLabel->set_zoomFactor(float(scale_spin0->value())/100.0f);

    camera = new QCamera;
    camera_interface = new ofeli::CameraInterface(this,imageLabel,settings_window1);
    camera->setViewfinder(camera_interface);
}

void CameraWindow::show_camera()
{
    if( !isVisible() )
    {
        show();
        camera->start();
    }
}

void CameraWindow::restart()
{
    camera->stop();
    camera_interface->delete_ac();
    if( !isVisible() )
    {
        show();
    }
    camera->start();
}

void CameraWindow::closeEvent(QCloseEvent*)
{
    camera->stop();
    camera_interface->delete_ac();
}

void CameraWindow::do_scale0(int value)
{
    if( imageLabel != nullptr )
    {
        imageLabel->setZoomFactor(float(value)/100.0);
    }
}

void CameraWindow::wheel_zoom(int val, ofeli::ScrollAreaWidget* obj)
{
    if( obj == scrollArea )
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

void CameraWindow::adjustVerticalScroll(int min, int max)
{
    if( img_height != 0 )
    {
        scrollArea->verticalScrollBar()->setValue(
                    (max-min)*positionY/img_height );
    }
}

void CameraWindow::adjustHorizontalScroll(int min, int max)
{
    if( img_width != 0 )
    {
        scrollArea->horizontalScrollBar()->setValue(
                    (max-min)*positionX/img_width );
    }
}

bool CameraWindow::eventFilter(QObject* object, QEvent* event)
{
    // deplacement uniquement en fonction de imageLabel et pas main_window
    if( object == imageLabel && event->type() == QEvent::MouseMove )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseEvent);
    }

    if( (object == scale_spin0 || object == scale_slider0) && event->type() == QEvent::MouseButtonPress )
    {
        //if( img1 != nullptr )
        //{
            imageLabel->set_hasText(false);
            imageLabel->setBackgroundRole(QPalette::Dark);
        //}
        positionX = img_width/2;
        positionY = img_height/2;
    }
    return false;
}

void CameraWindow::mouseMoveEvent(QMouseEvent* event)
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
}

void CameraWindow::set_img_width(unsigned int img_width1)
{
    img_width = img_width1;
}

void CameraWindow::set_img_height(unsigned int img_height1)
{
    img_height = img_height1;
}

void CameraWindow::save_settings() const
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue("Camera/Window/size", size());
    settings.setValue("Camera/Window/position", pos());

    settings.setValue("Camera/Display/zoom_factor", scale_spin0->value());
}

}
