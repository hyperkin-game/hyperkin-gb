#include "play-list.h"

#include <string.h>
#include <dirent.h>

MediaList::MediaList()
    : m_currentIndex(-1)
    , m_playmode(PlayInOrder)
{
    init();
}

void MediaList::init()
{
    m_suffixs.push_back("mp3");
    m_suffixs.push_back("wave");
    m_suffixs.push_back("wma");
    m_suffixs.push_back("ogg");
    m_suffixs.push_back("midi");
    m_suffixs.push_back("ra");
    m_suffixs.push_back("mod");
    m_suffixs.push_back("mp1");
    m_suffixs.push_back("mp2");
    m_suffixs.push_back("wav");
    m_suffixs.push_back("flac");
    m_suffixs.push_back("aac");
    m_suffixs.push_back("m4a");
}

void MediaList::setPlayMode(PlayMode playmode)
{
    m_playmode = playmode;
}

string MediaList::getPathAt(int index)
{
    if (m_list.empty() || index >= m_list.size() || index < 0)
        return string("");

    m_currentIndex = index;

    SList::iterator iter = m_list.begin();
    for (int i = 0; i < index; i++)
        iter++;

    return (*iter);
}

string MediaList::getNextSongPath()
{
    switch (m_playmode) {
    case PlayOneCircle:
        break;
    case PlayInOrder:
        if (m_currentIndex + 1 >= m_list.size())
            m_currentIndex = 0;
        else
            m_currentIndex++;
        break;
    case PlayRandom: {
        int xxx = rand() % m_list.size();
        m_currentIndex = xxx;
        break;
    }
    default:
        break;
    }

    return getPathAt(m_currentIndex);
}

void MediaList::onPlayItemChanged(string playItem)
{
    bool hasFind = false;

    SList::iterator iter = m_list.begin();
    for (int i = 0; i < m_list.size(); i++) {
        if ((*iter) == playItem) {
            m_currentIndex = i;
            hasFind = true;
            break;
        }
        iter++;
    }

    if (!hasFind) {
        m_currentIndex = 0;
        updateList();
    }
}

void MediaList::travelDir(const char *path)
{
    DIR *dp;
    struct dirent* file;
    char filePath[512];

    dp = opendir(path);
    if (dp == NULL)
        return;

    while ((file = readdir(dp)) != NULL) {
        if (strncmp(file->d_name, ".", 1) == 0)
            continue;

        memset(filePath, 0, sizeof(filePath));
        sprintf(filePath, "%s/%s", path, file->d_name);

        if (file->d_type == DT_DIR) {
            travelDir(filePath);
        } else {
            string filePathStr = string(filePath);
            // filter file format
            SList::iterator iter;
            for (iter = m_suffixs.begin(); iter != m_suffixs.end(); iter++) {
                string suffix = string(".") + *iter;
                if (filePathStr.find(suffix) != string::npos) {
                    m_list.push_back(filePathStr);
                    break;
                }
            }
        }
    }

    (void)closedir(dp);
}

void MediaList::updateList()
{
    m_list.clear();
    travelDir((char*)SEARCH_PATH);
}
