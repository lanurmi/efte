//
// General Logging... IN A CLASS!  :-)
//

/**

Class-based, OO-based logging

This is intended to be used as a trace utility in C++, if you follow the
following conventions.  Note that all macros are intended to work like
function calls - so you still need the terminating semicolon.

At the top of each function, you must use STARTFUNC(x).  The parameter to
this macro is the name of the function.  For example, STARTFUNC("main").

At the end of each function, or wherever you return from, use one of:

ENDFUNCRC - trace the return code
ENDFUNCRC_SAFE - same as above, but can be used when returning the
                 value of something with side effects
ENDFUNCAS - trace the return code, but pretend it's a different type
ENDFUNCAS_SAFE - trace the return code of something with side effects,
                 as if it were another type.

Finally, to log trace points throughout your code, use the LOG() macro
as if it were an ostream.  To terminate each line, do not use endl, but
use the ENDLINE macro.  Yes, currently it's the same thing, but I'm
reserving the right to change that in the future.

For example:

int main()
{
    STARTFUNC("main");

    LOG << "About to call foo" << ENDLINE;
    SomeObjectType baz;
    foo(baz);

    ENDFUNCRC(0);
    // no return - the macro does this for us.
}

void foo(SomeObjectType bar)
{
    STARTFUNC("foo")
    // assumes bar has some meaningful way to be put into an ostream:
    LOG << "bar = " << bar << ENDLINE;
    // as void, no need to endfunc.
}

ENDFUNCRC_SAFE is used such as:

ENDFUNCRC_SAFE(foo++); // side effect only happens once.
// was: return foo++

The AS versions are only used to log as a different type than it currently
is.  For example, to log a HANDLE as if it were an unsigned long:

HANDLE foo;
ENDFUNCAS(unsigned long, foo);
// was: return foo

Finally, ENDFUNCAS_SAFE:

ENDFUNCAS_SAFE(HANDLE, unsigned long, GetNextHandle());
// was: return GetNextHandle()

*/


#ifndef __LOGGING_HPP
#define __LOGGING_HPP

#include <fstream.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define bool  int
#define true  1
#define false 0

/**
 * GlobalLog handles the actual logging.
 */
class GlobalLog
{
    friend class FunctionLog;
private:
    char const* m_strLogFile;
    ofstream    m_ofsLog;

    bool        m_bOpened;

    int         indent;

    ostream&    operator()();

public:
    GlobalLog() : m_strLogFile(NULL), m_bOpened(false) {}
    GlobalLog(char const* strLogFile) : m_strLogFile(strdup(strLogFile)), m_bOpened(false) {}

    virtual ~GlobalLog() {free((void*)m_strLogFile);}

    void SetLogFile(char const* strNewLogFile)
    {
        if (m_strLogFile == NULL ||
            strNewLogFile == NULL ||
            strcmp(m_strLogFile,strNewLogFile) != 0)
        {
            free((void*)m_strLogFile);
            m_strLogFile = strNewLogFile == NULL ? NULL : strdup(strNewLogFile);
            m_bOpened    = false;
        }
    }

    operator bool() { return !m_ofsLog.fail(); }

protected:
    bool OpenLogFile(); // actually open it
};

/*template <class T>
ostream& operator<<(Log& log, T const& t)
{ return log() << t; }*/

extern GlobalLog globalLog;

/**
 * FunctionLog is the local object that handles logging inside a function.
 * All work is put through here.
 */
class FunctionLog
{
private:
    GlobalLog&  log;
    char const* func;
    int         myIndentLevel;
    char        indentChar;
public:
    // Enter:
    FunctionLog(GlobalLog& gl, char* funcName);

    // Exit:
    ~FunctionLog();

    // RC?
    ostream& RC();

    // output line.
    ostream& OutputLine()
    { return OutputIndent(log()) << '[' << func << "] "; }

private:
    ostream& OutputIndent(ostream& os);
};

template <class T>
ostream& operator<<(FunctionLog& fl, T const& t)
{ return fl.OutputLine() << t; }

#define LOG functionLog__obj
#define ENDLINE endl

#define STARTFUNC(func) FunctionLog LOG(globalLog, func)
#define ENDFUNCRC(rc) do { LOG.RC() << (rc) << ENDLINE; return (rc); } while (0)
#define ENDFUNCRC_SAFE(type,rc) do { type LOG__RC = (rc); LOG.RC() << LOG__RC << ENDLINE; return LOG__RC; } while (0)
#define ENDFUNCAS(type,rc) do { LOG.RC() << (type)(rc) << ENDLINE; return (rc); } while (0)
#define ENDFUNCAS_SAFE(logtype,rctype,rc) do { rctype LOG__RC = (rc); LOG.RC() << (logtype)LOG__RC << ENDLINE; return LOG__RC; } while (0)
#define BOOLYESNO(x) ((x) ? "YES" : "NO")

#endif // __LOGGING_HPP
