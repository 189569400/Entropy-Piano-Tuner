/*****************************************************************************
 * Copyright 2015 Haye Hinrichsen, Christoph Wick
 *
 * This file is part of Entropy Piano Tuner.
 *
 * Entropy Piano Tuner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Entropy Piano Tuner is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Entropy Piano Tuner. If not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

#ifndef CONFIG
#define CONFIG

// Assert mode:
//     1: release mode (throws in release mode)
//     2: exceptions mode (no asserts, throw exception instead)
//  else: normal mode
#define EPT_ASSERT_MODE                2

// Log level:
//     1: verbose
//     2: debug
//     3: information
//     4: warning
//     5: error
#define LOG_LEVEL 1

// Dialog size:
//     1: normal size (desktop)
//     2: maximized (mobile)
//     3: full screen (mobile)
// this is defined for each platform, default behaviour is 1
// CONFIG_DIALOG_SIZE 1

// Enabling what's this
#define EPT_WHATS_THIS_ENABLED         1

// Exclude the example algorithm
#define EPT_EXCLUDE_EXAMPLE_ALGORITHM  1

//#define CONFIG_ENABLE_OLD_RECORDER

#if __ANDROID__
// ======================================================================
// ANDROID
// ======================================================================

#define CONFIG_DIALOG_SIZE             2
#define CONFIG_OPTIMIZE_FFT            0
#define CONFIG_ENABLE_XMGRACE          0
#define CONFIG_USE_SIMPLE_FILE_DIALOG  1

#elif __linux__
// ======================================================================
// LINUX
// ======================================================================

#define CONFIG_ENABLE_RTMIDI           1
#define CONFIG_ENABLE_PULSEAUDIO       0
#define CONFIG_ENABLE_XMGRACE          0
#define CONFIG_OPTIMIZE_FFT            1
#define CONFIG_USE_SIMPLE_FILE_DIALOG  0
#define __LINUX_ALSA__ // for RtMidi

#elif __APPLE__
// ======================================================================
// APPLE
// ======================================================================

#define CONFIG_OPTIMIZE_FFT            0
#define CONFIG_ENABLE_RTMIDI           0
#define CONFIG_ENABLE_PULSEAUDIO       0
#define CONFIG_ENABLE_XMGRACE          0

#   if TARGET_IPHONE_SIMULATOR
#       define CONFIG_USE_SIMPLE_FILE_DIALOG 1
#       define CONFIG_DIALOG_SIZE            1
#   elif TARGET_OS_IPHONE
#       define CONFIG_USE_SIMPLE_FILE_DIALOG 1
#       define CONFIG_DIALOG_SIZE            1
#   endif
#

#elif _WIN32
// ======================================================================
// WINDOWS
// ======================================================================
#else

#error Unknown system

#endif


// compiler defines
#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

#ifndef CONFIG_DIALOG_SIZE
#   define CONFIG_DIALOG_SIZE 1
#endif

#endif // CONFIG

