#ifndef BRIGHTNESSMAPPER_H
#define BRIGHTNESSMAPPER_H

#include <QImage>

class brightnessMapper
{
public:
    brightnessMapper();
    ~brightnessMapper();

public:
    void setRedMap(unsigned char red[]);
    void setGreenMap(unsigned char green[]);
    void setBlueMap(unsigned char blue[]);
    void setMap(unsigned char map[]);

    void updateBCG(double brightness, double contrast, double gamma);
    void setColorMap(unsigned char red[], unsigned char green[], unsigned char blue[]);

    void apply(const QImage &from, QImage &to);
    QImage apply(const QImage &from);

    unsigned char *redMap(){ return m_red;}
    unsigned char *blueMap(){return m_blue;}
    unsigned char *greenMap(){return m_green;}

    void redMap(double red[256]);
    void greenMap(double green[256]);
    void blueMap(double blue[256]);

private:
    unsigned char m_red[256];
    unsigned char m_green[256];
    unsigned char m_blue[256];
};

#endif // BRIGHTNESSMAPPER_H
