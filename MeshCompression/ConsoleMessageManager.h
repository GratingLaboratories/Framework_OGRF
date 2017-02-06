#pragma once
#include <QString>
#include <string>

#define TRIVIAL_MSG     0x01
#define INFO_MSG        0x02
#define DATA_MSG        0x04
#define WARNING_MSG     0x08
#define ERROR_MSG       0x10
#define DEFAULT_MAG     0x20

class ConsoleMessageManager
{
public:
    explicit ConsoleMessageManager(std::ostream &o) : out(o)
    {
        // default setting;
        msg_mask = ERROR_MSG | WARNING_MSG | DATA_MSG | INFO_MSG | DEFAULT_MAG;
    }
    void enable(unsigned msg_code) { msg_mask |= msg_code; }
    void silent(unsigned msg_code) { msg_mask &= ~msg_code; }
    void log(const QString &s, unsigned msg_code = DEFAULT_MAG) const
    {
        if (msg_code & msg_mask)
        {
            out << s.toStdString() << std::endl;
        }
    }
    void log(const std::string &s, unsigned msg_code = DEFAULT_MAG) const
    {
        if (msg_code & msg_mask)
        {
            out << s << std::endl;
        }
    }
    void log(const char *s, unsigned msg_code = DEFAULT_MAG) const
    {
        if (msg_code & msg_mask)
        {
            out << s << std::endl;
        }
    }
    ~ConsoleMessageManager() {  }

private:
    std::ostream &out;
    unsigned      msg_mask;
};

