
#include <vector>
#include "logging/stllog.h"

namespace stl {
    class LogMessageData {
    public:
        LogMessageData(const char *file, unsigned int line, LogSeverity severity,
                       const char *tag)
                : file_(file),
                  line_number_(line),
                  severity_(severity),
                  tag_(tag) {}

        const char *GetFile() const {
            return file_;
        }

        unsigned int GetLineNumber() const {
            return line_number_;
        }

        LogSeverity GetSeverity() const {
            return severity_;
        }

        const char *GetTag() const { return tag_; }

        std::ostream &GetBuffer() {
            return buffer_;
        }

        std::string ToString() const {
            return buffer_.str();
        }

    private:
        std::ostringstream buffer_;
        const char *const file_;
        const unsigned int line_number_;
        const LogSeverity severity_;
        const char *const tag_;


        LogMessageData(const LogMessageData &) = delete;

        const LogMessageData &operator=(const LogMessageData &) = delete;
    };

    LogMessage::LogMessage(const char *file_name, unsigned int line, LogSeverity severity, const char *tag) :
            data_(new LogMessageData(file_name, line, severity, tag)) {
    }

    LogMessage::~LogMessage() {
        std::string msg = data_->ToString();


        stl::level::level_enum levelEnum;
        switch (data_->GetSeverity()) {
            case LogSeverity::DEBUG:
                stl::log({data_->GetFile(), (int) data_->GetLineNumber(), data_->GetTag()}, level::debug, msg);
                break;
            case LogSeverity::VERBOSE:
                stl::log({data_->GetFile(), (int) data_->GetLineNumber(), data_->GetTag()}, level::trace, msg);
                break;
            case LogSeverity::INFO:
                stl::log({data_->GetFile(), (int) data_->GetLineNumber(), data_->GetTag()}, level::info, msg);
                break;
            case LogSeverity::WARNING:
                stl::log({data_->GetFile(), (int) data_->GetLineNumber(), data_->GetTag()}, level::warn, msg);
                break;
            case LogSeverity::ERROR:
            case LogSeverity::FATAL_WITHOUT_ABORT:
                stl::log({data_->GetFile(), (int) data_->GetLineNumber(), data_->GetTag()}, level::err, msg);
                break;
            case LogSeverity::FATAL:
                stl::fatal({data_->GetFile(), (int) data_->GetLineNumber(), data_->GetTag()}, msg);
                break;
        }
        // Abort if necessary.
        if (data_->GetSeverity() == LogSeverity::FATAL) {
            abort();
        }
    }

    std::ostream &LogMessage::stream() {
        return data_->GetBuffer();
    }

    void StringAppendV(std::string *dst, const char *format, va_list ap) {
        // First try with a small fixed size buffer
        char space[1024] = {0};

        // It's possible for methods that use a va_list to invalidate
        // the data in it upon use.  The fix is to make a copy
        // of the structure before using it and use that copy instead.
        va_list backup_ap;
        va_copy(backup_ap, ap);
        int result = vsnprintf(space, sizeof(space), format, backup_ap);
        va_end(backup_ap);

        if (result < static_cast<int>(sizeof(space))) {
            if (result >= 0) {
                // Normal case -- everything fit.
                dst->append(space, result);
                return;
            }

            if (result < 0) {
                // Just an error.
                return;
            }
        }

        // Increase the buffer size to the size requested by vsnprintf,
        // plus one for the closing \0.
        int length = result + 1;
        char *buf = new char[length];

        // Restore the va_list before we use it again
        va_copy(backup_ap, ap);
        result = vsnprintf(buf, length, format, backup_ap);
        va_end(backup_ap);

        if (result >= 0 && result < length) {
            // It fit
            dst->append(buf, result);
        }
        delete[] buf;
    }

    void LogMessageFormat(const char *file, unsigned int line, LogSeverity severity, const char *tag, const char *fmt,
                          ...) {
        va_list ap;
        va_start(ap, fmt);
        std::string result;
        StringAppendV(&result, fmt, ap);
        va_end(ap);
        switch (severity) {
            case LogSeverity::DEBUG:
                stl::log({file, (int) line, tag}, level::debug, result);
                break;
            case LogSeverity::VERBOSE:
                stl::log({file, (int) line, tag}, level::trace, result);
                break;
            case LogSeverity::INFO:
                stl::log({file, (int) line, tag}, level::info, result);
                break;
            case LogSeverity::WARNING:
                stl::log({file, (int) line, tag}, level::warn, result);
                break;
            case LogSeverity::ERROR:
            case LogSeverity::FATAL_WITHOUT_ABORT:
                stl::log({file, (int) line, tag}, level::err, result);
                break;
            case LogSeverity::FATAL:
                stl::fatal({file, (int) line, tag}, result);
                break;
        }
        // Abort if necessary.
        if (severity == LogSeverity::FATAL) {
            abort();
        }
    }


}