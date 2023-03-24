#define LINE_MAX 128

#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_ERR     3
#define LOG_WARNING 4
#define LOG_NOTICE  5
#define LOG_INFO    6
#define LOG_DEBUG   7

#define openlog(ident, logopt, facility) \
    fprintf(stderr, "*** openlog: %s %d %d\n", ident, logopt, facility)
    
#define syslog(priority, fmt, ...) \
    do { \
        fprintf(stderr, "*** syslog: [priority %d]: ", priority); \
        fprintf(stderr, fmt, __VA_ARGS__); \
    } while (0)
    
#define vsyslog(priority, fmt, ap) \
    do { \
        fprintf(stderr, "*** vsyslog: [priority %d]: ", priority); \
        vfprintf(stderr, fmt, ap); \
    } while (0)
    