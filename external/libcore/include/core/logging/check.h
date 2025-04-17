#ifndef MANXI_CORE_CHECK_H
#define MANXI_CORE_CHECK_H
#include <sstream>
#include <istream>
#include <memory>
#include "../stl_macro.h"
namespace stl {
#ifndef LIKELY
# define LIKELY(x) __builtin_expect(!!(x), 1)
#endif
#ifndef UNLIKELY
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

    template<typename LHS, typename RHS>
    struct EagerEvaluator {
        constexpr EagerEvaluator(LHS l, RHS r) : lhs(l), rhs(r) {
        }

        LHS lhs;
        RHS rhs;
    };
#ifdef __APPLE__
    inline std::ostream& operator<<(std::ostream& os, std::nullptr_t) {
        return os << "nullptr";
    }
    template<typename LHS, typename RHS>
    constexpr EagerEvaluator<LHS, RHS> MakeEagerEvaluator(LHS lhs, RHS rhs) {
        return EagerEvaluator<LHS, RHS>(lhs, rhs);
    }

#else
    template<typename LHS, typename RHS>
    constexpr EagerEvaluator<LHS, RHS> MakeEagerEvaluator(LHS lhs, RHS rhs) {
        return EagerEvaluator<LHS, RHS>(lhs, rhs);
    }
#endif



#define EAGER_PTR_EVALUATOR(T1, T2)               \
  template <>                                     \
  struct EagerEvaluator<T1, T2> {                 \
    EagerEvaluator(T1 l, T2 r)                    \
        : lhs(reinterpret_cast<const void*>(l)),  \
          rhs(reinterpret_cast<const void*>(r)) { \
    }                                             \
    const void* lhs;                              \
    const void* rhs;                              \
  }

    EAGER_PTR_EVALUATOR(const char*, const char*);

    EAGER_PTR_EVALUATOR(const char*, char*);

    EAGER_PTR_EVALUATOR(char*, const char*);

    EAGER_PTR_EVALUATOR(char*, char*);

    EAGER_PTR_EVALUATOR(const unsigned char*, const unsigned char*);

    EAGER_PTR_EVALUATOR(const unsigned char*, unsigned char*);

    EAGER_PTR_EVALUATOR(unsigned char*, const unsigned char*);

    EAGER_PTR_EVALUATOR(unsigned char*, unsigned char*);

    EAGER_PTR_EVALUATOR(const signed char*, const signed char*);

    EAGER_PTR_EVALUATOR(const signed char*, signed char*);

    EAGER_PTR_EVALUATOR(signed char*, const signed char*);

