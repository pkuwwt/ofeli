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

#include "evaluation_widget.hpp"
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"
#include <QtWidgets>

namespace ofeli
{

int EvaluationWidget::count_this = 0;

EvaluationWidget::EvaluationWidget(QWidget *parent) :
    QWidget(parent),
    img_width(0), img_height(0), list(nullptr), list_length(0)
{
    count_this++; // static variable to count the instances
    id_this = count_this; // in order to know if *this is the first or the second widget of evaluation_window

    QSettings settings("Bessy", "Ofeli");

    text_list_length = new QLabel(this);
    text_list_length->setAlignment(Qt::AlignCenter);
    if( id_this == 1 )
    {
        text_list_length->setText("<font color=red>"+tr("List 1 length = ")+QString::number(list_length));
    }
    if( id_this == 2 )
    {
        text_list_length->setText("<font color=red>"+tr("List 2 length = ")+QString::number(list_length));
    }

    /////////////////////////////////////////////////////////////////////////////////

    name_label = new QLabel(this);
    name_label->setText( tr("Title - Size") );
    name_label->setAlignment(Qt::AlignCenter);

    ///////////////////////////////////////

    area = new ofeli::ScrollAreaWidget(this);
    img_disp = new ofeli::PixmapWidget(area);
    img_disp->setBackgroundRole(QPalette::Dark);
    img_disp->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    img_disp->setScaledContents(true);
    img_disp->setMouseTracking(true);
    img_disp->installEventFilter(this);
    img_disp->setAcceptDrops(true);
    img_disp->resize(200, 200);
    img_disp->setAlignment(Qt::AlignCenter);
    img_disp->set_text( tr("<drag image") + QString::number(id_this) + ">" );
    area->setBackgroundRole(QPalette::Dark);
    area->setWidget(img_disp);
    area->setAlignment(Qt::AlignCenter);
    area->setWidgetResizable(true);
    connect( area->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
             this, SLOT(adjust_vertical_scroll(int,int)) );
    connect( area->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)), this,
             SLOT(adjust_horizontal_scroll(int,int)) );

    ///////////////////////////////////////

    scale_spin = new QSpinBox;
    scale_spin->setSingleStep(25);
    scale_spin->setMinimum(1);
    scale_spin->setMaximum(5000);
    scale_spin->setSuffix(" %");
    scale_spin->setValue( settings.value("Evaluation/spin"
                                         +QString::number(id_this), 100).toInt() );
    connect( scale_spin, SIGNAL(valueChanged(int)), this,
             SLOT(scale_img_disp(int)) );
    scale_spin->installEventFilter(this);
    scale_spin->setMouseTracking(true);
    QFormLayout* scale_layout = new QFormLayout;
    scale_layout->addRow( tr("Display scale :"), scale_spin );

    ///////////////////////////////////////

    QPushButton* open_button = new QPushButton( tr("Open image") + " " +
                                                QString::number(id_this) );
    connect( open_button, SIGNAL(clicked()), this, SLOT(open_filename()) );

    QVBoxLayout* img_layout = new QVBoxLayout;
    img_layout->addWidget(name_label);
    img_layout->addWidget(area);
    img_layout->addLayout(scale_layout);
    img_layout->addWidget(open_button);
    QGroupBox* img_group = new QGroupBox( tr("Image") + " " +
                                          QString::number(id_this) );
    img_group->setLayout(img_layout);

    //////////////////////////////////////////////////////////////////////////////////

    color_list = new QComboBox;

    QPixmap pm(12,12);

    pm.fill(Qt::red);
    color_list->addItem( pm, tr("Red") );
    pm.fill(Qt::green);
    color_list->addItem( pm, tr("Green") );
    pm.fill(Qt::blue);
    color_list->addItem( pm, tr("Blue") );
    pm.fill(Qt::cyan);
    color_list->addItem( pm, tr("Cyan") );
    pm.fill(Qt::magenta);
    color_list->addItem( pm, tr("Magenta") );
    pm.fill(Qt::yellow);
    color_list->addItem( pm, tr("Yellow") );
    pm.fill(Qt::black);
    color_list->addItem( pm, tr("Black") );
    pm.fill(Qt::white);
    color_list->addItem( pm, tr("White") );

    red_selected = (unsigned char)( settings.value("Evaluation/R"
                                                   +QString::number(id_this), 128).toInt() );
    green_selected = (unsigned char)( settings.value("Evaluation/G"
                                                     +QString::number(id_this), 0).toInt() );
    blue_selected = (unsigned char)( settings.value("Evaluation/B"
                                                    +QString::number(id_this), 255).toInt() );
    pm.fill(QColor(red_selected, green_selected, blue_selected));
    color_list->addItem( pm, tr("Selected") );

    color_list->setCurrentIndex( settings.value("Evaluation/combo"
                                                +QString::number(id_this), 0).toInt() );
    connect( color_list, SIGNAL(currentIndexChanged(int)), this,
             SLOT(create_list(int)) );

    ///////////////////////////////////////

    QPushButton* color_select = new QPushButton( tr("Select") );
    connect( color_select, SIGNAL(clicked()), this, SLOT(get_list_color()) );

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("List from :"), color_list);
    form->addRow(tr("<click on image> |"), color_select);

    QGroupBox* color_group = new QGroupBox( tr("Color") + " "
                                            + QString::number(id_this) );
    color_group->setLayout(form);

    QVBoxLayout* this_layout = new QVBoxLayout;
    this_layout->addWidget(text_list_length);
    this_layout->addWidget(img_group);
    this_layout->addWidget(color_group);

    setLayout(this_layout);

    last_directory_used = settings.value("Main/Name/last_directory_used",
                                         QDir().homePath()).toString();

    name_filters << "*.bmp"
                    //<< "*.dcm"
                 << "*.gif"
                 << "*.jpg" << "*.jpeg" << "*.mng"
                 << "*.pbm" << "*.png" << "*.pgm"
                 << "*.ppm" << "*.svg" << "*.svgz"
                 << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    name_filters.removeDuplicates();

    connect( this, SIGNAL(change_list()), parent, SLOT(check_lists()) );
}

