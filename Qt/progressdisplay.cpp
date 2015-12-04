#include "progressdisplay.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QFontMetrics>

ProgressDisplay::ProgressDisplay(QWidget *mainWindow) :
    QFrame(mainWindow)
{
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
            setFixedHeight(br.height() * 1.5);
        }

    };

    layout->addWidget(mWaveformGeneratorStatusLabel = new FixedSizeLabel(tr("Synthesizer")));
    layout->addWidget(mWaveformGeneratorStatusBar = new FixedSizeProgressBar);

    mainWindow->installEventFilter(this);

    //resize(200, 100);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setAutoFillBackground(true);
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
        mWaveformGeneratorStatusBar->setVisible(false);
        mWaveformGeneratorStatusLabel->setVisible(false);
    } else {
        mWaveformGeneratorStatusBar->setVisible(true);
        mWaveformGeneratorStatusLabel->setVisible(true);
    }

    adjustSize();
}

void ProgressDisplay::updatePosition()
{
    QSize pos(parentWidget()->size() - size());
    move(pos.width(), pos.height());
}
