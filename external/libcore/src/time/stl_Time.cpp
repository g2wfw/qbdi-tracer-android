/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/
#include "stl_core.h"
#include "text/stl_String.h"
#include "text/stl_StringRef.h"
#include "memory/stl_HeapBlock.h"
#include "time/stl_Time.h"
#include "threads/stl_Thread.h"
#include "text/stl_LocalisedStrings.h"

namespace stl {

    namespace TimeHelpers {
        static std::tm millisToLocal(int64 millis) noexcept {
#if STL_WINDOWS && STL_MINGW
            auto now = (time_t) (millis / 1000);
            return *localtime (&now);

#elif STL_WINDOWS
            std::tm result;
            millis /= 1000;

            if (_localtime64_s (&result, &millis) != 0)
                zerostruct (result);

            return result;

#else
            std::tm result;
            auto now = (time_t) (millis / 1000);

            if (localtime_r(&now, &result) == nullptr)
                zerostruct(result);

            return result;
#endif
        }

        static std::tm millisToUTC(int64 millis) noexcept {
#if STL_WINDOWS && STL_MINGW
            auto now = (time_t) (millis / 1000);
            return *gmtime (&now);

#elif STL_WINDOWS
            std::tm result;
            millis /= 1000;

            if (_gmtime64_s (&result, &millis) != 0)
                zerostruct (result);

            return result;

#else
            std::tm result;
            auto now = (time_t) (millis / 1000);

            if (gmtime_r(&now, &result) == nullptr)
                zerostruct(result);

            return result;
#endif
        }

        static int getUTCOffsetSeconds(const int64 millis) noexcept {
            auto utc = millisToUTC(millis);
            utc.tm_isdst = -1;  // Treat this UTC time as local to find the offset

            return (int) ((millis / 1000) - (int64) mktime(&utc));
        }

        static int extendedModulo(const int64 value, const int modulo) noexcept {
            return (int) (value >= 0 ? (value % modulo)
                                     : (value - ((value / modulo) + 1) * modulo));
        }

        static String formatString(const String &format, const std::tm *const tm) {
#if STL_ANDROID
            using StringType = CharPointer_UTF8;
#elif STL_WINDOWS
            using StringType = CharPointer_UTF16;
#else
            using StringType = CharPointer_UTF32;
#endif

#ifdef STL_MSVC
            if (tm->tm_year < -1900 || tm->tm_year > 8099)
                return {};   // Visual Studio's library can only handle 0 -> 9999 AD
#endif

            for (size_t bufferSize = 256;; bufferSize += 256) {
                HeapBlock<StringType::CharType> buffer(bufferSize);

                auto numChars =
#if STL_ANDROID
                        strftime (buffer, bufferSize - 1, format.toUTF8(), tm);
#elif STL_WINDOWS
                        wcsftime (buffer, bufferSize - 1, format.toWideCharPointer(), tm);
#else
                        wcsftime(buffer, bufferSize - 1, format.toUTF32(), tm);
#endif

                if (numChars > 0 || format.isEmpty())
                    return String(StringType(buffer),
                                  StringType(buffer) + (int) numChars);
            }
        }

        //==============================================================================
        static bool isLeapYear(int year) noexcept {
            return (year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0));
        }

        static int daysFromJan1(int year, int month) noexcept {
            const short dayOfYear[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
                                       0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

            return dayOfYear[(isLeapYear(year) ? 12 : 0) + month];
        }

        static int64 daysFromYear0(int year) noexcept {
            --year;
            return 365 * year + (year / 400) - (year / 100) + (year / 4);
        }

        static int64 daysFrom1970(int year) noexcept {
            return daysFromYear0(year) - daysFromYear0(1970);
        }

        static int64 daysFrom1970(int year, int month) noexcept {
            if (month > 11) {
                year += month / 12;
                month %= 12;
            } else if (month < 0) {
                auto numYears = (11 - month) / 12;
                year -= numYears;
                month += 12 * numYears;
            }

            return daysFrom1970(year) + daysFromJan1(year, month);
        }

        // There's no posix function that does a UTC version of mktime,
        // so annoyingly we need to implement this manually..
        static int64 mktime_utc(const std::tm &t) noexcept {
            return 24 * 3600 * (daysFrom1970(t.tm_year + 1900, t.tm_mon) + (t.tm_mday - 1))
                   + 3600 * t.tm_hour
                   + 60 * t.tm_min
                   + t.tm_sec;
        }

        static Atomic<uint32> lastMSCounterValue{(uint32) 0};

        static String getUTCOffsetString(int utcOffsetSeconds, bool includeSemiColon) {
            if (const auto seconds = utcOffsetSeconds) {
                auto minutes = seconds / 60;

                return String::formatted(includeSemiColon ? "%+03d:%02d"
                                                          : "%+03d%02d",
                                         minutes / 60,
                                         abs(minutes) % 60);
            }

            return "Z";
        }
    }

//==============================================================================
    Time::Time(int64 ms) noexcept: millisSinceEpoch(ms) {}

