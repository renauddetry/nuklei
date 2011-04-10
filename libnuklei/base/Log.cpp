// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/Log.h>
#include <nuklei/Common.h>

namespace nuklei {
    
  std::ostream *Log::out = &std::cerr;

  std::ostream *Log::outInstance = NULL;

  boost::mutex Log::mutex_;

  std::string Log::msgColor = NUKLEI_LBLUE;
  std::string Log::errorColor = NUKLEI_LRED;
  std::string Log::nocolor = NUKLEI_NOCOLOR;

  const std::string Log::TypeNames[] = { "NEVER", "FATAL", "ERROR", "WARN", "INFO", "LOG", "DEBUG" }; 

  void Log::setOutput(const std::string &filename)
  {
    delete outInstance;
    outInstance = NULL;
  
    outInstance = new std::ofstream(filename.c_str(), std::ios::out);
    out = outInstance;
    msgColor = "";
    errorColor = "";
    nocolor = "";
  }

  void Log::setOutput(std::ostream *stream)
  {
    delete outInstance;
    outInstance = NULL;
    out = stream;
  }

  void Log::log(const char* file,
                int line,
                unsigned level,
                const std::string &s)
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    LAST_OUTPUT_LINE_IS_PROGRESS = false;
    std::string color_start, color_stop;
    if (INTERACTIVE_SHELL)
    {
      if (level <= WARN) color_start = errorColor;
      else if (level <= LOG) color_start = msgColor;
      else color_start = nocolor;
      color_stop = nocolor;
    }
    stream() << color_start << file << ":" << line <<
      " " << color_stop << TypeNames[level] << " - " <<
      color_start << breakLines(s) <<
      color_stop << std::endl;
  }

  std::string Log::breakLines(const std::string &s)
  {
    std::string input = "\n" + s + "\n";
    std::string output, line, token;
  
    output.reserve(input.size());
  
    // Paragraph indent (ideally set to the width of the log annoucement)
    const std::string lineBreak = "\n  ";
  
    // Context indent (indent defined in the message)
    std::string ctxIndent;
  
    std::string indent;
  
    const unsigned int width = 80;
  
    for (std::string::const_iterator i = input.begin();
         i != input.end();)
    {
      if (*i == ' ')
      {
        if (token.size() == 0)
        {
        }
        else if (line.size() == 0)
        {
          line = token;
          token = "";
        }
        else if (line.size() + 1 + token.size() < width - indent.size())
        {
          line.append( " " + token );
          token = "";
        }
        else if (line.size() + 1 + token.size() == width - indent.size())
        {
          output.append( indent + line + " " + token);
          line = "";
          token = "";
        }
        else if (line.size() + 1 + token.size() > width - indent.size())
        {
          if (line.size() > 0)
          {
            output.append( indent + line );
            line = "";
          }
          if (token.size() >= width)
          {
            output.append( indent + token );
            token = "";
          }
          else
          {
            line = token;
            token = "";
          }
        }
      }
      else if (*i == '\n')
      {
        if (line.size() == 0 && token.size() != 0)
          output.append(indent + token);
        else if (line.size() + 1 + token.size() <= width - indent.size())
        {
          output.append( indent + line + " " + token );
        }
        else
        {
          output.append( indent + line + indent + token );
        }
        line = "";
        token = "";
        ctxIndent = "";
        for (i++; i != input.end(); i++)
        {
          if (*i == ' ') ctxIndent.append(" ");
          else
          {
            indent = lineBreak + ctxIndent;
            break;
          }
        }
        line = "";
        continue;
      }
      else
      {
        token += *i;
      }
      i++;
    }
  
    return output;
  }

}

