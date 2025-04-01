// Last Change: 2025-04-01  Tuesday: 01:21:24 AM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "file_utils.h"
#include "logging.h"
#include "parser.h"

FileMapping *file_mappings = NULL; // Global array of file mappings
size_t file_mapping_count = 0; // Global count of file mappings

/*
   ===  FUNCTION  ======================================================================
           Name:  build_file_mappings
    Description:  Builds a global array of file mappings from the given resources.
                 Each resource is processed to extract its filename, original path,
                 relative path, and cousin status.
                 Cousins are files with the same filename but different directory structures.
   =====================================================================================
*/
void build_file_mappings(char **resources, size_t resource_count, const char *project_root) {
  // Allocate the mapping array
  file_mappings = malloc(resource_count * sizeof(FileMapping));

  if(!file_mappings) {
    perror("Failed to allocate memory for file mappings");
    return;
  }

  file_mapping_count = resource_count;

  // First pass: extract filenames and count occurrences
  for(size_t i = 0; i < resource_count; i++) {
    // Extract filename
    const char *filename = strrchr(resources[i], '/');
    filename = filename ? filename + 1 : resources[i];
    file_mappings[i].filename = strdup(filename);
    file_mappings[i].original_path = strdup(resources[i]);
    file_mappings[i].relative_path = NULL;
    file_mappings[i].is_cousin = 0;
  }

  // Second pass: identify cousins
  for(size_t i = 0; i < file_mapping_count; i++) {
    for(size_t j = 0; j < i; j++) {
      if(strcmp(file_mappings[i].filename, file_mappings[j].filename) == 0) {
        // Found a cousin!
        file_mappings[i].is_cousin = 1;
        file_mappings[j].is_cousin = 1;
      }
    }
  }

  // Third pass: build relative paths for cousins
  for(size_t i = 0; i < file_mapping_count; i++) {
    if(file_mappings[i].is_cousin) {
      // Extract directory path (without filename)
      char *path = strdup(file_mappings[i].original_path);
      char *last_slash = strrchr(path, '/');

      if(last_slash) {
        *last_slash = '\0'; // Truncate filename
      }

      char *candidate = NULL;
      bool found = false;

      while(!found) {
        // Start with the last directory component as candidate
        char *current_candidate_start = strrchr(path, '/');

        if(current_candidate_start) {
          candidate = strdup(current_candidate_start + 1);
        }

        else {
          candidate = strdup(path); // Full path if no slashes left
        }

        // Check uniqueness against all other cousins
        bool conflict = false;

        for(size_t j = 0; j < file_mapping_count; j++) {
          if(i != j && file_mappings[j].is_cousin) {
            const char *other_path = file_mappings[j].original_path;
            size_t candidate_len = strlen(candidate);
            size_t other_len = strlen(other_path);

            if(other_len >= candidate_len &&
               strcmp(other_path + other_len - candidate_len, candidate) == 0) {
              conflict = true;
              break;
            }
          }
        }

        if(!conflict) {
          file_mappings[i].relative_path = strdup(candidate);
          found = true;
        }

        else {
          // Move up one directory level
          char *prev_slash = strrchr(path, '/');

          if(!prev_slash) {
            // No more directories to move up; use full path
            file_mappings[i].relative_path = strdup(path);
            found = true;
            free(candidate);
            break;
          }

          *prev_slash = '\0'; // Truncate path
          free(candidate);
        }
      }

      free(path);
    }

    else {
      // For non-cousin files, set the relative path to the filename
      file_mappings[i].relative_path = strdup(file_mappings[i].filename);
    }
  }
}

/*
   ===  FUNCTION  ======================================================================
           Name:  get_destination_path
    Description:  Returns the destination path for a given source file.
                 If the file is a cousin, it uses the relative path.
                 Otherwise, it puts the file in the assets directory.
   =====================================================================================
*/
const char *get_destination_path(const char *source, const char *assets_dir) {
  static char result[4096];

  // Find the matching entry in the file_mappings
  for(size_t i = 0; i < file_mapping_count; i++) {
    if(strcmp(source, file_mappings[i].original_path) == 0) {
      if(file_mappings[i].is_cousin && file_mappings[i].relative_path) {
        // This is a cousin file - use the relative path
        snprintf(result, sizeof(result), "%s/%s/%s",
                 assets_dir, file_mappings[i].relative_path, file_mappings[i].filename);
      }

      else {
        // Regular file - just put in assets directory
        snprintf(result, sizeof(result), "%s/%s",
                 assets_dir, file_mappings[i].filename);
      }

      return result;
    }
  }

  // Not found in mappings - just use the filename
  const char *filename = strrchr(source, '/');
  filename = filename ? filename + 1 : source;
  snprintf(result, sizeof(result), "%s/%s", assets_dir, filename);
  return result;
}

