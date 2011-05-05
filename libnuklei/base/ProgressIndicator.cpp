// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

/**
 * @file
 */

#include <iostream>
#include <string>
#include <unistd.h>

#include <nuklei/ProgressIndicator.h>
#include <nuklei/Common.h>

namespace nuklei
{
  
  
  const int ProgressIndicator::spanStartDef = 0,
    ProgressIndicator::spanEndDef = 100;
  const std::string ProgressIndicator::messageDef = "Progress: ";
  const std::string ProgressIndicator::backspaceSequence = "\033[A";

  
  /**
   * @brief Constructs a PI to go from spanStartDef to spanEndDef.
   */
  ProgressIndicator::ProgressIndicator()
  {
    initialize(spanStartDef, spanEndDef, messageDef, 0);
  }
  
  /**
   * @brief Constructs a PI which will scale [spanStartDef spanEnd]
   * to [0 100].
   */
  ProgressIndicator::ProgressIndicator(const int spanEnd,
                                       const std::string &message,
                                       const unsigned minLogLevel)
  {
    initialize(spanStartDef, spanEnd, message, minLogLevel);
  }
  
  /**
   * @brief Constructs a PI which will scale [spanStart spanEnd]
   * to [0 100].
   */
  ProgressIndicator::ProgressIndicator(const int spanStart,
                                       const int spanEnd,
                                       const std::string &message,
                                       const unsigned minLogLevel)
  {
    initialize(spanStart, spanEnd, message, minLogLevel);
  }
  
  ProgressIndicator::~ProgressIndicator()
  {
    if (state_ == running)
      forceEnd();
  }
  
  void ProgressIndicator::initialize(const int spanStart,
                                     const int spanEnd,
                                     const std::string &message,
                                     const unsigned minLogLevel)
  {
    minLogLevel_ = minLogLevel;
  
    int shellIsInteractive, shellTerminalFileNumber;
    
    current_ = 0;
    message_ = message;
    state_ = ready;
    verbose_ = false;
    
    spanStart_ = spanStart;
    spanEnd_ = spanEnd;
    spanPos_ = spanStart_;
    spanLength_ = spanEnd_ - spanStart_;
    if (spanLength_ < 0)
      std::cerr << "Warning: `spanStart > spanEnd', reset indicator with "
        "valid values to proceed."
                << std::endl;
    
    
    shellTerminalFileNumber = STDIN_FILENO;
    shellIsInteractive = isatty(shellTerminalFileNumber);
    
    if (shellIsInteractive &&
        getenv("TERM") != NULL &&
        std::string(getenv("TERM")) != "dumb")
    {
      colorStart_ = NUKLEI_DBLACK;
      colorEnd_ = NUKLEI_NOCOLOR;
    }

    if ( !shellIsInteractive || !ENABLE_CONSOLE_BACKSPACE )
    {
      backspace_ = false;
      scale_ = 10;
    }
    else
    {
      backspace_ = true;
      scale_ = 100;
    }
        
    if (verbose_)
      std::cout << "isatty: " << shellIsInteractive << std::endl
                << "spanLength: " << spanLength_ << " "
                << spanStart_ << "->" << spanEnd_ << std::endl;
    
    setValue(spanStart_);
  }  
  
  void ProgressIndicator::setMinLogLevel(const unsigned l)
  { minLogLevel_ = l; }
  unsigned ProgressIndicator::getMinLogLevel() const
  { return minLogLevel_; }

  void ProgressIndicator::setBackspace(const bool backspace)
  {
    backspace_ = backspace;
  }
  
  /**
   * Use inside the loop.
   * @param value the progress value between spanStart
   *              and spanEnd.
   */
  void ProgressIndicator::setValue(const int value)
  {
    if (state_ == finished) return;
    if (spanLength_ == 0) return;
    
    if (state_ == ready)
    {
      if (LOG_LEVEL >= minLogLevel_) std::cout << colorStart_ << message_ << "[   0% ]" << colorEnd_ << std::endl;
      state_ = running;
      if (LOG_LEVEL >= minLogLevel_) LAST_OUTPUT_LINE_IS_PROGRESS = true;
    }    
    spanPos_ = value;
    int count = (value - spanStart_) * scale_ / spanLength_;
    if (count > current_)
    {
      current_ = count;
      if (LOG_LEVEL >= minLogLevel_)
      {
        if (backspace_)
        {
          if (LAST_OUTPUT_LINE_IS_PROGRESS)
            std::cout << backspaceSequence;
          std::cout << colorStart_ << message_ << "[ ";
          
          int width = std::cout.width();
          std::cout.width(3);
          std::cout << int(current_ * 100 / scale_);
          std::cout.width(width);
          
          std::cout << "% ]" << colorEnd_ << std::endl;
          LAST_OUTPUT_LINE_IS_PROGRESS = true;
        }
        else
        {
          std::cout << colorStart_ << message_ << "[ ";
          int width = std::cout.width();
          std::cout.width(3);
          std::cout << int(current_ * 100 / scale_);
          std::cout.width(width);
          std::cout << "% ]" << colorEnd_ << std::endl;
          LAST_OUTPUT_LINE_IS_PROGRESS = true;
        }
      }
      if (current_ == scale_)
      {
        state_ = finished;
        //if (backspace_)
        //  std::cout << std::endl;
      }
    }
  }
  
  /**
   * Use inside the loop.
   * @param value the progress value between spanStart
   *              and spanEnd.
   */
  void ProgressIndicator::inc(const int value)
  {
    setValue(spanPos_+value);
  }
  
  const std::string& ProgressIndicator::getMessage() const
  {
    return message_;
  }
  
  void ProgressIndicator::setMessage(const std::string& message)
  {
    message_ = message;
  }
  
  /**
   * Gets the PI ready to be used again.
   * This is not a reset, since spanStart, spanEnd etc. are not
   * reset.
   */
  void ProgressIndicator::rewind()
  {
    current_ = 0;
    spanPos_ = 0;
    state_ = ready;
  }
  
  /**
   * Forces the display of 100% and a new line.
   */
  void ProgressIndicator::forceEnd()
  {
    setValue(spanEnd_);
  }
   
  
  int ProgressIndicator::main(int argc, char ** argv)
  {
    if (argc < 3)
    {
      std::cout << "no no no, test 0 100" << std::endl;
      return 1;
    }
    
    int a = strtol(argv[1], NULL, 10);
    int b = strtol(argv[2], NULL, 10);
    
    ProgressIndicator pi(a, b);
    
    for (int i = a; i <= b; i++)
    {
      pi.setValue(i);
      for (unsigned int j = 0; j < 2000000; j++);
    }
    
    pi.rewind();
    std::string message = "Second use: ";
    pi.setMessage(message);
    
    for (int i = a; i <= b + 50 - a; i++)
    {
      pi.setValue(i);
      for (unsigned int j = 0; j < 7000000; j++);
    }
    
    pi.forceEnd(); // since we know the loop only goes to 150
    
    return 0;
  }
  
}

