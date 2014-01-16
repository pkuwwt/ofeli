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

#include "about_window.hpp"
#include <QtWidgets>

namespace ofeli
{

AboutWindow::AboutWindow(QWidget* parent) :
    QDialog(parent)
{
    QSettings settings("Bessy", "Ofeli");

    setWindowTitle( tr("About Ofeli") );
    resize( settings.value("About/Window/size", QSize(650, 400)).toSize() );
    move( settings.value("About/Window/position",
                         QPoint(200, 200)).toPoint() );



    ///////////////////////////////////////
    //////         Left Part        ///////
    ///////////////////////////////////////

    QLabel* icon_label = new QLabel;
    QImage icon(":Ofeli.png");
    QPixmap icon_pixmap = QPixmap::fromImage(icon);
    icon_label->setPixmap(icon_pixmap);
    icon_label->setAlignment(Qt::AlignCenter);

    QLabel* name_label = new QLabel;
    name_label->setText("<b>Ofeli</b>");
    QFont font1;
    font1.setPointSize(24);
    font1.setBold(true);
    name_label->setFont(font1);
    name_label->setAlignment(Qt::AlignCenter);
    name_label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                        | Qt::LinksAccessibleByMouse
                                        | Qt::LinksAccessibleByKeyboard);

    QLabel* version_label = new QLabel;
    QFont font2;
    font2.setPointSize(12);
    version_label->setFont(font2);
    version_label->setText("Version 1.0.8");
    version_label->setAlignment(Qt::AlignCenter);
    version_label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                           | Qt::LinksAccessibleByMouse
                                           | Qt::LinksAccessibleByKeyboard);

    QLabel* years_label = new QLabel;
    QFont font3;
    font3.setPointSize(12);
    years_label->setFont(font3);
    years_label->setText("2010-2013");
    years_label->setAlignment(Qt::AlignCenter);
    years_label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                         | Qt::LinksAccessibleByMouse
                                         | Qt::LinksAccessibleByKeyboard);

    QPushButton* webpage = new QPushButton( tr("Web page") );
    webpage->setAutoDefault(false);
    connect( webpage, SIGNAL(clicked()), this, SLOT(open_webpage()) );

    QPushButton* license = new QPushButton( tr("License") );
    license->setAutoDefault(false);


    QString locale = QLocale::system().name().section('_', 0, 0);
    int language = settings.value("Language/current_index",0).toInt();

    QString txt_file;
    if( language == 2 || ( language == 0 && locale == "fr" ) )
    {
        txt_file = QString(":licence.txt");
    }
    else
    {
        txt_file = QString(":license.txt");
    }

    QFile file(txt_file);
    QTextEdit* license_textedit = new QTextEdit;
    if( file.open(QIODevice::ReadOnly) )
    {
        QTextStream ts(&file);
        ts.setCodec("ISO 8859-1");
        license_textedit->setText( ts.readAll() );
    }

    license_textedit->setReadOnly(true);



    QHBoxLayout* layout_license = new QHBoxLayout;
    layout_license->addWidget(license_textedit);

    license_window = new QDialog(this);
    license_window->setWindowTitle( tr("License") );
    license_window->resize(settings.value( "About/License/size",
                                           QSize(562,351)).toSize() );

    license_window->move(settings.value( "About/License/position",
                                         QPoint(200, 200)).toPoint() );

    license_window->setLayout(layout_license);
    connect( license, SIGNAL(clicked()), license_window, SLOT(show()) );

    QVBoxLayout* left_layout = new QVBoxLayout;
    left_layout->addWidget(icon_label);
    left_layout->addWidget(name_label);
    left_layout->addWidget(version_label);
    left_layout->addWidget(years_label);
    left_layout->addSpacing(10);
    left_layout->addWidget(webpage);
    left_layout->addWidget(license);
    left_layout->addStretch(1);


    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////



    ///////////////////////////////////////
    //////         Right Part        //////
    ///////////////////////////////////////

    QLabel* description_label = new QLabel;
    description_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    description_label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                               | Qt::LinksAccessibleByMouse
                                               | Qt::LinksAccessibleByKeyboard);

    description_label->setOpenExternalLinks(true);
    description_label->setWordWrap(true);

    description_label->setText(tr("<p>Ofeli, as an acronym for <b>O</b>pen, <b>F</b>ast and <b>E</b>fficient <b>L</b>evel set "
                                 "<b>I</b>mplementation, demonstrates how to operate an image "
                                 "segmentation algorithm of Y. Shi and W. C. Karl <b>[1]</b>, using a "
                                 "discrete approach for the approximation of level-set-based "
                                 "curve evolution (implicit active contours).</p>"
                                 "<p>This is a novel (2005) and fast algorithm without the need "
                                 "of solving partial differential equations (PDE) while preserving the "
                                 "advantages of level set methods, such as the automatic handling of "
                                 "topological changes. Considerable speedups (×100) have been "
                                 "demonstrated as compared to PDE-based narrow band level-set implementations.</p>"
                                 "<hr>"
                                 "<p><b>[1]</b> Y. Shi, W. C. Karl - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_MGIwYmUwYzctYTRkMC00ODMwLWI3YmUtNTFjYThlMTBkOTIy&hl=en&authkey=CPT1xeYN'>A real-time algorithm for the approximation of level-set based curve evolution</a> - <i>IEEE Trans. Image Processing</i>, vol. 17, no. 5, May 2008</p>"));

    QVBoxLayout* description_layout = new QVBoxLayout;
    description_layout->addWidget(description_label);


    QLabel* author_label = new QLabel;
    author_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    author_label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                          | Qt::LinksAccessibleByMouse
                                          | Qt::LinksAccessibleByKeyboard);

    author_label->setOpenExternalLinks(true);
    author_label->setWordWrap(true);

    author_label->setText(tr("<p>This application has been developed by Fabien Bessy, under the supervision of Julien Olivier "
                                 "and Romuald Boné, during an internship in the <i>pattern recognition and image analysis research</i> "
                                 "<i>team (RFAI-LI)</i> of the <i>François Rabelais University's computer science laboratory</i>, at Tours, "
                                 "as part of the MSc in medical imaging of the same university, in 2010.</p>"
                                 "<p>If you have any questions, comments, or suggestions, please contact me via my email "
                                 "address : <a href='mailto:fabien.bessy@gmail.com'>fabien.bessy@gmail.com</a>.</p>"));

    QVBoxLayout* author_layout = new QVBoxLayout;
    author_layout->addWidget(author_label);


    QLabel* acknowledgments_label = new QLabel;
    acknowledgments_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    acknowledgments_label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                                   | Qt::LinksAccessibleByMouse
                                                   | Qt::LinksAccessibleByKeyboard);
    acknowledgments_label->setOpenExternalLinks(true);
    acknowledgments_label->setWordWrap(true);

    acknowledgments_label->setText(tr("<p>Thanks to :</p>"
                                 "<p>- J. Olivier, R. Boné, J-M. Girault, F. Amed, A. Lissy, "
                                 "C. Rouzière, L. Suta.</p>"
                                 "<p>- <i>pattern recognition and image analysis research team</i>, <i>computer science laboratory</i>, "
                                 "<i>François Rabelais University</i>.</p>"
                                 "<p>- students and professors of the MSc in medical imaging.</p>"));

    QVBoxLayout* acknowledgments_layout = new QVBoxLayout;
    acknowledgments_layout->addWidget(acknowledgments_label);


    QWidget* page1 = new QWidget;
    QWidget* page2 = new QWidget;
    QWidget* page3 = new QWidget;
    page1->setLayout(description_layout);
    page2->setLayout(author_layout);
    page3->setLayout(acknowledgments_layout);

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab( page1, tr("Description") );
    tabs->addTab( page2, tr("Author") );
    tabs->addTab( page3, tr("Acknowledgments") );

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    QHBoxLayout* layout_this = new QHBoxLayout;
    layout_this->addLayout(left_layout);
    layout_this->addWidget(tabs);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    this->setLayout(layout_this);
}

void AboutWindow::open_webpage()
{
    QDesktopServices::openUrl( QUrl("http://www.code.google.com/p/ofeli/",
                                    QUrl::TolerantMode) );
}

void AboutWindow::save_settings() const
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue( "About/Window/size", size() );
    settings.setValue( "About/Window/position", pos() );
    settings.setValue( "About/License/size", license_window->size() );
    settings.setValue( "About/License/position", license_window->pos() );
}

}
