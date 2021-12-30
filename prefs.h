#ifndef PREFS_H
#define PREFS_H

#include <QWidget>
#include <QFile>
#include <QApplication>

class Prefs {

public:
    const QObject *mainw_ptr;
    bool   BORDER_SETTING = true;       //true: black border only. false: border color = outer edge pixels

    int    SKIP_EDGE_PIXELS = 64;       //pick every nth pixel when checking edge for a border
    double EDGE_BORDER_RATIO = 0.20;    //same color must be this much of edge for image to have a border
    double MAX_BORDER_PERCENT = 0.95;   //up to 95% of image can be border
    int    DEFAULT_DEVIATION = 30;      //don't seek more than 30px for edge
    int    BORDER_THRESHOLD = 35;       //border/image color difference
    int    NOT_A_BORDER = 4;            //differing pixels needed for row/col to not be border
    QString IMAGE_FORMATS = "*.jp*g";   //search for this (these) image file formats

    Prefs(const QObject *main_window) : mainw_ptr(main_window) {
        QFile file(QStringLiteral("%1/settings.ini").arg(QApplication::applicationDirPath()));
        if (!file.open(QIODevice::ReadOnly))
            return;

        QTextStream text(&file);
        while (!text.atEnd()) {
            const QString line = text.readLine();
            if(line.startsWith(QStringLiteral(";")) || line.isEmpty())
                continue;

            if (line.startsWith("SKIP_EDGE_PIXELS", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const int pixels = substrings.at(1).toInt();
                    if (pixels && pixels > 0 && pixels <= 256)
                        SKIP_EDGE_PIXELS = pixels;
                }
            }
            if (line.startsWith("EDGE_BORDER_RATIO", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const double ratio = substrings.at(1).toDouble();
                    if (ratio && ratio > 0 && ratio <= 100.0)
                        EDGE_BORDER_RATIO = ratio;
                }
            }
            if (line.startsWith("MAX_BORDER_PERCENT", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const double percent = substrings.at(1).toDouble();
                    if (percent && percent > 0 && percent <= 100.0)
                        MAX_BORDER_PERCENT = percent;
                }
            }
            if (line.startsWith("DEFAULT_DEVIATION", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const int pixels = substrings.at(1).toInt();
                    if (pixels && pixels > 0 && pixels <= 64)
                        DEFAULT_DEVIATION = pixels;
                }
            }
            if (line.startsWith("BORDER_THRESHOLD", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const int pixels = substrings.at(1).toInt();
                    if (pixels && pixels > 0 && pixels <= 128)
                        BORDER_THRESHOLD = pixels;
                }
            }
            if (line.startsWith("NOT_A_BORDER", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const int pixels = substrings.at(1).toInt();
                    if (pixels && pixels > 0 && pixels <= 32)
                        NOT_A_BORDER = pixels;
                }
            }
            if (line.startsWith("IMAGE_FORMATS", Qt::CaseInsensitive)) {
                const QStringList substrings = line.split("=");
                if (!substrings.empty() && substrings.length() == 2) {
                    const QString formats = substrings.at(1);
                    if (!formats.isEmpty())
                        IMAGE_FORMATS = formats;
                }
            }
        }
        file.close();
    };

};

#endif // PREFS_H
