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

// Widget to display fastly an image (function drawPixmap in a PaintEvent is faster than function setPixmap)
// Thanks to Johan Thelin.
// This class is based on this example http://qt4.digitalfanatics.org/articles/zoomer.html .

#include "pixmap_widget.hpp"
#include <QPainter>

namespace ofeli
{

PixmapWidget::PixmapWidget(QWidget* parent) :
    QLabel(parent),
    zoomFactor(1.2f), hasText(true), text(QString(tr("<drag image>"))),
    doWheelEvent(false)
{
    setMinimumSize( qimage.width()*zoomFactor, qimage.height()*zoomFactor );
}

void PixmapWidget::set_qimage0(const QImage& qimage1)
{
    hasText = false;
    doWheelEvent = true;
    qimage = qimage1;

    int w, h;
    w = qimage.width()*zoomFactor;
    h = qimage.height()*zoomFactor;
    setMinimumSize(w,h);

    QWidget* p = dynamic_cast<QWidget*>( parent() );
    if( p != nullptr )
    {
        resize( p->width(), p->height() );
    }

    update();
}

void PixmapWidget::set_qimage(const QImage& qimage1)
{
    hasText = false;
    qimage = qimage1;

    update();
}

void PixmapWidget::setZoomFactor(float f)
{
    if( !hasText )
    {
        int w, h;

        if( f == zoomFactor )
        {
            return;
        }

        zoomFactor = f;
        emit( zoomFactorChanged(zoomFactor) );

        w = qimage.width()*zoomFactor;
        h = qimage.height()*zoomFactor;
        setMinimumSize(w,h);

        QWidget* p = dynamic_cast<QWidget*>( parent() );
        if( p == nullptr )
        {
            resize( p->width(), p->height() );
        }

        update();
    }
}

void PixmapWidget::paintEvent(QPaintEvent*)
{

    if( width() > qimage.width()*zoomFactor )
    {
        xoffset = (width()-qimage.width()*zoomFactor)/2;
    }
    else
    {
        xoffset = 0;
    }

    if( height() > qimage.height()*zoomFactor )
    {
        yoffset = (height()-qimage.height()*zoomFactor)/2;
    }
    else
    {
        yoffset = 0;
    }

    QPainter p(this);
    p.save();

    if( hasText )
    {
        p.drawText(width()/2-40, height()/2,text);
    }
    else
    {
        p.translate(xoffset,yoffset);
        p.scale(zoomFactor,zoomFactor);
        //p.drawImage(0,0,qimage); // faster with Qt 4
        p.drawPixmap(0,0,QPixmap::fromImage(qimage)); // faster with Qt 5
    }

    p.restore();
}

int PixmapWidget::getPixWidth() const
{
    return int( float( qimage.width() ) * zoomFactor );
}

int PixmapWidget::getPixHeight() const
{
    return int( float( qimage.height() ) * zoomFactor );
}

int PixmapWidget::get_xoffset() const
{
    return xoffset;
}

int PixmapWidget::get_yoffset() const
{
    return yoffset;
}

void PixmapWidget::set_hasText(bool hasText1)
{
    hasText = hasText1;
}

void PixmapWidget::set_text(QString text1)
{
    text = text1;
}

void PixmapWidget::set_zoomFactor(float zoomFactor1)
{
    zoomFactor = zoomFactor1;
}

float PixmapWidget::get_zoomFactor() const
{
    return zoomFactor;
}

void PixmapWidget::set_doWheelEvent(bool doWheelEvent1)
{
    doWheelEvent = doWheelEvent1;
}

}
