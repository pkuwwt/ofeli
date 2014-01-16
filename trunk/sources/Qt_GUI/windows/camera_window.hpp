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

#ifndef CAMERA_WINDOW_HPP
#define CAMERA_WINDOW_HPP

#include <QDialog>

class QCamera;
class QSpinBox;
class QSlider;

namespace ofeli
{

class PixmapWidget;
class ScrollAreaWidget;
class SettingsWindow;
class CameraInterface;

class CameraWindow : public QDialog
{

    Q_OBJECT

public :

    CameraWindow(QWidget* parent = nullptr, ofeli::SettingsWindow* settings_window1 = nullptr);

    void set_img_width(unsigned int img_width1);
    void set_img_height(unsigned int img_height1);

    //! This function is called by the the #main_window close event in order to save persistent settings (window size, position, etc... ) of #camera_window.
    void save_settings() const;
    void restart();

public slots :

    void show_camera();

private :

    QCamera* camera;
    ofeli::CameraInterface* camera_interface;
    virtual void closeEvent(QCloseEvent* event) override;
    ofeli::PixmapWidget* imageLabel;
    ofeli::ScrollAreaWidget* scrollArea;
    QSpinBox* scale_spin0;
    QSlider* scale_slider0;
    virtual bool eventFilter(QObject* object, QEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    unsigned int positionX;
    unsigned int positionY;
    unsigned int img_width;
    unsigned int img_height;

    // pour detecter le systeme automatiquement
    #ifdef Q_OS_WIN
    static const int operating_system = 1;
    #endif
    #ifdef Q_OS_MAC
    static const int operating_system = 2;
    #endif
    #ifdef Q_OS_LINUX
    static const int operating_system = 3;
    #endif
    
private slots :

    void do_scale0(int value);
    void wheel_zoom(int,ScrollAreaWidget*);
    void adjustVerticalScroll(int min, int max);
    void adjustHorizontalScroll(int min, int max);
    
};

}

#endif // CAMERA_WINDOW_HPP
