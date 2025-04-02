// Last Change: 2025-04-02  Wednesday: 12:29:27 PM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "logging.h"
#include "file_utils.h"

// Global log file pointer
FILE *log_file = NULL;

/*
   ===  FUNCTION  ======================================================================
           Name:  init_logging
    Description:  Initializes logging by opening a file for writing
    Written by Qwen 2.5 Turbo (https://chat.qwen.ai/)
   =====================================================================================
*/
void init_logging(const char *output_dir) {
  char *log_path = concat_paths(output_dir, "project_collector.log");

  if(!log_path) {
    perror("Failed to allocate memory for log file path");
    return;
  }

  log_file = fopen(log_path, "w");

  if(!log_file) {
    perror("Failed to open log file");
    free(log_path);
    return;
  }

  printf("Logging initialized: %s\n", log_path);
  free(log_path);
}

/*
   ===  FUNCTION  ======================================================================
           Name:  log_message
    Description:  Writes a message to the log file
    Written by Qwen 2.5 Turbo (https://chat.qwen.ai/)
   =====================================================================================
*/
void log_message(const char *format, ...) {
  if(log_file) {
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    // Optionally, flush the log file after each message
    fflush(log_file);
  }
}
