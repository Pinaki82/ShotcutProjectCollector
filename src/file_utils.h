// Last Change: 2025-04-02  Wednesday: 01:32:24 PM
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#if !defined(__linux__)
  #error "These functions declared in file_utils.h will work only on Linux"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  BUFFER  2048

/*
   Struct to hold file mappings
   Written by Claude 3.7 Sonnet
*/
typedef struct {
  char *filename;       // Just the filename
  char *original_path;  // Full original path
  char *relative_path;  // Path to use in the output
  int is_cousin;        // Flag indicating if this is a cousin
} FileMapping;

extern FileMapping *file_mappings;
extern size_t file_mapping_count;
// --- Claude 3.7 Sonnet

void build_file_mappings(char **resources, size_t resource_count, const char *project_root);
char *concat_paths(const char *path1, const char *path2);
const char *get_destination_path(const char *source, const char *assets_dir);

void init_logging(const char *output_dir);
void log_message(const char *format, ...);
void detect_and_prepare_cousins(char **resources, size_t resource_count, const char *assets_dir, const char *project_root);
int create_directory(const char *path);
void copy_file_to_directory(const char *source, const char *destination_dir, const char *project_root);
void copy_file_to_directory_with_context(const char *source, const char *destination_dir, const char *project_root, const char *input_file);
char *str_replace(const char *src, const char *search, const char *replace);
void str_replace_in_place(char *line, const char *search, const char *replace);
void process_resource_line(char *line, const char *assets_dir, FILE *out);
void process_lut_line(char *line, const char *lut_dir, FILE *out, const char *proj_root);
void process_file_stabilizer_line(char *line, const char *stabilizer_presets_dir, FILE *out, const char *proj_root);
void process_alpha_transition_line(char *line, const char *alpha_transition_dir, FILE *out, const char *proj_root);
int copy_and_modify_project_file(const char *input, const char *output, const char *assets_dir, const char *project_root);
void free_file_mappings();

#endif // FILE_UTILS_H
