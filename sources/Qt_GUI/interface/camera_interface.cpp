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

#include "camera_interface.hpp"
#include "pixmap_widget.hpp"
#include "tracking_active_contour.hpp"
#include "list.hpp"
#include "camera_window.hpp"
#include "settings_window.hpp"

namespace ofeli
{

CameraInterface::CameraInterface(QObject* parent, ofeli::PixmapWidget* widget1, ofeli::SettingsWindow* settings_window1) :
    QAbstractVideoSurface(parent), widget(widget1), ac(nullptr), settings_window(settings_window1)
{
    widget->set_zoomFactor(1.0);
}

QList<QVideoFrame::PixelFormat> CameraInterface::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType type) const
{
    Q_UNUSED(type);

    return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_BGRA32
            << QVideoFrame::Format_BGRA32_Premultiplied
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_BGR24;
}

bool CameraInterface::present(const QVideoFrame& frame)
{
    Q_UNUSED(frame);
    QVideoFrame my_frame = frame;
    my_frame.map(QAbstractVideoBuffer::ReadWrite);

    QImage::Format img_format;
    if( my_frame.pixelFormat() == QVideoFrame::Format_ARGB32 )
    {
        img_format = QImage::Format_ARGB32;
    }
    else if( my_frame.pixelFormat() == QVideoFrame::Format_ARGB32_Premultiplied )
    {
        img_format = QImage::Format_ARGB32_Premultiplied;
    }
    else if( my_frame.pixelFormat() == QVideoFrame::Format_RGB32 )
    {
        img_format = QImage::Format_RGB32;
    }
    else if( my_frame.pixelFormat() == QVideoFrame::Format_RGB24 )
    {
        img_format = QImage::Format_RGB888;
    }
    else if( my_frame.pixelFormat() == QVideoFrame::Format_RGB565 )
    {
        img_format = QImage::Format_RGB16;
    }
    else if( my_frame.pixelFormat() == QVideoFrame::Format_RGB555 )
    {
        img_format = QImage::Format_RGB555;
    }
    else
    {
        img_format = QImage::Format_Invalid;
    }

    QImage img_rgb1(my_frame.bits(),
           my_frame.width(), my_frame.height(), my_frame.bytesPerLine(),
           img_format);

    auto s = settings_window->get_settings();

    if( ac == nullptr )
    {
        if( widget == nullptr )
        {
            return false;
        }

        ac = new ofeli::ACwithoutEdgesYUVcamera(img_rgb1,
                                                true, 0.65, 0.65, 0.0, 0.0,
                                                s.hasSmoothingCycle1, s.kernel_curve1, s.std_curve1, s.Na1, s.Ns1,
                                                s.lambda_out1, s.lambda_in1, s.alpha1, s.beta1, s.gamma1);

        Lout_color = qRgb(int(s.Rout1),int(s.Gout1),int(s.Bout1));
        Lin_color = qRgb(int(s.Rin1),int(s.Gin1),int(s.Bin1));

        static_cast<ofeli::CameraWindow*>( parent() )->set_img_width( my_frame.width() );
        static_cast<ofeli::CameraWindow*>( parent() )->set_img_height( my_frame.height() );
    }

    ac->initialize_for_each_frame_set_buffer(img_rgb1);

    const unsigned int it_max = 3*(s.Na1+s.Ns1);
    for( unsigned int iteration = 0; iteration < it_max; iteration++ )
    {
        ac->evolve_one_iteration();
    }

    unsigned int x, y;
    for( auto it = ac->get_Lout().begin(); !it.end(); ++it )
    {
        // *it is the offset
        ac->get_phi().get_position(*it,x,y); // x and y passed by reference
        img_rgb1.setPixel(x,y,Lout_color);
    }

    for( auto it = ac->get_Lin().begin(); !it.end(); ++it )
    {
        // *it is the offset
        ac->get_phi().get_position(*it,x,y); // x and y passed by reference
        img_rgb1.setPixel(x,y,Lin_color);
    }

    widget->set_qimage(img_rgb1);
    return true;
}

void CameraInterface::delete_ac()
{
    if( ac != nullptr )
    {
        delete ac;
    }
    ac = nullptr;
}

}
