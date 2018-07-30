#pragma once

//#define BETA true
//#define NONCOMMERCIAL true
#define DEBUGVERSION true

#define FBX4V_CHARPOINTER(Managed) ((const char*) (Marshal::StringToHGlobalAnsi(Managed)).ToPointer())
#define FBX4V_FBBA_SAMPSIZE 16*4
#define FBX4V_FBBA_METASIZE 12
#define FBX4V_VVVV_SPREADADD(spread, value) spread->SliceCount++;spread[-1] = value