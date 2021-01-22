#include "audio-player.h"
#include "message-helper.h"
#include "message_queue_constant.h"
#include "timer.h"
#include "log.h"

#define GSTREAMER_PLAYBIN_ELEMENT_NAME  "playbin"

static void timerCallback(AudioPlayer *player)
{
    player->updatePosition();
}

static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    reinterpret_cast<AudioPlayer*>(data)->processBusMessage(message);
    return TRUE;
}

gpointer busLoopThreadFunc(gpointer data)
{
    GMainLoop *loop;
    GstBus *bus;
    AudioPlayer *player;

    player = reinterpret_cast<AudioPlayer*>(data);
    if (player->getPlaybin()) {
        loop = g_main_loop_new(NULL, false);
        bus = gst_element_get_bus(player->getPlaybin());
        gst_bus_add_watch_full(bus, G_PRIORITY_DEFAULT, busCallback, player, NULL);
        g_main_loop_run(loop);
    }
}

AudioPlayer::AudioPlayer(int argc, char *argv[])
    : m_playbin(0)
    , m_msgHelper(0)
    , m_state(AudioPlayer::StoppedState)
    , m_mediaStatus(AudioPlayer::NoMedia)
    , m_duration(-1)
    , m_stopUpdatePosition(false)
    , m_lastPosition(0)
    , m_volume(-1)
    , m_tagList(NULL)
{
    gst_init(&argc, &argv);
    m_playbin = gst_element_factory_make(GSTREAMER_PLAYBIN_ELEMENT_NAME, NULL);
    if (m_playbin) {
        //create new thread to sort out messages
        g_thread_new("busLoopThreadFunc", busLoopThreadFunc, this);
    }

    m_msgHelper = MessageHelper::getInstance();

    m_list = new MediaList;
    m_list->updateList();
}

AudioPlayer::~AudioPlayer()
{
    if (m_playbin) {
        this->stop();
        gst_object_unref(GST_OBJECT(m_playbin));
    }

    delete m_list;
}

GstElement* AudioPlayer::getPlaybin()
{
    return m_playbin;
}

void AudioPlayer::onClientStateChanged(bool isConnected)
{   
    if (isConnected) {
        m_list->updateList();
        if (m_state != AudioPlayer::StoppedState) {
            m_msgHelper->sendMessage(RCV_TYPE_STATE_CHANGED, m_state);
            m_msgHelper->sendMessage(RCV_TYPE_DURATION_CHANGE, getDuration());
            m_msgHelper->sendMessage(RCV_TYPE_POSITION_CHANGE, position());
            m_msgHelper->sendMessage(RCV_TYPE_META_DATA_AVAILABLE);
        }
    }
}

void AudioPlayer::loadFromUri(string uri)
{
    gchar *g_uri;

    m_duration = -1;
    m_uri = uri;
    m_lastPosition = 0;

    m_list->updateList();
    m_list->onPlayItemChanged(m_uri);
    this->stop();

    if (m_playbin && !uri.empty()) {
        if (gst_uri_is_valid(uri.c_str()))
            g_uri = g_strdup(uri.c_str());
        else
            g_uri = gst_filename_to_uri(uri.c_str(), NULL);

        g_object_set(G_OBJECT(m_playbin), "uri", g_uri, NULL);
        m_mediaStatus = AudioPlayer::LoadingMedia;
    }
}

