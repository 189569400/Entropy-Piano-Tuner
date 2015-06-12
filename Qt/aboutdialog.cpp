#include "aboutdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QScroller>
#include <QPushButton>
#include <QDateTime>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFile>
#include <QProcess>

#include "core/system/eptexception.h"
#include "core/system/version.h"
#include "core/system/log.h"
#include "qtconfig.h"


AboutDialog::AboutDialog(QWidget *parent, QString iconPostfix) :
    QDialog(parent, Qt::Window)
{
    EptAssert(parent, "Parent required for size");

    QRect r(parent->geometry());
    setGeometry(r.left() + r.width() / 4, r.top() + r.height() / 4, r.width() / 2, r.height() / 2);
    setWindowTitle(tr("About"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    mTitleBarLayout = titleLayout;
    mainLayout->addLayout(titleLayout);

    titleLayout->addWidget(new QLabel(QString("<h1>%1 %2</h1>").arg(tr("Entropy Piano Tuner"), EPT_VERSION_STRING)));
    titleLayout->addStretch();

    QLabel *webpage = new QLabel("<a href=\"http://www.entropy-tuner.org\">www.entropy-tuner.org</a> - <a href=\"mailto:info@entropy-tuner.org\">Feedback</a>");
    QObject::connect(webpage, SIGNAL(linkActivated(QString)), this, SLOT(onOpenAboutLink(QString)));
    mainLayout->addWidget(webpage);

    QString iconPath = ":/media/images/icon_256x256" + iconPostfix + ".png";
    QTextBrowser *text = new QTextBrowser;
    text->setStyleSheet("background-color: transparent;");
    text->setFrameShape(QFrame::NoFrame);
    text->setOpenLinks(false);
    QObject::connect(text, SIGNAL(anchorClicked(QUrl)), this, SLOT(onOpenAboutUrl(QUrl)));
    mainLayout->addWidget(text);

    const QString buildText = tr("Built on %1").arg(QDateTime::fromString(__TIMESTAMP__).toString(Qt::DefaultLocaleLongDate));
    const QString buildByText = tr("by %1 and %2").arg("Prof. Dr. Haye Hinrichsen", "Christoph Wick M.Sc.");

    QString dependenciesText = tr("Based on");
    dependenciesText.append(" <a href=\"Qt\">Qt</a>, <a href=\"http://fftw.org\">fftw3</a>");
    dependenciesText.append(", <a href=\"http://www.grinninglizard.com/tinyxml2\">tinyxml2</a>");
    dependenciesText.append(", <a href=\"http://www.music.mcgill.ca/~gary/rtmidi\">RtMidi</a>");
#ifdef __ANDROID__
    dependenciesText.append(", <a href=\"https://github.com/kshoji/USB-MIDI-Driver\">USB-Midi-Driver</a>");
#endif

    const QString copyrightText = tr("Copyright 2015 Dept. of Theor. Phys. III, University of Würzburg. All rights reserved.");
    const QString licenseText = tr("This software is licensed unter the terms of the %1. The source code can be accessed at %2.").
            arg("<a href=\"http://www.gnu.org/licenses/gpl-3.0-standalone.html\">GPLv3</a>",
                "<a href=\"https://gitlab.com/entropytuner/Entropy-Piano-Tuner\">GitLab</a>");

    const QString warrantyText = tr("The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.");

    const QString acknowledgementsText = tr("We thank all those who have contributed to the project:") +
           " Prof. Dr. S. R. Dahmen, A. Frick, A. Heilrath, M. Jiminez, Prof. Dr. W. Kinzel, M. Kohl, L. Kusmierz, Prof. Dr. A. C. Lehmann, B. Olbrich.";

    auto makeParagraphTags = [](const QString &t) {return "<p>" + t + "</p>";};
    QString completeText;
    completeText.append("<html><img src=\"" + iconPath + "\" style=\"float: left;\"/>");

    completeText.append(makeParagraphTags(buildText));
    completeText.append(makeParagraphTags(buildByText));
    completeText.append(makeParagraphTags(dependenciesText));
    completeText.append(makeParagraphTags(copyrightText));
    completeText.append(makeParagraphTags(licenseText));
    completeText.append(makeParagraphTags(warrantyText));
    completeText.append(makeParagraphTags(acknowledgementsText));

    completeText.append("</html>");

    text->setHtml(completeText);

    QHBoxLayout *okButtonLayout = new QHBoxLayout;
    okButtonLayout->setMargin(0);
    mainLayout->addLayout(okButtonLayout);
    okButtonLayout->addStretch();

    QPushButton *okButton = new QPushButton(tr("Ok"));
    okButtonLayout->addWidget(okButton);

    QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    QScroller::grabGesture(text);
    text->setReadOnly(true);
    text->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);

    SHOW_DIALOG(this);

}

void AboutDialog::onOpenAboutUrl(QUrl url) {
    if (url.toString() == "Qt") {
        QMessageBox::aboutQt(this);
    } else {
        QDesktopServices::openUrl(url);
    }
}

void AboutDialog::onOpenAboutLink(QString link) {
    QDesktopServices::openUrl(QUrl::fromUserInput(link));
}

