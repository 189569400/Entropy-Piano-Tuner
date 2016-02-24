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

#include "progressdisplay.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QFontMetrics>

ProgressDisplay::ProgressDisplay(QWidget *mainWindow) :
    QFrame(mainWindow)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    class FixedSizeLabel : public QLabel
    {
    public:
        FixedSizeLabel(QString s) :
            QLabel(s)
        {
            QRect br(fontMetrics().boundingRect(text()));
            setFixedSize(br.size());
        }
    };

    class FixedSizeProgressBar : public QProgressBar
    {
    public:
        FixedSizeProgressBar()
        {
            QRect br(fontMetrics().boundingRect('A'));
            setFixedHeight(br.height());
            setTextVisible(false);
        }

    };

    layout->addWidget(mWaveformGeneratorStatusLabel = new FixedSizeLabel(tr("Synthesizer")));
    layout->addWidget(mWaveformGeneratorStatusBar = new FixedSizeProgressBar);

    mainWindow->installEventFilter(this);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setAutoFillBackground(true);
    setFrameShape(QFrame::StyledPanel);
    show();

    adjustSize();

    QObject::connect(this, SIGNAL(wgPercentageChanged(int)), this, SLOT(updateWGPercentage(int)));

    emit wgPercentageChanged(100);
}


void ProgressDisplay::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    updatePosition();
}

bool ProgressDisplay::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    if (e->type() == QEvent::Resize) {
        updatePosition();
    }
    return false;
}

void ProgressDisplay::queueSizeChanged(size_t size, size_t total_size)
{
    emit wgPercentageChanged(((total_size - size) * 100) / total_size);
}

void ProgressDisplay::updateWGPercentage(int percentage)
{
    mWaveformGeneratorStatusBar->setValue(percentage);

    if (percentage >= 100) {
        mWaveformGeneratorStatusBar->hide();
        mWaveformGeneratorStatusLabel->hide();
    } else {
        mWaveformGeneratorStatusBar->show();
        mWaveformGeneratorStatusLabel->show();
    }

    updateVisibility();
    adjustSize();
}

void ProgressDisplay::updateVisibility()
{
    if (!mWaveformGeneratorStatusBar->isHidden()) {
        show();
    } else {
        hide();
    }
}

void ProgressDisplay::updatePosition()
{
    QSize pos(parentWidget()->size() - size());
    move(pos.width(), pos.height());
}
