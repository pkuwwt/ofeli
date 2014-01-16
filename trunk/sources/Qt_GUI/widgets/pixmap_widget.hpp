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

#ifndef PIXMAP_WIDGET_HPP
#define PIXMAP_WIDGET_HPP

#include <QLabel>

class QImage;

namespace ofeli
{

class PixmapWidget : public QLabel
{

    Q_OBJECT

public :

    PixmapWidget( QWidget* parent = nullptr );
    void set_qimage0(const QImage& qimage1);
    void set_qimage(const QImage& qimage1);
    void set_doWheelEvent(bool doWheelEvent1);
    void set_zoomFactor(float zoomFactor1);
    float get_zoomFactor() const;
    int getPixWidth() const;
    int getPixHeight() const;
    int get_xoffset() const;
    int get_yoffset() const;
    void set_hasText(bool hasText1);
    const QImage& get_qimage() const { return qimage; }
    void set_text(QString text1);

public slots :

    void setZoomFactor(float f);

signals :

    void zoomFactorChanged(float f);

protected :

    virtual void paintEvent(QPaintEvent* event) override;

private :

    QImage qimage;
    float zoomFactor;
    int xoffset;
    int yoffset;
    bool hasText;
    QString text;
    bool doWheelEvent;
};

}

#endif // PIXMAP_WIDGET_HPP

//! \class ofeli::PixmapWidget
//! The class PixmapWidget is a Qt4 widget to display fastly an image (function drawPixmap in a PaintEvent is faster than function setPixmap) and gives the possibility to zoom with a mouse wheel event in function of the position of the cursor.
//! Thanks to Johan Thelin. This class is based on this example http://qt4.digitalfanatics.org/articles/zoomer.html.
