///////////////////////////////////////////////////////////
//  MediaNotification.h
//  Implementation of the Class MediaNotification
///////////////////////////////////////////////////////////

#if !defined(EA_51D90FF0_CEC7_46f9_BAD1_0783CB83FD58__INCLUDED_)
#define EA_51D90FF0_CEC7_46f9_BAD1_0783CB83FD58__INCLUDED_

enum FileEvent
{
    FileAdd,
    FileRemove,
    DeviceAdd,
    DeviceRemove,
};

typedef struct
{
    char path[4096];
    int size;
}FileData;

typedef struct
{
    FileEvent event;
    FileData data;

}MediaNotification;
#endif // !defined(EA_51D90FF0_CEC7_46f9_BAD1_0783CB83FD58__INCLUDED_)