    Time::Time(int year, int month, int day,
               int hours, int minutes, int seconds, int milliseconds,
               bool useLocalTime) noexcept {
        std::tm t;
        t.tm_year = year - 1900;
        t.tm_mon = month;
        t.tm_mday = day;
        t.tm_hour = hours;
        t.tm_min = minutes;
        t.tm_sec = seconds;
        t.tm_isdst = -1;

        millisSinceEpoch = 1000 * (useLocalTime ? (int64) mktime(&t)
                                                : TimeHelpers::mktime_utc(t))
                           + milliseconds;
    }

//==============================================================================
    int64 Time::currentTimeMillis() noexcept {
#if STL_WINDOWS && !STL_MINGW
        struct _timeb t;
        _ftime_s (&t);
        return ((int64) t.time) * 1000 + t.millitm;
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return ((int64) tv.tv_sec) * 1000 + tv.tv_usec / 1000;
#endif
    }

    Time STL_CALLTYPE Time::getCurrentTime() noexcept {
        return Time(currentTimeMillis());
    }

//==============================================================================
    uint32 stl_millisecondsSinceStartup() noexcept;

    uint32 Time::getMillisecondCounter() noexcept {
        auto now = stl_millisecondsSinceStartup();

        if (now < TimeHelpers::lastMSCounterValue.get()) {
            // in multi-threaded apps this might be called concurrently, so
            // make sure that our last counter value only increases and doesn't
            // go backwards..
            if (now < TimeHelpers::lastMSCounterValue.get() - (uint32) 1000)
                TimeHelpers::lastMSCounterValue = now;
        } else {
            TimeHelpers::lastMSCounterValue = now;
        }

        return now;
    }

    uint32 Time::getApproximateMillisecondCounter() noexcept {
        auto t = TimeHelpers::lastMSCounterValue.get();
        return t == 0 ? getMillisecondCounter() : t;
    }

    void Time::waitForMillisecondCounter(uint32 targetTime) noexcept {
        for (;;) {
            auto now = getMillisecondCounter();

            if (now >= targetTime)
                break;

            auto toWait = (int) (targetTime - now);

            if (toWait > 2) {
                Thread::sleep(jmin(20, toWait >> 1));
            } else {
                // xxx should consider using mutex_pause on the mac as it apparently
                // makes it seem less like a spinlock and avoids lowering the thread pri.
                for (int i = 10; --i >= 0;)
                    Thread::yield();
            }
        }
    }

//==============================================================================
    double Time::highResolutionTicksToSeconds(const int64 ticks) noexcept {
        return (double) ticks / (double) getHighResolutionTicksPerSecond();
    }

    int64 Time::secondsToHighResolutionTicks(const double seconds) noexcept {
        return (int64) (seconds * (double) getHighResolutionTicksPerSecond());
    }

//==============================================================================
    String Time::toString(bool includeDate,
                          bool includeTime,
                          bool includeSeconds,
                          bool use24HourClock) const {
        String result;

        if (includeDate) {
            result << getDayOfMonth() << ' '
                   << getMonthName(true) << ' '
                   << getYear();

            if (includeTime)
                result << ' ';
        }

        if (includeTime) {
            auto mins = getMinutes();

            result << (use24HourClock ? getHours() : getHoursInAmPmFormat())
                   << (mins < 10 ? ":0" : ":") << mins;

            if (includeSeconds) {
                auto secs = getSeconds();
                result << (secs < 10 ? ":0" : ":") << secs;
            }

            if (!use24HourClock)
                result << (isAfternoon() ? "pm" : "am");
        }

        return result.trimEnd();
    }