/*
   ===  FUNCTION  ======================================================================
           Name:  concat_paths
    Description:  Concatenates two paths to form a new path
   =====================================================================================
*/
char *concat_paths(const char *path1, const char *path2) {
  if(!path1 || !path2) {
    printf("DEBUG: concat_paths received NULL pointer: path1=%p, path2=%p\n", (void *)path1, (void *)path2);
    return NULL; // Return NULL for invalid inputs
  }

  size_t len1 = strlen(path1);
  size_t len2 = strlen(path2);
  size_t total_length = len1 + len2 + 2; // +2 for '/' and '\0'
  char *result = malloc(total_length);

  if(!result) {
    return NULL; // Memory allocation failed
  }

  result[0] = '\0';
  strncat(result, path1, len1);

  // Append '/' if path1 does not already end with one
  if(len1 > 0 && path1[len1 - 1] != '/') {
    strncat(result, "/", 2);
  }

  strncat(result, path2, len2 + 1);
  return result;
}

/*
   ===  FUNCTION  ======================================================================
           Name:  detect_and_prepare_cousins
    Description:  Detects filenames with different directory structures and prepares
                 the necessary directory structures in the destination directory.
                 Unused function.
   =====================================================================================
*/
void detect_and_prepare_cousins(char **resources, size_t resource_count, const char *assets_dir, const char *project_root) {
  if(!resources || resource_count == 0) {
    return; // No resources to process
  }

  // Temporary array to store filenames and their directory structures
  char **filenames = malloc(resource_count * sizeof(char *));
  char **directories = malloc(resource_count * sizeof(char *));

  if(!filenames || !directories) {
    perror("Failed to allocate memory for filenames or directories");
    free_strings_array(filenames, resource_count);
    free_strings_array(directories, resource_count);
    return;
  }

  for(size_t i = 0; i < resource_count; ++i) {
    char *filename = strrchr(resources[i], '/');

    if(!filename) {
      filename = resources[i]; // No '/' found, use the entire path as the filename
    }

    else {
      filename++; // Move past the last '/'
    }

    // Extract the relative directory part of the path
    char *directory = strdup(filename != resources[i] ? resources[i] : ".");

    if(directory && strstr(directory, filename)) {
      *strrchr(directory, '/') = '\0'; // Null-terminate at the last slash
    }

    // Remove project_root prefix to get the relative directory
    if(project_root && strstr(directory, project_root)) {
      directory += strlen(project_root); // Start after project_root

      while(*directory == '/') {        // Skip any leading slashes
        directory++;
      }
    }

    // Check if the filename exists in multiple directories
    int duplicate_found = 0;

    for(size_t j = 0; j < i; ++j) {
      if(strcmp(filenames[j], filename) == 0 && strcmp(directories[j], directory) != 0) {
        duplicate_found = 1;
        break;
      }
    }

    if(duplicate_found) {
      // Construct the full destination path for the cousin file
      char *full_destination_path = concat_paths(assets_dir, directory);

      if(full_destination_path) {
        printf("DEBUG: Conflicting filename detected: %s. Creating directory: %s\n", filename, full_destination_path);

        if(!create_directory(full_destination_path)) {
          fprintf(stderr, "Error: Failed to create directory for cousin file: %s\n", full_destination_path);
        }

        free(full_destination_path);
      }
    }

    // Store the filename and directory for future checks
    filenames[i] = strdup(filename);
    directories[i] = strdup(directory);
    free(directory); // Free the temporary directory string
  }

  // Clean up temporary arrays
  free_strings_array(filenames, resource_count);
  free_strings_array(directories, resource_count);
}

/*
   ===  FUNCTION  ======================================================================
           Name:  create_directory
    Description:  Creates a directory
   =====================================================================================
*/
int create_directory(const char *path) {
  /*printf("DEBUG: fn create_directory, file_utils.c, received path: %s, returning: %d\n", path, mkdir(path, 0755) == 0 || errno == EEXIST);*/
  return mkdir(path, 0755) == 0 || errno == EEXIST;
}

