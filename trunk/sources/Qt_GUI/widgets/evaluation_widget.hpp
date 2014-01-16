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

#ifndef EVALUATION_WIDGET_HPP
#define EVALUATION_WIDGET_HPP

#include <QtWidgets>

namespace ofeli
{

class PixmapWidget;
class ScrollAreaWidget;

class EvaluationWidget : public QWidget
{
    Q_OBJECT

public :

    EvaluationWidget(QWidget* parent = nullptr);

    int get_img_width() const { return img_width; }
    int get_img_height() const { return img_height; }
    const int* get_list() const { return list; }
    int get_list_length() const { return list_length; }

    void save_settings() const;
    
private :

    virtual bool eventFilter(QObject* object, QEvent* event) override;

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;

    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

    QLabel* text_list_length;
    QString absolute_name;
    QLabel* name_label;
    ofeli::PixmapWidget* img_disp;
    ofeli::ScrollAreaWidget* area;
    QSpinBox* scale_spin;
    QPushButton* open_button;

    QComboBox* color_list;
    unsigned char red_selected;
    unsigned char green_selected;
    unsigned char blue_selected;

    QImage img;
    int img_width;
    int img_height;
    int X_position;
    int Y_position;
    int* list;
    int list_length;

    QString last_directory_used;
    QStringList name_filters;

    static int count_this;
    int id_this;

private slots :

    void open_filename();
    void open_img();
    void create_list(int);
    void get_list_color();

    void scale_img_disp(int value);

    void adjust_vertical_scroll(int min, int max);
    void adjust_horizontal_scroll(int min, int max);

    void wheel_zoom(int val, ScrollAreaWidget* obj);

signals :

    void changed(const QMimeData* mimeData = 0);
    void change_list();

};

}

#endif // EVALUATION_WIDGET_HPP
