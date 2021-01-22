#include "audio-service.h"
#include "audio-player.h"
#include "message-helper.h"
#include "message_queue_constant.h"
#include "log.h"

#include <string>

int main(int argc, char *argv[])
{
    /* Initialize Log class */
    Log::init("audioService", true);
    Log::info("<======= start audio service ========>\n");
    
    MessageHelper *msgHelper = MessageHelper::getInstance();

    /* Initialize gstreamer music player */
    AudioPlayer player(argc, argv);
    control_message data;
    while (true) {
        if (msgHelper->receiveMessage(&data)) {
            switch (data.msg_type) {
            case REQ_TYPE_SET_MEDIA:
                player.loadFromUri(string(data.textValue));
                break;
            case REQ_TYPE_GET_MEDIA:
                player.sendMediaName();
                break;
            case REQ_TYPE_PLAY:
                player.play();
                break;
            case REQ_TYPE_PAUSE:
                player.pause();
                break;
            case REQ_TYPE_STOP:
                player.stop();
                break;
            case REQ_TYPE_GET_DURATION:
                player.sendDuration();
                break;
            case REQ_TYPE_GET_STATE:
                player.sendState();
                break;
            case REQ_TYPE_SET_VOLUME:
                player.setVolume(data.intValue);
                break;
            case REQ_TYPE_GET_VOLUME:
                player.sendVolume();
                break;
            case REQ_TYPE_SET_POSITION:
                player.setPosition(data.intValue);
                break;
            case REQ_TYPE_GET_POSITION:
                player.sendPosition();
                break;
            case REQ_TYPE_GET_MEDIA_TITLE:
                player.sendTitle();
                break;
            case REQ_TYPE_GET_MEDIA_ARTIST:
                player.sendArtist();
                break;
            case REQ_TYPE_PLAY_MODE_CHANGED:
                player.changePlayMode(data.intValue);
                break;
            case REQ_TYPE_CONNECT_STATE_CHANGE:
            {
                bool isConnected = data.intValue == 0 ? false : true;
                msgHelper->onClientStateChanged(isConnected);
                player.onClientStateChanged(isConnected);
                break;
            }
            default:
                break;
            }
        }
    }

    return 0;
}