    String Time::formatted(const String &format) const {
        std::tm t(TimeHelpers::millisToLocal(millisSinceEpoch));
        return TimeHelpers::formatString(format, &t);
    }

//==============================================================================
    int Time::getYear() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_year + 1900; }

    int Time::getMonth() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_mon; }

    int Time::getDayOfYear() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_yday; }

    int Time::getDayOfMonth() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_mday; }

    int Time::getDayOfWeek() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_wday; }

    int Time::getHours() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_hour; }

    int Time::getMinutes() const noexcept { return TimeHelpers::millisToLocal(millisSinceEpoch).tm_min; }

    int Time::getSeconds() const noexcept { return TimeHelpers::extendedModulo(millisSinceEpoch / 1000, 60); }

    int Time::getMilliseconds() const noexcept { return TimeHelpers::extendedModulo(millisSinceEpoch, 1000); }

    int Time::getHoursInAmPmFormat() const noexcept {
        auto hours = getHours();

        if (hours == 0) return 12;
        if (hours <= 12) return hours;

        return hours - 12;
    }

    bool Time::isAfternoon() const noexcept {
        return getHours() >= 12;
    }

    bool Time::isDaylightSavingTime() const noexcept {
        return TimeHelpers::millisToLocal(millisSinceEpoch).tm_isdst != 0;
    }

    String Time::getTimeZone() const {
        String zone[2];

#if STL_WINDOWS && (STL_MSVC || STL_CLANG)
        _tzset();

        for (int i = 0; i < 2; ++i)
        {
            char name[128] = { 0 };
            size_t length;
            _get_tzname (&length, name, sizeof (name) - 1, i);
            zone[i] = name;
        }
#else
        tzset();

        auto zonePtr = (const char **) tzname;
        zone[0] = zonePtr[0];
        zone[1] = zonePtr[1];
#endif

        if (isDaylightSavingTime()) {
            zone[0] = zone[1];

            if (zone[0].length() > 3
                && zone[0].containsIgnoreCase("daylight")
                && zone[0].contains("GMT"))
                zone[0] = "BST";
        }

        return zone[0].substring(0, 3);
    }

    int Time::getUTCOffsetSeconds() const noexcept {
        return TimeHelpers::getUTCOffsetSeconds(millisSinceEpoch);
    }

    String Time::getUTCOffsetString(bool includeSemiColon) const {
        return TimeHelpers::getUTCOffsetString(getUTCOffsetSeconds(), includeSemiColon);
    }

    String Time::toISO8601(bool includeDividerCharacters) const {
        return String::formatted(includeDividerCharacters ? "%04d-%02d-%02dT%02d:%02d:%06.03f"
                                                          : "%04d%02d%02dT%02d%02d%06.03f",
                                 getYear(),
                                 getMonth() + 1,
                                 getDayOfMonth(),
                                 getHours(),
                                 getMinutes(),
                                 getSeconds() + getMilliseconds() / 1000.0)
               + getUTCOffsetString(includeDividerCharacters);
    }

    static int parseFixedSizeIntAndSkip(String::CharPointerType &t, int numChars, char charToSkip) noexcept {
        int n = 0;

        for (int i = numChars; --i >= 0;) {
            auto digit = (int) (*t - '0');

            if (!isPositiveAndBelow(digit, 10))
                return -1;

            ++t;
            n = n * 10 + digit;
        }

        if (charToSkip != 0 && *t == (stl_wchar) charToSkip)
            ++t;

        return n;
    }

    Time Time::fromISO8601(StringRef iso) {
        auto t = iso.text;
        auto year = parseFixedSizeIntAndSkip(t, 4, '-');

        if (year < 0)
            return {};

        auto month = parseFixedSizeIntAndSkip(t, 2, '-');

        if (month < 0)
            return {};

        auto day = parseFixedSizeIntAndSkip(t, 2, 0);

        if (day < 0)
            return {};

        int hours = 0, minutes = 0, milliseconds = 0;

        if (*t == 'T') {
            ++t;
            hours = parseFixedSizeIntAndSkip(t, 2, ':');

            if (hours < 0)
                return {};

            minutes = parseFixedSizeIntAndSkip(t, 2, ':');

            if (minutes < 0)
                return {};

            auto seconds = parseFixedSizeIntAndSkip(t, 2, 0);

            if (seconds < 0)
                return {};

            if (*t == '.' || *t == ',') {
                ++t;
                milliseconds = parseFixedSizeIntAndSkip(t, 3, 0);

                if (milliseconds < 0)
                    return {};
            }

            milliseconds += 1000 * seconds;
        }

        auto nextChar = t.getAndAdvance();

        if (nextChar == '-' || nextChar == '+') {
            auto offsetHours = parseFixedSizeIntAndSkip(t, 2, ':');

            if (offsetHours < 0)
                return {};

            auto offsetMinutes = parseFixedSizeIntAndSkip(t, 2, 0);

            if (offsetMinutes < 0)
                return {};

            auto offsetMs = (offsetHours * 60 + offsetMinutes) * 60 * 1000;
            milliseconds += nextChar == '-' ? offsetMs : -offsetMs; // NB: this seems backwards but is correct!
        } else if (nextChar != 0 && nextChar != 'Z') {
            return {};
        }

        return Time(year, month - 1, day, hours, minutes, 0, milliseconds, false);
    }

    String Time::getMonthName(const bool threeLetterVersion) const {
        return getMonthName(getMonth(), threeLetterVersion);
    }

    String Time::getWeekdayName(const bool threeLetterVersion) const {
        return getWeekdayName(getDayOfWeek(), threeLetterVersion);
    }

    static const char *const shortMonthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
                                                  "Nov", "Dec"};
    static const char *const longMonthNames[] = {"January", "February", "March", "April", "May", "June", "July",
                                                 "August", "September", "October", "November", "December"};

    String Time::getMonthName(int monthNumber, const bool threeLetterVersion) {
        monthNumber %= 12;

        return TRANS (threeLetterVersion ? shortMonthNames[monthNumber]
                                         : longMonthNames[monthNumber]);
    }

    String Time::getWeekdayName(int day, const bool threeLetterVersion) {
        static const char *const shortDayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        static const char *const longDayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
                                                   "Saturday"};

        day %= 7;

        return TRANS (threeLetterVersion ? shortDayNames[day]
                                         : longDayNames[day]);
    }

