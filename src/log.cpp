//
// Logging data:
//

#include <time.h>
#include "log.h"
#include <iomanip.h>

/*********************************************************************
 *
 * Required variables:
 *
 *********************************************************************/

GlobalLog globalLog;

bool GlobalLog::OpenLogFile()
{
    if (!m_bOpened && m_strLogFile != NULL)
    {
        m_ofsLog.open(m_strLogFile, ios::out /*| ios::text*/ | ios::app /*append*/);
        if (!m_ofsLog)
        {
            m_strLogFile = NULL;
            m_bOpened = false;
        }
        else
        {
            m_bOpened = true;
            m_ofsLog.setbuf(NULL, 0);
        }
    }
    return m_bOpened;
}

//
// Operator():
//

// useful variable for returning an invalid ofstream to kill off any
// output to the logfile with wrong loglevel.
static ofstream ofsInvalid;

ostream& GlobalLog::operator()()
{
    // Ensure the current file is open:
    if (!OpenLogFile()) // if it can't be opened, shortcut everything:
        return ofsInvalid;

    time_t tNow = time(NULL);
    struct tm* ptm = localtime(&tNow);

    char cOldFill = m_ofsLog.fill('0');
    m_ofsLog << setw(4) << ptm->tm_year + 1900 << '-'
        << setw(2) << ptm->tm_mon  << '-'
        << setw(2) << ptm->tm_mday << ' '
        << setw(2) << ptm->tm_hour << ':'
        << setw(2) << ptm->tm_min  << ':'
        << setw(2) << ptm->tm_sec  << ' '
        << "FTE" << ' ';
    m_ofsLog.fill(cOldFill);
    return m_ofsLog;
}

FunctionLog::FunctionLog(GlobalLog& gl, char* funcName)
    : log(gl), func(funcName), myIndentLevel(++log.indent), indentChar('+')
{
    OutputLine() << "Entered function" << ENDLINE;
}

FunctionLog::~FunctionLog()
{
    indentChar = '+';
    OutputLine() << "Exited function" << ENDLINE;
    --log.indent;
}

ostream& FunctionLog::RC()
{
    indentChar = '!';
    return OutputLine() << "Returning rc = ";
}

ostream& FunctionLog::OutputIndent(ostream& os)
{
    for (int i = 1; i < myIndentLevel; ++i)
        os << '|';
    os << indentChar << ' ';
    indentChar = '|'; // reset it to |'s.
    return os;
}
