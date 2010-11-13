#ifndef UNITTESTCONFIG_H
#define UNITTESTCONFIG_H

#define CONSTRUCTOR "/Volumes/Macintosh HD"
#define ROOT_PATH "/"
#define NAME "Macintosh HD"
#define DEVICE "/dev/disk0s2"
#define FILESYSTEM "hfs"
#define TYPE QDriveInfo::InternalDrive
// next 2 values are retrieved for df utility
#define TOTALSIZE 195050360
#define BLOCKSIZE 512

#define DRIVEPATHS QStringList() << "/" << "/Volumes/Data HD" << "/home" << "/net";

#endif // UNITTESTCONFIG_H