/*
   ===  FUNCTION  ======================================================================
           Name:  copy_file_to_directory
    Description:  Copies a file to a specified directory, handling both absolute and
                 relative paths by converting relative paths to absolute in-memory.
   =====================================================================================
*/
void copy_file_to_directory(const char *source, const char *destination_dir, const char *project_root) {
  /*printf("DEBUG: fn copy_file_to_directory, file_utils.c, source=%s, destination_dir=%s, project_root=%s\n", source, destination_dir, project_root);*/

  // Check if source or destination_dir is NULL
  if(!source || !destination_dir) {
    fprintf(stderr, "Error: Null pointer for source or destination_dir.\n");
    return;
  }

  // Determine if the source path is absolute or relative
  char full_source_path[4096] = {0};

  if(source[0] == '/') {
    // Absolute path: Use as-is
    strncpy(full_source_path, source, sizeof(full_source_path) - 1);
  }

  else {
    // Relative path: Concatenate with project_root
    if(!project_root || strlen(project_root) == 0) {
      fprintf(stderr, "Error: Project root directory not provided for relative path: %s\n", source);
      return;
    }

    snprintf(full_source_path, sizeof(full_source_path), "%s/%s", project_root, source);
  }

  /* Debugging statement */
  /*printf("DEBUG: Full source path: %s\n", full_source_path);*/
  // Construct the destination path
  char *filename = strrchr(full_source_path, '/');

  if(!filename) {
    filename = full_source_path; // No '/' found, use the entire path as the filename
  }

  else {
    filename++; // Move past the last '/'
  }

  char destination[4096] = {0};
  snprintf(destination, sizeof(destination) + 1, "%s/%s", destination_dir, filename);
  /* Debugging statement */
  /*printf("DEBUG: Destination path: %s\n", destination);*/

  // Check if the destination file already exists
  if(access(destination, F_OK) == 0) {
    /*printf("DEBUG: Destination file already exists: %s. Skipping copy.\n", destination);*/
    return; // File already exists, skip copying
  }

  // Open the source file
  FILE *src = fopen(full_source_path, "rb");

  if(!src) {
    printf("\nSource File: %s\n\n", full_source_path);
    perror("Failed to open source file");
    return;
  }

  // Open the destination file
  FILE *dst = fopen(destination, "wb");

  if(!dst) {
    perror("Failed to open destination file");
    fclose(src);
    return;
  }

  // Copy the file contents
  const int buf_size = 4096;
  char buffer[buf_size];
  size_t bytes_read;

  while((bytes_read = fread(buffer, 1, buf_size, src)) > 0) {
    fwrite(buffer, 1, bytes_read, dst);
  }

  // Close files
  fclose(src);
  fclose(dst);
  printf("Copied file from %s to %s\n", full_source_path, destination);
}

/*
   ===  FUNCTION  ======================================================================
           Name:  copy_file_to_directory_with_context
    Description:  Copies a file to a destination directory, handling both absolute and
                 relative paths. If the source is relative, it uses the project root
                 to construct the full source path and the input file to construct
                 the full destination path.
   =====================================================================================
*/
void copy_file_to_directory_with_context(const char *source, const char *destination_dir, const char *project_root, const char *input_file) {
  if(!source || strlen(source) == 0) {
    return;
  }

  // Determine if the source path is absolute or relative
  char full_source_path[4096] = {0};

  if(source[0] == '/') {
    // Absolute path: Use as-is
    strncpy(full_source_path, source, sizeof(full_source_path) - 1);
  }

  else {
    // Relative path: Concatenate with project_root
    if(!project_root || strlen(project_root) == 0) {
      fprintf(stderr, "Error: Project root directory not provided for relative path: %s\n", source);
      return;
    }

    snprintf(full_source_path, sizeof(full_source_path), "%s/%s", project_root, source);
  }

  // Construct the destination path
  const char *destination = get_destination_path(source, destination_dir);

  if(!destination) {
    fprintf(stderr, "Error: Failed to determine destination path for source: %s\n", source);
    return;
  }

  // Check if the destination file already exists
  if(access(destination, F_OK) == 0) {
    return; // File already exists, skip copying
  }

  // Open the source file
  FILE *src = fopen(full_source_path, "rb");

  if(!src) {
    printf("\nSource File: %s\n\n", full_source_path);
    perror("Failed to open source file");
    return;
  }

  // Open the destination file
  FILE *dst = fopen(destination, "wb");

  if(!dst) {
    perror("Failed to open destination file");
    fclose(src);
    return;
  }

  // Copy the file contents
  const int buf_size = 4096;
  char buffer[buf_size];
  size_t bytes_read;

  while((bytes_read = fread(buffer, 1, buf_size, src)) > 0) {
    fwrite(buffer, 1, bytes_read, dst);
  }

  // Close files
  fclose(src);
  fclose(dst);
  printf("Copied file from %s to %s\n", full_source_path, destination);
}