    EAGER_PTR_EVALUATOR(signed char*, signed char*);

#define ABORT_AFTER_LOG_EXPR_IF(c, x) (x)
#define ABORT_AFTER_LOG_FATAL_EXPR(x) ABORT_AFTER_LOG_EXPR_IF(true, x)


#define CHECK(x)                                                                                    \
  LIKELY((x)) || ABORT_AFTER_LOG_FATAL_EXPR(false) ||                                               \
      ::stl::LogMessage(__FILE__, __LINE__,::stl::FATAL, __FUNCTION__)                              \
              .stream()                                                                             \
          << "Check failed: " #x << " "


#define CHECK_OP(LHS, RHS, OP)                                                                      \
  for (auto _values = ::stl::MakeEagerEvaluator(LHS, RHS);                                          \
       UNLIKELY(!(_values.lhs OP _values.rhs));                                                     \
       /* empty */)                                                                                 \
  ::stl::LogMessage(__FILE__, __LINE__,::stl::FATAL,__FUNCTION__)                                   \
          .stream()                                                                                 \
      << "Check failed: " << #LHS << " " << #OP << " " << #RHS << " (left_value=" << _values.lhs    \
      << ", right_value=" << _values.rhs << ") "                                                                                \


#define CHECK_MSG(x, y)                                                                             \
  LIKELY((x)) || ABORT_AFTER_LOG_FATAL_EXPR(false) ||                                               \
      stl::fatal(stl::source_loc(__FILE__, __LINE__, __FUNCTION__),"Check failed: " #x","#y)        \


#if defined(__clang__) || defined(__GNUC__)
// Helper macro for declaring functions as having similar signature to printf.
// This allows the compiler to catch format errors at compile-time.
#define LOGGER_PRINTF_LIKE(fmtarg, firstvararg) __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#define LOGGER_FORMAT_STRING_TYPE const char*
#elif defined(_MSC_VER)
#define LOGGER_PRINTF_LIKE(fmtarg, firstvararg)
#define LOGGER_FORMAT_STRING_TYPE _In_z_ _Printf_format_string_ const char*
#else
#define LOGGER_PRINTF_LIKE(fmtarg, firstvararg)
#define LOGGER_FORMAT_STRING_TYPE const char*
#endif

#ifdef WIN32
#undef ERROR
#endif


#define CHECK_EQ(x, y) CHECK_OP(x, y, == )
#define CHECK_NE(x, y) CHECK_OP(x, y, != )
#define CHECK_LE(x, y) CHECK_OP(x, y, <= )
#define CHECK_LT(x, y) CHECK_OP(x, y, < )
#define CHECK_GE(x, y) CHECK_OP(x, y, >= )
#define CHECK_GT(x, y) CHECK_OP(x, y, > )


    enum LogSeverity {
        VERBOSE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL_WITHOUT_ABORT,
        FATAL,
    };

#define SEVERITY_LAMBDA(severity) ([&]() {    \
  using ::stl::VERBOSE;             \
  using ::stl::DEBUG;               \
  using ::stl::INFO;                \
  using ::stl::WARNING;             \
  using ::stl::ERROR;               \
  using ::stl::FATAL_WITHOUT_ABORT; \
  using ::stl::FATAL;               \
  return (severity); }())

    class  LogMessageData;

    class STL_EXPORT LogMessage {
    public:
        LogMessage(const char *file_name, unsigned int line, LogSeverity severity, const char *tag);

        ~LogMessage();

        // Returns the stream associated with the message, the LogMessage performs
        // output when it goes out of scope.
        std::ostream &stream();

    private:
        const std::unique_ptr<LogMessageData> data_;

        LogMessage(const LogMessage &) = delete;

        void operator=(const LogMessage &) = delete;
    };

    STL_EXPORT  void LogMessageFormat(const char *file, unsigned int line, LogSeverity severity, const char *tag, const char *fmt,
                          ...)LOGGER_PRINTF_LIKE(5, 6);

#define LOG_STREAM_TO(severity)                                                         \
  ::stl::LogMessage(__FILE__, __LINE__,SEVERITY_LAMBDA(severity),__FUNCTION__)     \
      .stream()

#define LOG_TO(severity) LOG_STREAM_TO( severity)
#define UNIMPLEMENTED(severity) LOG_TO(severity)


#ifndef LOGV
#define LOGV(...) LogMessageFormat(__FILE__, __LINE__,::stl::VERBOSE ,NULL,__VA_ARGS__)
#endif


#ifndef LOGD
#define LOGD(...) LogMessageFormat(__FILE__, __LINE__,::stl::DEBUG ,NULL,__VA_ARGS__)
#endif


#ifndef LOGI
#define LOGI(...) LogMessageFormat(__FILE__, __LINE__,::stl::INFO ,NULL,__VA_ARGS__)
#endif


#ifndef LOGW
#define LOGW(...) LogMessageFormat(__FILE__, __LINE__,::stl::WARNING ,NULL,__VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(...)  LogMessageFormat(__FILE__, __LINE__,::stl::ERROR ,NULL,__VA_ARGS__)
#endif

#ifndef LOGF
#define LOGF(...) LogMessageFormat(__FILE__, __LINE__,::stl::FATAL ,NULL,__VA_ARGS__)
#endif


#define DCHECK(x) \
   CHECK(x)
#define DCHECK_EQ(x, y) \
   CHECK_EQ(x, y)
#define DCHECK_NE(x, y) \
   CHECK_NE(x, y)
#define DCHECK_LE(x, y) \
   CHECK_LE(x, y)
#define DCHECK_LT(x, y) \
   CHECK_LT(x, y)
#define DCHECK_GE(x, y) \
   CHECK_GE(x, y)
#define DCHECK_GT(x, y) \
   CHECK_GT(x, y)
#define DCHECK_STREQ(s1, s2) \
   CHECK_STREQ(s1, s2)
#define DCHECK_STRNE(s1, s2) \
   CHECK_STRNE(s1, s2)

}
#endif //MANXI_CORE_CHECK_H