//==============================================================================
    Time &Time::operator+=(RelativeTime delta) noexcept {
        millisSinceEpoch += delta.inMilliseconds();
        return *this;
    }

    Time &Time::operator-=(RelativeTime delta) noexcept {
        millisSinceEpoch -= delta.inMilliseconds();
        return *this;
    }

    Time operator+(Time time, RelativeTime delta) noexcept {
        Time t(time);
        return t += delta;
    }

    Time operator-(Time time, RelativeTime delta) noexcept {
        Time t(time);
        return t -= delta;
    }

    Time operator+(RelativeTime delta, Time time) noexcept {
        Time t(time);
        return t += delta;
    }

    RelativeTime operator-(Time time1, Time time2) noexcept {
        return RelativeTime::milliseconds(time1.toMilliseconds() - time2.toMilliseconds());
    }

    bool operator==(Time time1, Time time2) noexcept { return time1.toMilliseconds() == time2.toMilliseconds(); }

    bool operator!=(Time time1, Time time2) noexcept { return time1.toMilliseconds() != time2.toMilliseconds(); }

    bool operator<(Time time1, Time time2) noexcept { return time1.toMilliseconds() < time2.toMilliseconds(); }

    bool operator>(Time time1, Time time2) noexcept { return time1.toMilliseconds() > time2.toMilliseconds(); }

    bool operator<=(Time time1, Time time2) noexcept { return time1.toMilliseconds() <= time2.toMilliseconds(); }

    bool operator>=(Time time1, Time time2) noexcept { return time1.toMilliseconds() >= time2.toMilliseconds(); }

    static int getMonthNumberForCompileDate(const String &m) {
        for (int i = 0; i < 12; ++i)
            if (m.equalsIgnoreCase(shortMonthNames[i]))
                return i;

        // If you hit this because your compiler has an unusual __DATE__
        // format, let us know so we can add support for it!
        jassertfalse;
        return 0;
    }

    Time Time::getCompilationDate() {
        StringArray dateTokens, timeTokens;

        dateTokens.addTokens(__DATE__, true);
        dateTokens.removeEmptyStrings(true);

        timeTokens.addTokens(__TIME__, ":", StringRef());

        return Time(dateTokens[2].getIntValue(),
                    getMonthNumberForCompileDate(dateTokens[0]),
                    dateTokens[1].getIntValue(),
                    timeTokens[0].getIntValue(),
                    timeTokens[1].getIntValue());
    }

    String Time::currentTimeFormatted() {
        stl::Time time(stl::Time::currentTimeMillis());
        return time.formatted("%Y-%m-%d-%H:%M:%S");
    }

    String Time::currentYear() {
        stl::Time time(stl::Time::currentTimeMillis());
        return std::to_string(time.getYear());
    }

    String Time::currentMonth() {
        stl::Time time(stl::Time::currentTimeMillis());
        return std::to_string(time.getMonth());
    }

    String Time::currentDayOfMonth() {
        stl::Time time(stl::Time::currentTimeMillis());
        return std::to_string(time.getDayOfMonth());
    }
#ifdef STL_ANDROID

    uint32 juce_millisecondsSinceStartup() noexcept
    {
        timespec t;
        clock_gettime (CLOCK_MONOTONIC, &t);

        return static_cast<uint32> (t.tv_sec) * 1000U + static_cast<uint32> (t.tv_nsec) / 1000000U;
    }

    int64 Time::getHighResolutionTicks() noexcept
    {
        timespec t;
        clock_gettime (CLOCK_MONOTONIC, &t);

        return (t.tv_sec * (int64) 1000000) + (t.tv_nsec / 1000);
    }

    int64 Time::getHighResolutionTicksPerSecond() noexcept
    {
        return 1000000;  // (microseconds)
    }

    double Time::getMillisecondCounterHiRes() noexcept
    {
        return (double) getHighResolutionTicks() * 0.001;
    }

    bool Time::setSystemTimeToThisTime() const
    {
        jassertfalse;
        return false;
    }

#endif

} // namespace stl
