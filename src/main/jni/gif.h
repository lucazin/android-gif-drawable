#include <unistd.h>
#include <jni.h>
#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <malloc.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <sys/cdefs.h>
#include "giflib/gif_lib.h"

//#include <android/log.h>
//#define  LOG_TAG    "libgif"
//#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define GET_ADDR(bm, width, left, top) bm + top * width + left

/**
 * Some gif files are not strictly follow 89a.
 * DGifSlurp will return read head error or get record type error.
 * but the image still can display. so here should ignore the error.
 */
//#define STRICT_FORMAT_89A

/**
 * Decoding error - no frames
 */
#define D_GIF_ERR_NO_FRAMES     	1000
/**
* Decoding error - invalid GIF screen size
*/
#define D_GIF_ERR_INVALID_SCR_DIMS 	1001
/**
* Decoding error - invalid frame size
*/
#define D_GIF_ERR_INVALID_IMG_DIMS 	1002
/**
* Decoding error - frame size is greater that screen size
*/
#define D_GIF_ERR_IMG_NOT_CONFINED 	1003
/**
* Decoding error - input source rewind failed
*/
#define D_GIF_ERR_REWIND_FAILED 	1004

#define ILLEGAL_STATE_EXCEPTION "java/lang/IllegalStateException"

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
    uint8_t alpha;
} argb;


typedef struct
{
	unsigned int duration;
	int transpIndex;
	unsigned char disposalMethod;
} FrameInfo;

typedef struct GifInfo GifInfo;
typedef int
(*RewindFunc)(GifInfo *);

struct GifInfo
{
	GifFileType* gifFilePtr;
    time_t lastFrameRemainder;
    time_t nextStartTime;
	int currentIndex;
    FrameInfo* infos;
	argb* backupPtr;
	long startPos;
	unsigned char* rasterBits;
	char* comment;
	unsigned short loopCount;
	int currentLoop;
	RewindFunc rewindFunction;
	jfloat speedFactor;
	void* surfaceBackupPtr;
};

typedef struct
{
	jobject stream;
	jclass streamCls;
	jmethodID readMID;
	jmethodID resetMID;
	jbyteArray buffer;
} StreamContainer;

typedef struct
{
	long pos;
	jbyteArray buffer;
	jsize arrLen;
} ByteArrayContainer;

typedef struct
{
	long pos;
	jbyte* bytes;
	jlong capacity;
} DirectByteBufferContainer;

/**
* Generates default color map, used when there is no color map defined in GIF file.
* Upon successful allocation in JNI_OnLoad it is stored for further use.
*
*/
static ColorMapObject *genDefColorMap(void);

/**
* @return the real time, in ms
*/
static inline time_t getRealTime(void);

/**
* Frees dynamically allocated memory
*/
static void cleanUp(GifInfo *info);