bool AudioPlayer::play()
{
    if (m_playbin && m_mediaStatus != AudioPlayer::NoMedia) {
        if (gst_element_set_state(m_playbin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
            Log::info("GStreamer; Unable to play - %s\n", m_uri.c_str());
        else
            return true;
    }

    return false;
}

bool AudioPlayer::pause()
{    
    if (m_playbin && m_mediaStatus != AudioPlayer::NoMedia) {
        if (gst_element_set_state(m_playbin, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
            Log::info("GStreamer; Unable to pause -%s\n", m_uri.c_str());
        else
            return true;
    }

    return false;
}

void AudioPlayer::stop()
{
    if (m_playbin) {
        gst_element_set_state(m_playbin, GST_STATE_NULL);

        m_lastPosition = 0;
        AudioPlayer::State oldState = m_state;
        m_state = AudioPlayer::StoppedState;
        m_mediaStatus = AudioPlayer::NoMedia;

        m_msgHelper->sendMessage(RCV_TYPE_POSITION_CHANGE, 0);
        if (oldState != m_state)
            m_msgHelper->sendMessage(RCV_TYPE_STATE_CHANGED, m_state);
    }
}

void AudioPlayer::updateList()
{
    m_list->updateList();
}

void AudioPlayer::nextSong()
{
    string nextPath = m_list->getNextSongPath();
    if (nextPath != string("")) {
        this->loadFromUri(nextPath);
        this->play();
    }
}

void AudioPlayer::changePlayMode(int playMode)
{
    m_list->setPlayMode(PlayMode(playMode));
}

gint64 AudioPlayer::position()
{
    gint64 position = 0;

    if (m_mediaStatus == AudioPlayer::EndOfMedia)
        return getDuration();

    if (m_playbin && gst_element_query_position(m_playbin, GST_FORMAT_TIME, &position))
        m_lastPosition = position / 1000000;

    if (m_lastPosition >= getDuration())
        m_lastPosition = getDuration();

    return m_lastPosition;
}

void AudioPlayer::sendPosition()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_POSITION, position());
}

void AudioPlayer::updatePosition()
{
    Timer timer;

    m_msgHelper->sendMessage(RCV_TYPE_POSITION_CHANGE, position());

    if (!m_stopUpdatePosition)
        timer.AsyncWait(200, timerCallback, this);
}

bool AudioPlayer::setPosition(gint64 pos)
{
    if (m_mediaStatus == AudioPlayer::EndOfMedia) {
        m_mediaStatus = AudioPlayer::LoadedMedia;
    }

    if (m_playbin && m_state != AudioPlayer::StoppedState) {
        pos = pos > 0 ? pos : 0;
        gint64 position = pos * 1000000;

        bool isSeeking = gst_element_seek(m_playbin,
                                          1.0,
                                          GST_FORMAT_TIME,
                                          GstSeekFlags(GST_SEEK_FLAG_FLUSH),
                                          GST_SEEK_TYPE_SET,
                                          position,
                                          GST_SEEK_TYPE_NONE,
                                          0);
        if (isSeeking) {
            m_lastPosition = pos;
        }
        return isSeeking;
    }

    return false;
}

gint64 AudioPlayer::getDuration()
{
    return m_duration;
}

void AudioPlayer::updateDuration()
{
    gint64 gstDuration = 0;
    int duration = 0;

    if (m_playbin && gst_element_query_duration(m_playbin, GST_FORMAT_TIME, &gstDuration))
        duration = gstDuration / 1000000;

    if (m_duration != duration && duration > 0) {
        m_duration = duration;
        m_msgHelper->sendMessage(RCV_TYPE_DURATION_CHANGE, m_duration);
    } else {
        updateDuration();
    }
}

void AudioPlayer::sendDuration()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_DURATION, m_duration);
}

void AudioPlayer::sendState()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_STATE, m_state);
}

void AudioPlayer::setVolume(int volume)
{
    if (m_playbin && m_volume != volume) {
        m_volume = volume;
        g_object_set(G_OBJECT(m_playbin), "volume", m_volume / 100.0, NULL);
    }
}

void AudioPlayer::sendVolume()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_VOLUME, m_volume);
}

void AudioPlayer::sendMediaName()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_MEDIA, 0, m_uri.c_str());
}

void AudioPlayer::sendTitle()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_MEDIA_TITLE, 0, m_mediaTitle.c_str());
}

void AudioPlayer::sendArtist()
{
    m_msgHelper->sendFeedbackMessage(REQ_TYPE_GET_MEDIA_ARTIST, 0, m_mediaArtist.c_str());
}

void AudioPlayer::processEOS()
{
    m_mediaStatus = AudioPlayer::EndOfMedia;

    m_msgHelper->sendMessage(RCV_TYPE_POSITION_CHANGE, position());
    m_msgHelper->sendMessage(RCV_TYPE_MEDIA_STATE_CHANGED, m_mediaStatus);

    if (m_state != AudioPlayer::StoppedState) {
        this->stop();
    }

    // Process next song path when the client not conneted in.
    if (!m_msgHelper->isClientConnected())
        nextSong();
}

void AudioPlayer::parseTagList(const GstTagList *tags)
{
    if (m_tagList == NULL || !gst_tag_list_is_equal(m_tagList, tags)) {
        gchar *str;
        
        if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &str))
            m_mediaArtist = str;

        if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &str))
            m_mediaTitle = str;

        m_msgHelper->sendMessage(RCV_TYPE_META_DATA_AVAILABLE);
        m_tagList = gst_tag_list_copy(tags);
    }
}