void EvaluationWidget::open_filename()
{
    absolute_name = QFileDialog::getOpenFileName(this,
                                                 tr("Open File") + " " + QString::number(id_this),
                                                 last_directory_used,
                                                 tr("Image Files (%1)").arg(name_filters.join(" ")));
    open_img();
}

void EvaluationWidget::open_img()
{
    if( !absolute_name.isEmpty() )
    {
        img = QImage(absolute_name);
        if( img.isNull() )
        {
            QMessageBox::information(this, tr("Opening error - Ofeli"),
                                     tr("Cannot load %1.").arg(absolute_name));
            return;
        }

        img_disp->set_qimage0(img);
        QApplication::processEvents();

        img_height = img.height();
        img_width = img.width();

        if( list != nullptr )
        {
            delete[] list;
            list = nullptr;
        }
        if( list == nullptr )
        {
            list = new int[img_width*img_height];
        }

        create_list( color_list->currentIndex() );

        X_position = img_width/2;
        Y_position = img_height/2;

        QFileInfo fi(absolute_name);
        QString name = fi.fileName();

        QString string_lists_text;
        string_lists_text=QString::number(img_width)+"×"
                +QString::number(img_height);
        name_label->setText(name +" - "+string_lists_text);
    }
}

void EvaluationWidget::create_list(int color_list_index)
{
    if( list != nullptr )
    {
        unsigned char red, green, blue;

        if( color_list_index == 0 )
        {
            red = 255;
            green = 0;
            blue = 0;
        }
        if( color_list_index == 1 )
        {
            red = 0;
            green = 255;
            blue = 0;
        }
        if( color_list_index == 2 )
        {
            red = 0;
            green = 0;
            blue = 255;
        }
        if( color_list_index == 3 )
        {
            red = 0;
            green = 255;
            blue = 255;
        }
        if( color_list_index == 4 )
        {
            red = 255;
            green = 0;
            blue = 255;
        }
        if( color_list_index == 5 )
        {
            red = 255;
            green = 255;
            blue = 0;
        }
        if( color_list_index == 6 )
        {
            red = 0;
            green = 0;
            blue = 0;
        }
        if( color_list_index == 7 )
        {
            red = 255;
            green = 255;
            blue = 255;
        }
        if( color_list_index == 8 )
        {
            red = red_selected;
            green = green_selected;
            blue = blue_selected;
        }

        QRgb pix;
        int index = 0;

        for( int y = 0; y < img_height; y++ )
        {
            for( int x = 0; x < img_width; x++ )
            {
                pix = img.pixel(x,y);

                if( (unsigned char)(qRed(pix)) == red
                        && (unsigned char)(qGreen(pix)) == green
                        && (unsigned char)(qBlue(pix) == blue) )
                {
                    list[index++] = x+y*img_width; // offset
                }
            }
        }
        list_length = index;

        if( list_length != 0 )
        {
            if( id_this == 1 )
            {
                text_list_length->setText("<font color=green>"
                                          +tr("List 1 length = ")
                                          +QString::number(list_length));
            }
            if( id_this == 2 )
            {
                text_list_length->setText("<font color=green>"
                                          +tr("List 2 length = ")
                                          +QString::number(list_length));
            }
        }
        else
        {
            if( id_this == 1 )
            {
                text_list_length->setText("<font color=red>"
                                          +tr("List 1 length = ")
                                          +QString::number(list_length));
            }
            if( id_this == 2 )
            {
                text_list_length->setText("<font color=red>"
                                          +tr("List 2 length = ")
                                          +QString::number(list_length));
            }
        }

        emit change_list();
    }
}

