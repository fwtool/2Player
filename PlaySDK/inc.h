#pragma once

#include "SDL.h"

#include <queue>

extern "C"
{
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/pixfmt.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libavutil/imgutils.h"

#include "libswresample/swresample.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
