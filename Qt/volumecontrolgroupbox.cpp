#include "volumecontrolgroupbox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLine>
#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QShortcut>
#include <QKeySequence>

#include "displaysize.h"

VolumeControlGroupBox::VolumeControlGroupBox(QWidget *parent) :
    DisplaySizeDependingGroupBox(parent, new QVBoxLayout, toFlag(MODE_IDLE) | toFlag(MODE_RECORDING))
{
    setTitle(tr("Volume control"));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(mMainWidgetContainer->layout());
    mainLayout->setSpacing(0);

    mVolumeControlLevel = new VolumeControlLevel(this);
    mVolumeControlLevel->setFormat(QString());
    if (!mGroupBox) {
        mVolumeControlLevel->setFormat(tr("Volume"));
    }
    mainLayout->addWidget(mVolumeControlLevel);

    QHBoxLayout *linesLayout = new QHBoxLayout;
    mLinesLayout = linesLayout;
    mainLayout->addLayout(linesLayout);

    auto createLine = []() {
        QFont f;
        QFrame *frame = new QFrame;
        frame->setFrameShape(QFrame::VLine);
        frame->setFrameShadow(QFrame::Plain);
        frame->setFixedSize(1, f.pointSizeF() * 0.75);
        frame->setLineWidth(3);
        return frame;
    };

    linesLayout->addStretch();
    linesLayout->addWidget(createLine());
    linesLayout->addStretch();
    linesLayout->addWidget(createLine());
    linesLayout->addStretch();


    QHBoxLayout *textLayout = new QHBoxLayout;
    mTextLayout = textLayout;
    mainLayout->addLayout(textLayout);
    textLayout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->addStretch();

    QLabel *offLabel = new QLabel(tr("Off"));
    textLayout->addWidget(offLabel);

    QHBoxLayout *onLayout = new QHBoxLayout;
    onLayout->setMargin(0);
    textLayout->addLayout(onLayout);

    QLabel *onLabel = new QLabel(tr("On"));
    onLayout->addWidget(onLabel);
    onLayout->addStretch();

    // add buttons if we have a big enough screen
    // if (mGroupBox) {
        QToolButton *refreshButton = new QToolButton;
        onLayout->addWidget(refreshButton);
        refreshButton->setIcon(QIcon(":/media/images/refresh.png"));
        refreshButton->setIconSize(QSize(1, 1) * DisplaySizeDefines::getSingleton()->getMediumIconSize());
        QObject::connect(refreshButton, SIGNAL(clicked(bool)), this, SLOT(onRefreshClicked()));

        QToolButton *muteButton = new QToolButton;
        onLayout->addWidget(muteButton);
        muteButton->setCheckable(true);
        QIcon muteIcons;
        muteIcons.addFile(QStringLiteral(":/media/images/micro_active.png"), QSize(), QIcon::Normal, QIcon::Off);
        muteIcons.addFile(QStringLiteral(":/media/images/micro_mute.png"), QSize(), QIcon::Normal, QIcon::On);
        muteButton->setIcon(muteIcons);
        muteButton->setIconSize(QSize(1, 1) * DisplaySizeDefines::getSingleton()->getMediumIconSize());
        QObject::connect(muteButton, SIGNAL(toggled(bool)), this, SLOT(onMuteToggled(bool)));
        new QShortcut(QKeySequence(Qt::Key_M), muteButton, SLOT(toggle()));
    //}

    mainLayout->addStretch();
}

void VolumeControlGroupBox::updateLevels(double stopLevel, double onLevel) {
    int max = 10000;
    int pos0 = stopLevel * max;
    int pos1 = onLevel * max;

    mTextLayout->setStretch(0, pos0);
    mTextLayout->setStretch(1, pos1 - pos0);
    mTextLayout->setStretch(2, max - pos1);

    mLinesLayout->setStretch(0, pos0);
    mLinesLayout->setStretch(1, 0);
    mLinesLayout->setStretch(2, pos1 - pos0);
    mLinesLayout->setStretch(3, 0);
    mLinesLayout->setStretch(4, max - pos1);
}

void VolumeControlGroupBox::onRefreshClicked() {
    emit refreshInputLevels();
}

void VolumeControlGroupBox::onMuteToggled(bool b) {
    emit muteToggled(b);
}