bool EvaluationWidget::eventFilter(QObject* object, QEvent* event)
{
    if( object == scale_spin && event->type() == QEvent::MouseButtonPress )
    {
        if( !img.isNull() )
        {
            img_disp->set_hasText(false);
            img_disp->setBackgroundRole(QPalette::Dark);
        }
        X_position = img_width/2;
        Y_position = img_height/2;
    }

    if( object == img_disp && event->type() == QEvent::MouseMove )
    {
        QMouseEvent* move = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(move);
    }

    if( object == img_disp && event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* press = static_cast<QMouseEvent*>(event);
        mousePressEvent(press);
    }

    if( object == img_disp && event->type() == QEvent::DragEnter )
    {
        QDragEnterEvent* drag = static_cast<QDragEnterEvent*>(event);
        dragEnterEvent(drag);
    }

    if( object == img_disp && event->type() == QEvent::DragMove )
    {
        QDragMoveEvent* drag = static_cast<QDragMoveEvent*>(event);
        dragMoveEvent(drag);
    }

    if( object == img_disp && event->type() == QEvent::Drop )
    {
        QDropEvent* drop = static_cast<QDropEvent*>(event);
        dropEvent(drop);
    }

    if( object == img_disp && event->type() == QEvent::DragLeave )
    {
        QDragLeaveEvent* drag = static_cast<QDragLeaveEvent*>(event);
        dragLeaveEvent(drag);
    }

    return false;
}

void EvaluationWidget::mouseMoveEvent(QMouseEvent* event)
{
    if( !img.isNull() )
    {
        X_position = int(double(img_width)
                         *double(   (  (event->pos()).x() -img_disp->get_xoffset()   )
                                    /double(img_disp->getPixWidth())));
        Y_position = int(double(img_height)
                         *double(   (  (event->pos()).y() -img_disp->get_yoffset()   )
                                    /double(img_disp->getPixHeight())));
    }
}

