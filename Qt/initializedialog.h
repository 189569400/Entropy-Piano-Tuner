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

#ifndef INITIALIZEDIALOG_H
#define INITIALIZEDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include "../core/adapters/coreinitialisationadapter.h"

class InitializeDialog;

class QtCoreInitialisation : public CoreInitialisationAdapter {
public:
    explicit QtCoreInitialisation(QWidget *parent = 0);
    ~QtCoreInitialisation();

private:
    virtual void updateProgress (CoreStatusTypes type, int percentage);

    virtual void create_impl() override;
    virtual void destroy_impl() override;


private:
    QWidget *mParent;
    InitializeDialog *mInitializeDialog;
};




class InitializeDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit InitializeDialog(QWidget *parent = 0);
    ~InitializeDialog();

    bool onceDrawn() const {return mOnceDrawn;}
    void updateProgress(QtCoreInitialisation::CoreStatusTypes type, int percentage);

protected:
    virtual void paintEvent(QPaintEvent *p) override;
private:
    bool mOnceDrawn;
};

#endif // INITIALIZEDIALOG_H
