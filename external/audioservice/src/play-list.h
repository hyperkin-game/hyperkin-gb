#ifndef MEDIALIST_H
#define MEDIALIST_H

#include <list>
#include <string>

using namespace std;

#define SEARCH_PATH "/mnt"

enum PlayMode
{
    PlayInOrder = 0,
    PlayRandom = 1,
    PlayOneCircle = 2
};

typedef list<string> SList;

class MediaList
{
public:
    MediaList();
    ~MediaList() {}

    void travelDir(const char *path);
    void updateList();

    void setPlayMode(PlayMode);
    string getNextSongPath();
    string getPathAt(int index);
    void onPlayItemChanged(string playItem);

private:
    int m_currentIndex;
    PlayMode m_playmode;
    SList  m_list;
    SList m_suffixs;

    void init();
};

#endif // MEDIALIST_H
