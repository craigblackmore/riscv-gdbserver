// Main program.

// Copyright (C) 2009, 2013, 2017  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
// Contributor Ian Bolton <ian.bolton@embecosm.com>

// This file is part of the RISC-V GDB server

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <getopt.h>

// RISC-V headers in general and for each target
#include "ITarget.h"
#include "Picorv32.h"
#include "Ri5cy.h"

// Class headers
#include "GdbServer.h"
#include "TraceFlags.h"


using std::atoi;
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::strcmp;


//! Convenience function to output the usage to a specified stream.

//! @param[in] s  Output stream to use.

static void
usage (ostream & s)
{
  s << "Usage: riscv-gdbserver --core | -c <corename>" << endl
    << "                       [ --trace | -t <traceflags> ]" << endl
    << "                       <rsp-port>" << endl;

}	// usage ()


//! Main function

//! @see usage () for information on the parameters.  Instantiates the core
//! and GDB server.

//! @param[in] argc  Number of arguments.
//! @param[in] argv  Vector or arguments.
//! @return  The return code for the program.

int
main (int   argc,
      char *argv[] )
{
  // Argument handling.

  unsigned int  flags = 0;
  char         *coreName = nullptr;

  while (true) {
    int c;
    int longOptind = 0;
    static struct option longOptions[] = {
      {"core",  required_argument, nullptr,  'c' },
      {"help",  no_argument,       nullptr,  'h' },
      {"trace", required_argument, nullptr,  't' },
      {0,       0,                 0,  0 }
    };

    if ((c = getopt_long (argc, argv, "c:t:", longOptions, &longOptind)) == -1)
      break;

    switch (c) {
    case 'c':
      coreName = strdup (optarg);
      break;

    case 'h':
      usage (cout);
      return  EXIT_SUCCESS;

    case 't':
      // @todo We should allow more than just decimal values.

      flags = atoi (optarg);
      break;

    case '?':
    case ':':
      usage (cerr);
      return  EXIT_FAILURE;

    default:
      cerr << "ERROR: getopt_long returned character code " << c << endl;
    }
  }

  // 1 positional arg

  if ((argc - optind) != 1)
    {
      usage (cerr);
      return  EXIT_FAILURE;
    }

  int   port = atoi (argv[optind]);

  // Trace flags for the server

  TraceFlags * traceFlags = new TraceFlags (flags);

  // The RISC-V model

  ITarget * cpu;

  if (0 == strcasecmp ("PicoRV32", coreName))
    cpu = new Picorv32 ();
   else if (0 == strcasecmp ("RI5CY", coreName))
     cpu = new Ri5cy ();
  else
    {
      cerr << "ERROR: Unrecognized core: " << coreName << ": exiting" << endl;
      return  EXIT_FAILURE;
    }

  // The RSP server

  GdbServer *gdbServer = new GdbServer (port, cpu, traceFlags);

  // Run the GDB server. If we return, then we have hit some sort of problem.

  gdbServer->rspServer ();

  // Free memory

  delete  gdbServer;
  delete  traceFlags;
  free (coreName);

  return EXIT_FAILURE;			// If we return it's a failure!

}	/* sc_main() */


// Local Variables:
// mode: C++
// c-file-style: "gnu"
// End:
