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

#include "scroll_area_widget.hpp"
#include <QScrollBar>
#include <QObject>
#include <QWheelEvent>

namespace ofeli
{

ScrollAreaWidget::ScrollAreaWidget(QWidget* parent)
    : QScrollArea(parent)
{
    verticalScrollBar()->installEventFilter(this);
    horizontalScrollBar()->installEventFilter(this);
    connect( this, SIGNAL(changed(int,ScrollAreaWidget*)), parentWidget(),
             SLOT(wheel_zoom(int,ScrollAreaWidget*)) );
}

bool ScrollAreaWidget::eventFilter(QObject* obj, QEvent* event)
{
    if( event->type() == QEvent::Wheel )
    {
        if( obj == verticalScrollBar() || obj == horizontalScrollBar() )
        {
            return QScrollArea::eventFilter(obj,event);
        }
        else
        {
            QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
            emit changed( wheel_event->delta(), this );

            return true;
        }
    }
    else
    {
        return QScrollArea::eventFilter(obj,event);
    }
}

}
