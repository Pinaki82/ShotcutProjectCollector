/*
  Enhancing Logging for Production Use

  If you'd like to enhance logging, here is the option:
  Write Logs to a File.

  Instead of printing debug messages to stdout, redirect them to a log file.
  This avoids cluttering the terminal and provides a persistent record of the program's behavior.
*/

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

void init_logging(const char *output_dir);
void log_message(const char *format, ...);

/*

  Then, replace all printf("DEBUG: ...") statements with log_message() calls. For example:

  log_message("DEBUG: Original path: %s\n", original_path);
  log_message("DEBUG: Relative path constructed: assets/%s\n", filename);

  Finally, ensure the log file is closed during cleanup:

  if (log_file) {
    fclose(log_file);
  }

*/

#endif // LOGGING_H