void EvaluationWidget::mousePressEvent(QMouseEvent*)
{
    if( !img.isNull() )
    {
        QRgb pix = img.pixel(X_position,Y_position);

        red_selected = (unsigned char)(qRed(pix));
        green_selected = (unsigned char)(qGreen(pix));
        blue_selected = (unsigned char)(qBlue(pix));

        QPixmap pm(12,12);
        pm.fill(QColor(red_selected, green_selected, blue_selected));
        color_list->setItemIcon(8,pm);

        color_list->setCurrentIndex(8);

        create_list(8);
    }
}

void EvaluationWidget::dragEnterEvent(QDragEnterEvent* event)
{
    QString text( tr("<drop image")+QString::number(id_this)+">" );
    img_disp->set_text(text);
    img_disp->setBackgroundRole(QPalette::Highlight);
    img_disp->set_hasText(true);

    event->acceptProposedAction();
    emit changed(event->mimeData());
}

void EvaluationWidget::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void EvaluationWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if( mimeData->hasUrls() )
    {
        QList<QUrl> urlList = mimeData->urls();
        absolute_name = urlList.first().toLocalFile();
    }
    img_disp->setBackgroundRole(QPalette::Dark);
    open_img();
    event->acceptProposedAction();
}

void EvaluationWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    QString text( tr("<drag image")+QString::number(id_this)+">" );
    img_disp->set_text(text);
    img_disp->setBackgroundRole(QPalette::Dark);
    img_disp->set_hasText(true);
    emit changed();
    event->accept();
}

void EvaluationWidget::get_list_color()
{
    QColor color;
    if( id_this == 1 )
    {
        color = QColorDialog::getColor(Qt::white, this,
                                       tr("Select list 1 color"));
    }
    if( id_this == 2 )
    {
        color = QColorDialog::getColor(Qt::white, this,
                                       tr("Select list 2 color"));
    }

    if( color.isValid() )
    {
        red_selected = (unsigned char)(color.red());
        green_selected = (unsigned char)(color.green());
        blue_selected = (unsigned char)(color.blue());

        QPixmap pm(12,12);
        pm.fill(color);
        color_list->setItemIcon(8,pm);

        color_list->setCurrentIndex(8);

        create_list(8);
    }
}

void EvaluationWidget::wheel_zoom(int val, ofeli::ScrollAreaWidget* obj)
{
    if( obj == area && !img.isNull() )
    {
        img_disp->set_hasText(false);
        img_disp->setBackgroundRole(QPalette::Dark);

        float value = 0.002f*float( val ) + img_disp->get_zoomFactor();

        if( value < 32.0f/float( img_disp->get_qimage().width() ) )
        {
            value = 32.0f/float( img_disp->get_qimage().width() );
        }

        scale_spin->setValue( int(100.0f*value) );
    }
}

void EvaluationWidget::scale_img_disp(int value)
{
    if( !img.isNull() )
    {
        double scale_factor = value/100.0;
        img_disp->setZoomFactor(scale_factor);
    }
}

void EvaluationWidget::adjust_vertical_scroll(int min, int max)
{
    if( img_height != 0 )
    {
        area->verticalScrollBar()->setValue( (max-min)*Y_position/img_height );
    }
}

void EvaluationWidget::adjust_horizontal_scroll(int min, int max)
{
    if( img_width != 0 )
    {
        area->horizontalScrollBar()->setValue( (max-min)*X_position/img_width );
    }
}

void EvaluationWidget::save_settings() const
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue("Main/Name/last_directory_used", last_directory_used);

    settings.setValue( "Evaluation/spin"+QString::number(id_this),
                       scale_spin->value() );

    settings.setValue( "Evaluation/combo"+QString::number(id_this),
                       color_list->currentIndex() );

    settings.setValue( "Evaluation/R"+QString::number(id_this),
                       int(red_selected) );
    settings.setValue( "Evaluation/G"+QString::number(id_this),
                       int(green_selected) );
    settings.setValue( "Evaluation/B"+ QString::number(id_this),
                       int(blue_selected) );
}

}
