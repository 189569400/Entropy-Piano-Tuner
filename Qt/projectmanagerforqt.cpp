/*****************************************************************************
 * Copyright 2015 Haye Hinrichsen, Christoph Wick
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

#include "projectmanagerforqt.h"
#include "editpianosheetdialog.h"
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDate>
#ifdef __ANDROID__
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

#include "qtconfig.h"
#include "../core/system/log.h"
#include "../core/config.h"
#include "mainwindow.h"
#include "simplefiledialog.h"

ProjectManagerForQt::ProjectManagerForQt(MainWindow *mainwindow)
    : ProjectManagerAdapter(),
      mMainWindow(mainwindow) {

}

ProjectManagerForQt::~ProjectManagerForQt()
{

}


ProjectManagerForQt::Results ProjectManagerForQt::editFile() {
    EditPianoSheetDialog d(mCore->getPianoManager()->getPiano(), mMainWindow);

    if (d.exec() == QDialog::Accepted) {
        d.applyData(&mCore->getPianoManager()->getPiano());
        return R_ACCEPTED;
    } else {
        return R_CANCELED;
    }
}

ProjectManagerForQt::Results ProjectManagerForQt::askForSaving() {
    QMessageBox msgBox(mMainWindow);
    msgBox.setWindowTitle(mMainWindow->windowTitle());
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(MainWindow::tr("The document has been modified."));
    msgBox.setInformativeText(MainWindow::tr("Do you want to save your changes?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    switch (msgBox.exec()) {
    case QMessageBox::Yes:
        return R_YES;
    case QMessageBox::No:
        return R_NO;
    default:
        return R_CANCELED;
    }
}

std::string ProjectManagerForQt::getSavePath() {
#if CONFIG_USE_SIMPLE_FILE_DIALOG
    return SimpleFileDialog::getSaveFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toStdString();
#else
    QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QFileDialog d(mMainWindow, MainWindow::tr("Save"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), MainWindow::tr("Entopy piano tuner (*.ept);; All files (*.*)"));
    d.setAcceptMode(QFileDialog::AcceptSave);
    d.setFileMode(QFileDialog::AnyFile);
    d.setDefaultSuffix("ept");
    SHOW_DIALOG(&d);
    if (d.exec() == QDialog::Accepted) {
        return d.selectedFiles().first().toStdString();
    } else {
        return std::string();
    }
#endif
}

std::string ProjectManagerForQt::getOpenPath()  {
#if CONFIG_USE_SIMPLE_FILE_DIALOG
    return SimpleFileDialog::getOpenFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toStdString();
#else
    QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QFileDialog d(mMainWindow, MainWindow::tr("Open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), MainWindow::tr("Entopy piano tuner (*.ept);; All files (*.*)"));
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setFileMode(QFileDialog::AnyFile);
    d.setDefaultSuffix("ept");
    SHOW_DIALOG(&d);
    if (d.exec() == QDialog::Accepted) {
        return d.selectedFiles().first().toStdString();
    } else {
        return std::string();
    }
#endif
}

ProjectManagerForQt::Results ProjectManagerForQt::share() {
#ifdef __ANDROID__
    // on android we have to call a java method, that does the sharing

    // get the application instance
    QAndroidJniObject instance = QAndroidJniObject::callStaticObjectMethod("org/uniwue/tp3/TunerApplication", "getInstance", "()Lorg/uniwue/tp3/TunerApplication;");
    QAndroidJniObject jTitle = QAndroidJniObject::fromString(MainWindow::tr("Share tuning data"));
    QAndroidJniObject jPath = QAndroidJniObject::fromString(QString::fromStdString(getCurrentFilePath()));
    // get the path to the file to open, zero length if there is none
    instance.callMethod<void>("shareFile", "(Ljava/lang/String;Ljava/lang/String;)V", jTitle.object<jstring>(), jPath.object<jstring>());
    // check for errors
    QAndroidJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
#else
    INFORMATION("Sharing for this platform is not supported yet.");
#endif
    return R_CANCELED; // function has to return something, therefore I put this here
}

void ProjectManagerForQt::fillNew(Piano &piano) {
    piano.setName(MainWindow::tr("New piano").toStdString());
    piano.setSerialNumber("0000-0000");
    piano.setManufactureYear(QString("%1").arg(QDate::currentDate().year()).toStdString());
    piano.setManufactureLocation(MainWindow::tr("Unknown").toStdString());

    piano.setTuningLocation(MainWindow::tr("Unknown").toStdString());
    piano.setNow();
    piano.setConcertPitch(Piano::DEFAULT_CONCERT_PITCH);
    piano.getKeyboard().setNumberOfKeys(Piano::DEFAULT_NUMBER_OF_KEYS,
                                        Piano::DEFAULT_KEY_NUMBER_OF_A);
}

void ProjectManagerForQt::showOpenError() {
    QMessageBox::critical(mMainWindow, MainWindow::tr("Error"), MainWindow::tr("File could not be opened."));
}

void ProjectManagerForQt::showSaveError() {
    QMessageBox::critical(mMainWindow, MainWindow::tr("Error"), MainWindow::tr("File could not be saved."));
}
