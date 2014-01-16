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

#ifndef EVALUATION_WINDOW_HPP
#define EVALUATION_WINDOW_HPP

#include <QtWidgets>

namespace ofeli
{

class EvaluationWidget;
class PixmapWidget;
class ScrollAreaWidget;

class EvaluationWindow : public QDialog
{
    Q_OBJECT

public :

    //! A parametric constructor with a pointer on the QWidget parent.
    EvaluationWindow(QWidget* parent = nullptr);

    //! This function is called by the the #main_window close event in order to save persistent settings (window size, position, etc...) of #evaluation_window.
    void save_settings() const;

public slots :

    void check_lists();

private :

    QTabWidget* tabs;

    ofeli::EvaluationWidget* widget1;
    ofeli::EvaluationWidget* widget2;

    QLabel* hausdorff_label;
    QLabel* mh_label;
    QLabel* hausdorff_ratio_label;
    QLabel* mh_ratio_label;
    QLabel* centroids_dist_label;
    QLabel* centroids_ratio_label;
    QLabel* time_label;

private slots :

    void compute_hd(int index);
};

}

#endif // EVALUATION_WINDOW_HPP

//! \class ofeli::EvaluationWindow
//! The class EvaluationWindow is a QDialog window that informs the user about Ofeli . An instance of this class is created by #ofeli::MainWindow and displayed when the user clicks on menu About.
