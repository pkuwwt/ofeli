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

#include "evaluation_window.hpp"
#include "evaluation_widget.hpp"
#include "hausdorff_distance.hpp"
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"
#include <QtWidgets>
#include <ctime>       // for std::clock_t, std::clock() and CLOCKS_PER_SEC

namespace ofeli
{

EvaluationWindow::EvaluationWindow(QWidget* parent) :
    QDialog(parent)
{
    QSettings settings("Bessy", "Ofeli");

    setWindowTitle(tr("Evaluation"));

    move(settings.value( "Evaluation/Window/position",
                         QPoint(200, 200)).toPoint() );
    resize(settings.value( "Evaluation/Window/size",
                           QSize(665,542)).toSize() );

    ///////////////////////////////////////
    /////           1st Tab          //////
    ///////////////////////////////////////

    widget1 = new ofeli::EvaluationWidget(this);
    widget2 = new ofeli::EvaluationWidget(this);
    QHBoxLayout* lists_select_layout = new QHBoxLayout;
    lists_select_layout->addWidget(widget1);
    lists_select_layout->addWidget(widget2);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////


    ///////////////////////////////////////
    /////           2nd Tab          //////
    ///////////////////////////////////////

    hausdorff_label = new QLabel(this);
    hausdorff_label->setText( tr("Hausdorff distance = ") );
    hausdorff_label->setAlignment(Qt::AlignCenter);
    hausdorff_ratio_label = new QLabel(this);
    hausdorff_ratio_label->setText( tr("modified Hausdorff distance = ") );
    hausdorff_ratio_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* hausdorff_layout = new QVBoxLayout;
    hausdorff_layout->addWidget(hausdorff_label);
    hausdorff_layout->addWidget(hausdorff_ratio_label);
    QGroupBox* hausdorff_group = new QGroupBox( tr("Hausdorff measure") );
    hausdorff_group->setLayout(hausdorff_layout);

    mh_label = new QLabel(this);
    mh_label->setText( tr("Hausdorff ratio = ") );
    mh_label->setAlignment(Qt::AlignCenter);
    mh_ratio_label = new QLabel(this);
    mh_ratio_label->setText( tr("modified Hausdorff ratio = ") );
    mh_ratio_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* mh_layout = new QVBoxLayout;
    mh_layout->addWidget(mh_label);
    mh_layout->addWidget(mh_ratio_label);
    QGroupBox* mh_group = new QGroupBox( tr("Modified Hausdoff measure") );
    mh_group->setLayout(mh_layout);

    centroids_dist_label = new QLabel(this);
    centroids_dist_label->setText( tr("distance between centroids = ") );
    centroids_dist_label->setAlignment(Qt::AlignCenter);
    centroids_ratio_label = new QLabel(this);
    centroids_ratio_label->setText( tr("ratio between centroids = ") );
    centroids_ratio_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* centroids_layout = new QVBoxLayout;
    centroids_layout->addWidget(centroids_dist_label);
    centroids_layout->addWidget(centroids_ratio_label);
    QGroupBox* centroids_group = new QGroupBox( tr("Shapes gap") );
    centroids_group->setLayout(centroids_layout);

    time_label = new QLabel(this);
    time_label->setText( tr("Calculating time = ") );
    time_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* time_layout = new QVBoxLayout;
    time_layout->addWidget(time_label);
    QGroupBox* time_group = new QGroupBox( tr("Calculating time") );
    time_group->setLayout(time_layout);


    QVBoxLayout* distances_layout = new QVBoxLayout;
    distances_layout->addWidget(hausdorff_group);
    distances_layout->addWidget(mh_group);
    distances_layout->addWidget(centroids_group);
    distances_layout->addWidget(time_group);
    distances_layout->addStretch(1);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    QWidget* page1 = new QWidget;
    QWidget* page2 = new QWidget;
    page1->setLayout(lists_select_layout);
    page2->setLayout(distances_layout);

    tabs = new QTabWidget(this);
    tabs->addTab( page1, tr("Select lists") );
    tabs->addTab( page2, tr("Compute distances") );
    connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(compute_hd(int)) );
    check_lists();

    QHBoxLayout* this_layout = new QHBoxLayout;
    this_layout->addWidget(tabs);
    setLayout(this_layout);
}

void EvaluationWindow::compute_hd(int index)
{
    if( index == 1 )
    {
        std::clock_t start_time, stop_time;
        start_time = std::clock();
        ofeli::HausdorffDistance results( widget1->get_list(),
                                          widget1->get_list_length(),
                                          widget1->get_img_width(),
                                          widget1->get_img_height(),
                                          widget2->get_list(),
                                          widget2->get_list_length(),
                                          widget2->get_img_width(),
                                          widget2->get_img_height() );
        stop_time = std::clock();

        double hd = results.get_hausdorff_dist();
        double hr = results.get_hausdorff_ratio();
        hausdorff_label->setText(tr("Hausdorff distance = ")
                                 +QString::number(hd)+(tr(" pixels")));
        hausdorff_ratio_label->setText(tr("Hausdorff ratio = ")
                                       +QString::number(100.0*hr)+(" %"));

        double mhd = results.get_modified_hausdorff_dist();
        double mhr = results.get_modified_hausdorff_ratio();
        mh_label->setText(tr("modified Hausdorff distance = ")
                          +QString::number(mhd)+(tr(" pixels")));
        mh_ratio_label->setText(tr("modified Hausdorff ratio = ")
                                +QString::number(100.0*mhr)+(" %"));

        double cd = results.get_centroids_dist();
        double cr = results.get_centroids_ratio();
        centroids_dist_label->setText(tr("distance between centroids = ")
                                      +QString::number(cd)+(tr(" pixels")));
        centroids_ratio_label->setText(tr("ratio between centroids = ")
                                       +QString::number(100.0*cr)+(" %"));

        double time = double(stop_time - start_time) / double(CLOCKS_PER_SEC);
        time_label->setText(tr("time = ")+QString::number(time)+(" s"));
    }
}

void EvaluationWindow::check_lists()
{
    if( widget1->get_list_length() > 0 && widget2->get_list_length() > 0 )
    {
        tabs->setTabEnabled(1,true);
    }
    else
    {
        tabs->setTabEnabled(1,false);
    }
}

void EvaluationWindow::save_settings() const
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue("Evaluation/Window/size", size());
    settings.setValue("Evaluation/Window/position", pos());

    widget1->save_settings();
    widget2->save_settings();
}

}
