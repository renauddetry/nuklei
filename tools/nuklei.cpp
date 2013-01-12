// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <nuklei/Common.h>
#include <nuklei/Log.h>
#include "util/util.h"

int help(int argc, char ** argv);

struct CommandList
{
  const char* command;
  const char* help;
  int (*call) (int, char**);
};

const CommandList commands[] = {
  {
    command: "help",
    help: NULL,
    call: &help
  },
  {
    command: "convert",
    help: "Transformation and concatenation.",
    call: &convert
  },
  {
    command: "conv",
    help: "Shortcut for convert.",
    call: &convert
  },
  {
    command: "info",
    help: "Information on files.",
    call: &info
  },
  {
    command: "cat",
    help: NULL,
    call: &concatenate
  },
  {
    command: "kde",
    help: "",
    call: &kde
  },
  {
    command: "is",
    help: "",
    call: &importance_sampling
  },
  {
    command: "resample",
    help: "",
    call: &resample
  },
  {
    command: "homogeneous_subset",
    help: "",
    call: &homogeneous_subset
  },
  {
    command: "eval",
    help: "",
    call: &evaluate
  },
  {
    command: "invert_transfo",
    help: "",
    call: &invert_transfo
  },
  {
    command: "partial_view",
    help: "",
    call: &partial_view
  },
  {
    command: "create_mesh",
    help: "",
    call: &create_mesh
  },
  {
    command: "test",
    help: "",
    call: &test
  },
  {
    command: NULL,
    help: NULL,
    call: NULL
  }
};


int help(int argc, char ** argv)
{
  std::cout << "Usage: " << argv[0] << " FUNCTION [ARGS ...]" << std::endl;
  std::cout << "\nAvailable commands:\n";
  for (int i = 0; commands[i].command != NULL; i++)
  {
    std::cout << "  " << commands[i].command;
    if (commands[i].help)
    {
      std::cout << nuklei::Log::breakLines(std::string("  ") + commands[i].help);
    }
    std::cout << std::endl;
  }
  return EXIT_SUCCESS;
}


int main(int argc, char ** argv)
{
  try {
    if (argc < 2)
    {
      std::cerr << "Type '" << argv[0] << " help' for usage." << std::endl;
      return EXIT_FAILURE;
    }

    char* subargv[argc-1];
    for (int i = 1; i < argc; ++i)
      subargv[i-1] = argv[i];

    for (int i = 0; commands[i].command != NULL; i++)
    {
      if (std::string(argv[1]) == commands[i].command)
        return commands[i].call(argc-1, subargv);
    }

    std::cerr << "Unknown command " << argv[1] << "." <<
      " Type '" << argv[0] << " help' for usage." << std::endl;

    return EXIT_FAILURE;
  }
  catch (std::exception &e) {
    std::cerr << "Exception caught: ";
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Caught unknown exception." << std::endl;
    return EXIT_FAILURE;
  }

}

