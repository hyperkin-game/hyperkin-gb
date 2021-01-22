#include "brightnessmapper.h"

#include <math.h>

brightnessMapper::brightnessMapper()
{
    for (int i = 0; i < 256; i++) {
        m_red[i] = i;
        m_green[i] = i;
        m_blue[i] = i;
    }
}

brightnessMapper::~brightnessMapper()
{
}

void brightnessMapper::updateBCG(double brightness, double contrast, double gamma)
{
    double x, y;
    for (int i = 0; i < 256; i++) {
        x = i / 255.0;
        y = exp(log(x) * gamma);
        y = (y - 0.5) * brightness + 0.5 + contrast / 255;
        y = y * 255.0;
        m_red[i] = qBound(0.0, y, 255.0);
        m_green[i] = m_red[i];
        m_blue[i] = m_red[i];
    }
}

QImage brightnessMapper::apply(const QImage &from)
{
    QImage to = from;
    apply(from, to);
    return to;
}

void brightnessMapper::apply(const QImage &from, QImage &to)
{
    if (to.size() != from.size() || to.format() != from.format())
        to = from.copy();

    int height = from.height();
    int width = from.width();

    switch (from.format()) {
    case QImage::Format_Indexed8:
        for (int i = 0; i < height; i++) {
            const uchar *pFrom = (const uchar*)from.constScanLine(i);
            uchar *pTo = (uchar*)to.scanLine(i);
            for (int j = 0; j < width; j++)
                pTo[j] = m_red[pFrom[i]];
        }
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        for (int i = 0; i < height; i++) {
            const QRgb *pFrom = (const QRgb*)from.constScanLine(i);
            QRgb *pTo = (QRgb*)to.scanLine(i);
            for (int j = 0; j < width; j++) {
                int r, g, b;
                r = qRed(pFrom[j]);
                g = qGreen(pFrom[j]);
                b = qBlue(pFrom[j]);
                r = m_red[r];
                g = m_green[g];
                b = m_blue[b];
                pTo[j] = qRgb(r, g, b);
            }
        }
        break;
    default:
        break;
    }
}

void brightnessMapper::setRedMap(unsigned char red[])
{
    for (int i = 0; i < 256; i++)
        m_red[i] = red[i];
}

void brightnessMapper::setGreenMap(unsigned char green[])
{
    for (int i = 0; i < 256; i++)
        m_green[i] = green[i];
}

void brightnessMapper::setBlueMap(unsigned char blue[])
{
    for (int i = 0; i < 256; i++)
        m_blue[i] = blue[i];
}

void brightnessMapper::setMap(unsigned char map[])
{
    for (int i = 0; i < 256; i++) {
        m_red[i] = map[i];
        m_green[i] = map[i];
        m_blue[i] = map[i];
    }
}

void brightnessMapper::setColorMap(unsigned char red[], unsigned char green[], unsigned char blue[])
{
    for (int i = 0; i < 256; i++) {
        m_red[i] = red[i];
        m_green[i] = green[i];
        m_blue[i] = blue[i];
    }
}
void brightnessMapper::redMap(double red[256])
{
    for (int i = 0; i < 256; i++)
        red[i] = m_red[i];
}

void brightnessMapper::greenMap(double green[256])
{
    for (int i = 0; i < 256; i++)
        green[i] = m_green[i];
}

void brightnessMapper::blueMap(double blue[256])
{
    for (int i = 0; i < 256; i++)
        blue[i] = m_blue[i];
}
