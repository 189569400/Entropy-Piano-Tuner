/*****************************************************************************
 * Copyright 2018 Haye Hinrichsen, Christoph Wick
 *
 * This file is part of Entropy Piano Tuner.
 *
 * Entropy Piano Tuner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Entropy Piano Tuner is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Entropy Piano Tuner. If not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

#include "logviewer.h"
#include <QTextStream>
#include <QFile>
#include <QScroller>
#include <QClipboard>
#include <QTextCursor>

#include "core/config.h"

#include "implementations/filemanagerforqt.h"

#include "ui_logviewer.h"
#include "qtconfig.h"

LogViewer::LogViewer(int defaultIndex, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::LogViewer)
{
    ui->setupUi(this);
    setModal(true);

    if (parent) {
        QRect p(parent->geometry());
        setGeometry(p.left() + p.width() / 4, p.top() + p.height() / 4, p.width() / 2, p.height() / 2);
    }

    ui->logSelectionComboBox->setCurrentIndex(-1);
    ui->logSelectionComboBox->setCurrentIndex(defaultIndex);


    ui->textBrowser->setReadOnly(true);
    ui->textBrowser->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard | Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    // icon
    ui->copyToClipboardButton->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/media/icons/edit-copy.png")));

    SHOW_DIALOG(this);

    QScroller::grabGesture(ui->textBrowser);
}

LogViewer::~LogViewer()
{
    delete ui;
}

void LogViewer::copyToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->textBrowser->toPlainText());
}

void LogViewer::logIndexChanged(int i) {
    if (i < 0) {
        ui->textBrowser->setText("Internal log error. This should not have appened!dropb    "
                                 "");
        return;
    }
    QString path;
    switch (i) {
    case CURRENT_LOG:
        path = tp3Log::logPath();
        break;
    case PREVIOUS_LOG:
        path = tp3Log::oldLogPath();
        break;
    default:
        path = tp3Log::logPath();
        LogW("Invalid log index %d. Using default", i);
        break;
    }
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))  {
        ui->textBrowser->setText(tr("Log file does not exist."));
        return;
    }
    QTextStream in(&f);
    ui->textBrowser->setText(in.readAll());

    QTextCursor tc = ui->textBrowser->textCursor();
    tc.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(tc);
    ui->textBrowser->ensureCursorVisible();

}
