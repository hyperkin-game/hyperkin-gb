#ifndef VIDEOINFOUTIL_H
#define VIDEOINFOUTIL_H

#include <QString>

class VideoInfoUtil
{
public:
    VideoInfoUtil(){}

    /**
     * check whether video resolution is greater than 1920 x 1080
     * note: provided by ffmepg tool
     * @return check result
     */
    static bool isVideoSolutionSuitable(const QString&);
};

#endif // VIDEOINFOUTIL_H