/*
   ===  FUNCTION  ======================================================================
           Name:  str_replace
    Description:  Replaces all occurrences of 'search' with 'replace' in 'src'
   =====================================================================================
*/
char *str_replace(const char *src, const char *search, const char *replace) {
  if(!src || !search || !replace) {
    return NULL;
  }

  // Calculate lengths of input strings
  size_t search_len = strlen(search);

  if(search_len == 0) {
    return strdup(src);  // Avoid infinite loops when searching for empty string
  }

  size_t replace_len = strlen(replace);
  size_t src_len = strlen(src);
  // Count the number of replacements needed
  size_t count = 0;
  const char *temp = src;

  while((temp = strstr(temp, search)) != NULL) {
    count++;
    temp += search_len;
  }

  // Allocate memory for the result
  size_t result_len = src_len + count * (replace_len - search_len) + 1;
  char *result = malloc(result_len);

  if(!result) {
    return NULL;
  }

  // Perform the replacement
  char *ptr = result;
  temp = src;

  while(count--) {
    const char *pos = strstr(temp, search);
    size_t prefix_len = pos - temp;
    memcpy(ptr, temp, prefix_len);
    ptr += prefix_len;
    memcpy(ptr, replace, replace_len);
    ptr += replace_len;
    temp = pos + search_len;
  }

  strcpy(ptr, temp); // Copy the remaining part of the string
  return result;
}

