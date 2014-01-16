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

#include "language_window.hpp"
#include <QtWidgets>

namespace ofeli
{

LanguageWindow::LanguageWindow(QWidget* parent) :
    QDialog(parent)
{
    setWindowTitle( tr("Language") );

    QSettings settings("Bessy", "Ofeli");

    resize( settings.value("Language/Window/size",
                           QSize(250, 250)).toSize() );

    move( settings.value("Language/Window/position",
                         QPoint(200, 200)).toPoint() );

    list_widget = new QListWidget(this);

    QString locale = QLocale::system().name().section('_', 0, 0);

    list_widget->addItem( tr("System (")+locale+")" );
    list_widget->addItem( tr("English") );
    list_widget->addItem( tr("French") );
    selected_index = settings.value("Language/current_index",0).toInt();
    list_widget->setCurrentRow(selected_index);

    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);
    connect( buttons, SIGNAL(accepted()), this, SLOT(accept()) );
    connect( buttons, SIGNAL(rejected()), this, SLOT(reject()) );

    QLabel* restart_label = new QLabel(this);
    restart_label->setAlignment(Qt::AlignJustify);
    restart_label->setWordWrap(true);
    restart_label->setText(tr("The change will take effect after restarting the application."));

    QVBoxLayout* layout_this = new QVBoxLayout;
    layout_this->addWidget(list_widget);
    layout_this->addWidget(restart_label);
    layout_this->addWidget(buttons);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(layout_this);
}

void LanguageWindow::apply_setting()
{
    selected_index = list_widget->currentRow();
}

void LanguageWindow::cancel_setting()
{
    list_widget->setCurrentRow(selected_index);
}

void LanguageWindow::save_settings() const
{
    QSettings settings("Bessy", "Ofeli");

    settings.setValue( "Language/Window/size", size() );
    settings.setValue( "Language/Window/position", pos() );

    settings.setValue("Language/current_index", selected_index);
}


}
