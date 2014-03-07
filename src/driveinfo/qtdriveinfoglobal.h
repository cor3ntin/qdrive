#ifndef QTDRIVEINFOGLOBAL_H
#define QTDRIVEINFOGLOBAL_H

#include <QtCore/qglobal.h>

#ifndef Q_DRIVEINFO_EXPORT
#  ifndef QT_STATIC
#    if defined(QT_BUILD_DRIVEINFO_LIB)
#      define Q_DRIVEINFO_EXPORT Q_DECL_EXPORT
#    else
#      define Q_DRIVEINFO_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif
#ifndef Q_DRIVEINFO_EXPORT
#  define Q_DRIVEINFO_EXPORT
#endif

#ifdef Q_OS_LINUX

#define EINTR_LOOP(var, cmd)                    \
    do {                                        \
        var = cmd;                              \
    } while (var == -1 && errno == EINTR)

#endif

#ifdef Q_OS_MACX

#include <ApplicationServices/ApplicationServices.h>

template <typename T>
class Q_CORE_EXPORT QCFType
{
public:
    inline QCFType(const T &t = 0) : type(t) {}
    inline QCFType(const QCFType &helper) : type(helper.type) { if (type) CFRetain(type); }
    inline ~QCFType() { if (type) CFRelease(type); }
    inline operator T() { return type; }
    inline QCFType operator =(const QCFType &helper)
    {
    if (helper.type)
        CFRetain(helper.type);
    CFTypeRef type2 = type;
    type = helper.type;
    if (type2)
        CFRelease(type2);
    return *this;
    }
    inline T *operator&() { return &type; }
    static QCFType constructFromGet(const T &t)
    {
        CFRetain(t);
        return QCFType<T>(t);
    }
protected:
    T type;
};

class Q_CORE_EXPORT QCFString : public QCFType<CFStringRef>
{
public:
    inline QCFString(const QString &str) : QCFType<CFStringRef>(0), string(str) {}
    inline QCFString(const CFStringRef cfstr = 0) : QCFType<CFStringRef>(cfstr) {}
    inline QCFString(const QCFType<CFStringRef> &other) : QCFType<CFStringRef>(other) {}
    operator QString() const;
    operator CFStringRef() const;
    static QString toQString(CFStringRef cfstr);
    static CFStringRef toCFStringRef(const QString &str);
private:
    QString string;
};

#endif

#endif // QTDRIVEINFOGLOBAL_H