bool AudioPlayer::processBusMessage(GstMessage *msg)
{
    if (msg) {
        // tag message comes from elements inside playbin, not from playbin itself
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TAG) {
            GstTagList *tag_list;
            gst_message_parse_tag(msg, &tag_list);
            if (!gst_tag_list_is_empty(tag_list)) {
                parseTagList(tag_list);
            }
            gst_tag_list_free(tag_list);
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_DURATION) {
            m_duration = -1;
            updateDuration();
        }
        
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT_CAST(m_playbin)) {
            switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_STATE_CHANGED: {
                GstState oldState;
                GstState newState;
                GstState pending;
                
                gst_message_parse_state_changed(msg, &oldState, &newState, &pending);
                switch (newState) {
                case GST_STATE_VOID_PENDING:
                case GST_STATE_NULL:
                    if (m_state != AudioPlayer::StoppedState) {
                        m_state = AudioPlayer::StoppedState;
                        m_msgHelper->sendMessage(RCV_TYPE_STATE_CHANGED, m_state);
                    }

                    m_stopUpdatePosition = true;
                    m_mediaStatus = AudioPlayer::NoMedia;
                    break;
                case GST_STATE_READY:
                    if (m_state != AudioPlayer::StoppedState) {
                        m_state = AudioPlayer::StoppedState;
                        m_msgHelper->sendMessage(RCV_TYPE_STATE_CHANGED, m_state);

                        m_stopUpdatePosition = true;
                    }

                    if (m_mediaStatus != AudioPlayer::LoadedMedia) {
                        m_mediaStatus = AudioPlayer::LoadedMedia;
                        m_msgHelper->sendMessage(RCV_TYPE_MEDIA_STATE_CHANGED, m_mediaStatus);
                    }
                    break;
                case GST_STATE_PAUSED:
                {
                    AudioPlayer::State prevState = m_state;
                    m_state = AudioPlayer::PausedState;

                    if (m_state != prevState)
                        m_msgHelper->sendMessage(RCV_TYPE_STATE_CHANGED, m_state);

                    if (m_mediaStatus != AudioPlayer::LoadedMedia) {
                        m_mediaStatus = AudioPlayer::LoadedMedia;
                        m_msgHelper->sendMessage(RCV_TYPE_MEDIA_STATE_CHANGED, m_mediaStatus);
                    }
                    m_stopUpdatePosition = true;
                    break;
                }
                case GST_STATE_PLAYING:
                    if (m_state != AudioPlayer::PlayingState) {
                        m_state = AudioPlayer::PlayingState;
                        m_msgHelper->sendMessage(RCV_TYPE_STATE_CHANGED, m_state);

                        if (m_duration <= 0) {
                            updateDuration();
                        }

                        m_stopUpdatePosition = false;
                        updatePosition();
                    }

                    if (m_mediaStatus != AudioPlayer::LoadedMedia) {
                        m_mediaStatus = AudioPlayer::LoadedMedia;
                        m_msgHelper->sendMessage(RCV_TYPE_MEDIA_STATE_CHANGED, m_mediaStatus);
                    }
                    break;
                }
                break;
            }
            case GST_MESSAGE_EOS:
                processEOS();
                break;
            case GST_MESSAGE_TAG:
            case GST_MESSAGE_STREAM_STATUS:
            case GST_MESSAGE_UNKNOWN:
                break;
            case GST_MESSAGE_ERROR:
            {
                GError *err;
                gchar *debug;
                Error error;

                gst_message_parse_error(msg, &err, &debug);
                if (err->domain == GST_STREAM_ERROR && err->code == GST_STREAM_ERROR_CODEC_NOT_FOUND)
                    error = AudioPlayer::FormatError;
                else
                    error = AudioPlayer::ResourceError;

                m_msgHelper->sendMessage(RCV_TYPE_ERROR, error);

                // Play next song when there is no client connected in.
                if (!m_msgHelper->isClientConnected()) {
                    this->updateList();
                    this->nextSong();
                }

                g_error_free(err);
                g_free(debug);
                break;
            }
            case GST_MESSAGE_SEGMENT_START:
            {
                const GstStructure *structure = gst_message_get_structure(msg);
                gint64 position = g_value_get_int64(gst_structure_get_value(structure, "position"));
                position /= 1000000;

                m_lastPosition = position;
                m_msgHelper->sendMessage(RCV_TYPE_POSITION_CHANGE, m_lastPosition);
                break;
            }
            case GST_MESSAGE_ASYNC_DONE:
            {
                gint64  position = 0;
                if (gst_element_query_position(m_playbin, GST_FORMAT_TIME, &position)) {
                    position /= 1000000;
                    m_lastPosition = position;
                    m_msgHelper->sendMessage(RCV_TYPE_POSITION_CHANGE, m_lastPosition);
                }
                break;
            }
            default:
                break;
            }
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            m_msgHelper->sendMessage(RCV_TYPE_ERROR, AccessDeniedError);
            
            // Play next song when there is no client connected in.
            if (!m_msgHelper->isClientConnected()) {
                this->updateList();
                this->nextSong();
            }
        }
    }
    
    return true;
}