/*
   ===  FUNCTION  ======================================================================
           Name:  str_replace_in_place
    Description:  Replaces all occurrences of 'search' with 'replace' in 'line'
   =====================================================================================
*/
void str_replace_in_place(char *line, const char *search, const char *replace) {
  char buffer[4 * BUFFER];
  strncpy(buffer, line, sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0';
  size_t search_len = strlen(search);
  size_t replace_len = strlen(replace);
  size_t src_len = strlen(buffer);
  size_t count = 0;
  const char *temp = buffer;

  while((temp = strstr(temp, search)) != NULL) {
    count++;
    temp += search_len;
  }

  size_t result_len = src_len + count * (replace_len - search_len) + 1;

  if(result_len > sizeof(buffer)) {
    perror("Buffer overflow in str_replace_in_place");
    return;
  }

  char *ptr = line;
  temp = buffer;

  while(count--) {
    const char *pos = strstr(temp, search);
    size_t prefix_len = pos - temp;
    memcpy(ptr, temp, prefix_len);
    ptr += prefix_len;
    memcpy(ptr, replace, replace_len);
    ptr += replace_len;
    temp = pos + search_len;
  }

  strcpy(ptr, temp); // Copy the remaining part of the string
}

/*
   ===  FUNCTION  ======================================================================
           Name:  process_resource_line
    Description:  Processes a resource line by extracting the original path and validating it.
    Resource lines are XML tags that contain video, audio, and image file paths.
   =====================================================================================
*/
void process_resource_line(char *line, const char *assets_dir, FILE *out) {
  char *start = strchr(line, '>') + 1;
  char *end = strrchr(line, '<');

  if(start && end && start < end) {
    size_t len = end - start;
    char original_path[4096] = {0};
    strncpy(original_path, start, len);
    original_path[len] = '\0';
    /* Debugging statement */
    /*printf("DEBUG: Original path: %s\n", original_path);*/

    // Validate original_path
    if(len == 0 || original_path[0] == '\0' || strcmp(original_path, "0") == 0) {
      if(strcmp(line, "<property name=\"resource\">0</property>") == 0) {
        fprintf(stderr, "Error: Invalid resource path in line: %s\n", line);
        return;
      }

      else {
        fputs(line, out); // Write the line as-is
        return;
      }
    }

    // Get the destination path using get_destination_path
    const char *destination = get_destination_path(original_path, assets_dir);

    if(!destination) {
      fprintf(stderr, "Error: Failed to determine destination path for source: %s\n", original_path);
      fputs(line, out); // Write the line as-is
      return;
    }

    // Extract the relative path from the destination path
    const char *assets_dir_end = strstr(destination, "assets/");

    if(!assets_dir_end) {
      fprintf(stderr, "Error: Incorrect destination path format: %s\n", destination);
      fputs(line, out); // Write the line as-is
      return;
    }

    // Skip "assets/" to get the relative path
    const char *relative_path = assets_dir_end + strlen("assets/");
    // Replace the original path with the new relative path in the line
    char new_path[4096] = {0};
    snprintf(new_path, sizeof(new_path), "assets/%s", relative_path);
    // Replace the original path with the new path in the line
    char *modified_line = str_replace(line, original_path, new_path);

    if(modified_line) {
      fputs(modified_line, out);
      free(modified_line);
    }

    else {
      perror("Failed to allocate memory for modified resource line");
      fputs(line, out); // Fallback: Write the original line
    }
  }

  else {
    // Copy lines with no valid resource path
    fputs(line, out);
  }
}

/*
   ===  FUNCTION  ======================================================================
           Name:  process_lut_line
    Description:  Processes a LUT line by extracting the original path and validating it.
    LUT lines are XML tags that contain LUT file paths.
   =====================================================================================
*/
void process_lut_line(char *line, const char *lut_dir, FILE *out, const char *proj_root) {
  char *start = strchr(line, '>') + 1;
  char *end = strrchr(line, '<');

  if(start && end && start < end) {
    size_t len = end - start;
    char original_path[4096] = {0};
    strncpy(original_path, start, len);
    original_path[len] = '\0';
    /* Debugging statement */
    /*printf("DEBUG: Original LUT path: %s\n", original_path);*/

    // Validate original_path
    if(len == 0 || original_path[0] == '\0') {
      fprintf(stderr, "Error: Invalid LUT path in line: %s\n", line);
      fputs(line, out); // Write the line as-is
      return;
    }

    // Extract just the filename from the path
    const char *filename = strrchr(original_path, '/');
    filename = filename ? filename + 1 : original_path;
    char new_path[4096] = {0};
    snprintf(new_path, sizeof(new_path) + BUFFER, "assets/LUT/%s", filename);
    // Copy the LUT file to the LUT directory
    copy_file_to_directory(original_path, lut_dir, proj_root);
    // Replace the original path with the new path in the line
    char *modified_line = str_replace(line, original_path, new_path);

    if(modified_line) {
      fputs(modified_line, out);
      free(modified_line);
    }

    else {
      perror("Failed to allocate memory for modified LUT line");
      fputs(line, out); // Fallback: Write the original line
    }
  }

  else {
    // Copy lines with no valid LUT path
    fputs(line, out);
  }
}

/*
   ===  FUNCTION  ======================================================================
           Name:  process_file_stabilizer_line
    Description:  Processes a file stabilizer line by extracting the original path and validating it.
    File stabilizer lines are XML tags that contain the stabilization data file paths.
   =====================================================================================
*/
void process_file_stabilizer_line(char *line, const char *stabilizer_presets_dir, FILE *out, const char *proj_root) {
  char *start = strchr(line, '>') + 1;
  char *end = strrchr(line, '<');

  if(start && end && start < end) {
    size_t len = end - start;
    char original_path[4096] = {0};
    strncpy(original_path, start, len);
    original_path[len] = '\0';
    /* Debugging statement */
    /*printf("DEBUG: Original stabilization_data path: %s\n", original_path);*/

    // Validate original_path
    if(len == 0 || original_path[0] == '\0') {
      fprintf(stderr, "Error: Invalid LUT path in line: %s\n", line);
      fputs(line, out); // Write the line as-is
      return;
    }

    // Extract just the filename from the path
    const char *filename = strrchr(original_path, '/');
    filename = filename ? filename + 1 : original_path;
    char new_path[4096] = {0};
    snprintf(new_path, sizeof(new_path) + BUFFER, "assets/stabilization_data/%s", filename);
    // Copy the LUT file to the LUT directory
    copy_file_to_directory(original_path, stabilizer_presets_dir, proj_root);
    // Replace the original path with the new path in the line
    char *modified_line = str_replace(line, original_path, new_path);

    if(modified_line) {
      fputs(modified_line, out);
      free(modified_line);
    }

    else {
      perror("Failed to allocate memory for modified file_stabilizer_line line");
      fputs(line, out); // Fallback: Write the original line
    }
  }

  else {
    // Copy lines with no valid LUT path
    fputs(line, out);
  }
}

/*
  copy_and_modify_project_file - Copies an MLT project file with modified resource paths
  --------------------------------------------------------------------------------------
  Initially written by Qwen 2.5 Turbo.
  Claude Sonnet 3.7. 2025.03.26. IST 01:02 PM.
  Rewritten several times by Qwen 2.5 Turbo.

  PURPOSE:
  This function reads through an MLT project file line by line, modifying and copying
  various resource files to ensure all project dependencies are collected in a centralized
  assets directory structure.

  WHAT IT DOES:
  1. Handles three types of resource references:
     a. Media files (via <property name="resource"> tags)
     b. LUT (Look-Up Table) files (via <property name="av.file"> tags)
     c. Stabilizer data files (via <property name="filename"> tags)

  2. For media files:
     - Extracts the original file path
     - Copies the file to the main assets directory
     - Updates the project file to use a relative "assets/filename" path

  3. For LUT files:
     - Extracts the original LUT file path
     - Copies the file to a special "assets/LUT/" subdirectory
     - Updates the project file to use a relative "assets/LUT/filename" path

  4. For stabilizer files:
     - Extracts the original stabilizer data file path
     - Copies the file to a special "assets/stabilization_data/" subdirectory
     - Updates the project file to use a relative "assets/stabilization_data/filename" path

  EXAMPLE TRANSFORMATIONS:
  Media file:
     Input:  <property name="resource">/full/path/to/video.webm</property>
     Output: <property name="resource">assets/video.webm</property>

  LUT file:
     Input:  <property name="av.file">/path/to/lut/color_grading.cube</property>
     Output: <property name="av.file">assets/LUT/color_grading.cube</property>

  Stabilizer file:
     Input:  <property name="filename">/path/to/stabilization/data.txt</property>
     Output: <property name="filename">assets/stabilization_data/data.txt</property>

  BENEFITS:
  - Makes project files more portable
  - Centralizes all project resources
  - Simplifies file path management
  - Preserves all project dependencies including stabilization data

  PARAMETERS:
  input       - Path to the original MLT project file
  output      - Path where the modified file should be saved
  assets_dir  - Base directory for collecting assets
  project_root - Root directory of the project for relative path resolution

  RETURNS:
  1 on success, 0 on failure
*/
int copy_and_modify_project_file(const char *input, const char *output, const char *assets_dir, const char *project_root) {
  /*printf("DEBUG: fn copy_and_modify_project_file, file_utils.c, received input: %s, output: %s, assets_dir: %s\n", input, output, assets_dir);*/
  FILE *in = fopen(input, "r");

  if(!in) {
    perror("Failed to open input project file");
    return 0;
  }

  FILE *out = fopen(output, "w");

  if(!out) {
    perror("Failed to open output project file");
    fclose(in);
    return 0;
  }

  char line[4 * BUFFER]; // Max line length
  char *lut_dir = concat_paths(assets_dir, "LUT");
  char *stabilizer_dir = concat_paths(assets_dir, "stabilization_data");

  if(!lut_dir || !stabilizer_dir) {
    perror("Failed to allocate memory for subdirectories");
    fclose(in);
    fclose(out);
    free(lut_dir);
    free(stabilizer_dir);
    return 0;
  }

  while(fgets(line, sizeof(line), in)) {
    if(strstr(line, "<property name=\"resource\">")) {
      process_resource_line(line, assets_dir, out);
    }

    else if(strstr(line, "<property name=\"av.file\">")) {
      process_lut_line(line, lut_dir, out, project_root);
    }

    else if(strstr(line, "<property name=\"filename\">")) {
      process_file_stabilizer_line(line, stabilizer_dir, out, project_root);
    }

    else {
      // Copy lines that don't need modification
      fputs(line, out);
    }
  }

  free(lut_dir);
  free(stabilizer_dir);
  fclose(in);
  fclose(out);
  return 1;
}

void free_file_mappings() {
  if(!file_mappings) {
    return;
  }

  for(size_t i = 0; i < file_mapping_count; i++) {
    free(file_mappings[i].filename);
    free(file_mappings[i].original_path);
    free(file_mappings[i].relative_path);
  }

  free(file_mappings);
  file_mapping_count = 0;
}
