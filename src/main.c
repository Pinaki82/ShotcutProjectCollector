// Last Change: 2025-04-01  Tuesday: 01:12:55 AM
/*
  Shotcut Project Collector

  ------------------------

  A utility program that processes MLT (Shotcut) project files to collect and organise
  all project dependencies into a centralised assets directory.

  Usage:
  ./shotcut_project_collector '<input_mlt_file>' '<output_directory>'

  Functionality:

  1. Reads an MLT project file

  2. Identifies and copies all referenced media files, LUT files, and stabiliser data

  3. Creates a modified project file with updated relative paths to the collected assets

  4. Organises assets into appropriate subdirectories:

  - Media files in the main assets directory
  - LUT files in "assets/LUT/"
  - Stabiliser data in "assets/stabilisation_data/"

  Benefits:


  - Creates a portable project file with all necessary dependencies
  - Organises assets in a clear, hierarchical structure
  - Simplifies project sharing and relocation
*/

/*
  Development History

  ------------------

  This program was developed through a collaborative effort involving multiple AI systems:

  - Initial development and core functionality by Qwen 2.5 Turbo

  - Claude Sonnet 3.7 provided significant enhancements and improvements

  - Codium Base provided ongoing support and assistance

  - Development was carried out using both GVim (Tulu-C-IDE) and Windsurf IDEs

  While AI assistance played a significant role, this is not a purely AI-generated project.
  The development process included:

  - Manual debugging and testing
  - Setting up proper project structure
  - Fixing implementation issues
  - Manual code review and optimisation

  The program represents a synthesis of AI-assisted development combined with human
  oversight and debugging, showcasing how modern AI systems can work together with
  developers to create sophisticated software solutions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "file_utils.h"
#include "logging.h"

const char *proj_root_dir_path;

int main(int argc, char *argv[]) {
  // Check for correct number of arguments
  if(argc != 3) {
    fprintf(stderr, "Usage: %s '<input_mlt_file>' '<output_directory>'\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Create writable copies of the input arguments
  const char *input_file_raw = argv[1];
  const char *output_dir_raw = argv[2];
  size_t input_len = strlen(input_file_raw);
  size_t output_len = strlen(output_dir_raw);
  // Create writable copies of the input arguments
  char *input_file = malloc(input_len + 1);
  char *output_dir = malloc(output_len + 1);

  if(!input_file || !output_dir) {
    perror("Failed to allocate memory for input or output arguments");
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  strncpy(input_file, input_file_raw, input_len);
  input_file[input_len] = '\0'; // Null-terminate
  strncpy(output_dir, output_dir_raw, output_len);
  output_dir[output_len] = '\0'; // Null-terminate

  // Remove single quotes from arguments if present
  if(input_file[0] == '\'' && input_file[input_len - 1] == '\'') {
    memmove(input_file, input_file + 1, input_len - 2); // Shift left to skip leading quote
    input_file[input_len - 2] = '\0'; // Null-terminate before trailing quote
  }

  if(output_dir[0] == '\'' && output_dir[output_len - 1] == '\'') {
    memmove(output_dir, output_dir + 1, output_len - 2); // Shift left to skip leading quote
    output_dir[output_len - 2] = '\0'; // Null-terminate before trailing quote
  }

  // Step 0: Extract the directory part of the input file
  char *last_slash = strrchr(input_file, '/');

  if(!last_slash) {
    fprintf(stderr, "Error: Invalid input file path.\n");
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  size_t input_dir_len = last_slash - input_file;
  char *input_dir = malloc(input_dir_len + 1);

  if(!input_dir) {
    perror("Failed to allocate memory for input directory");
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  strncpy(input_dir, input_file, input_dir_len);
  input_dir[input_dir_len] = '\0'; // Null-terminate the input directory
  proj_root_dir_path = strdup(input_dir);
  printf("proj_root_dir_path: %s\n", proj_root_dir_path);

  // Remove the last `/` character from the end of the second argument if present
  if(output_dir[output_len - 1] == '/') {
    output_dir[output_len - 1] = '\0';
  }

  /* Debugging statements */
  /*printf("DEBUG: Input directory: %s\n", input_dir);*/
  /*printf("DEBUG: Output directory: %s\n", output_dir);*/
  // Step 1: Check if input directory matches output directory
  if(strcmp(input_dir, output_dir) == 0) {
    fprintf(stderr, "Error: Input file's directory and output directory cannot be the same.\n");
    free(input_dir);
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  free(input_dir); // Free the extracted input directory after the check
  // Step 2: Parse the project file to extract resources
  char **resources = NULL;
  size_t resource_count = 0;

  if(!parse_project_file(input_file, &resources, &resource_count)) {
    fprintf(stderr, "Error: Failed to parse the project file.\n");
    free_strings_array(resources, resource_count);
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  // Step 3: Build file mappings for cousin detection
  build_file_mappings(resources, resource_count, proj_root_dir_path);
  // Step 4: Create the assets directory
  char *assets_dir = concat_paths(output_dir, "assets");

  if(!create_directory(assets_dir)) {
    fprintf(stderr, "Error: Failed to create assets directory.\n");
    free_strings_array(resources, resource_count);
    free(assets_dir);
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  // Step 5: Create subdirectories (LUT and stabilization_data)
  char *lut3d_presets_dir = concat_paths(assets_dir, "LUT");

  if(!create_directory(lut3d_presets_dir)) {
    fprintf(stderr, "Error: Failed to create lut3d_presets directory.\n");
    free_strings_array(resources, resource_count);
    free(assets_dir);
    free(lut3d_presets_dir);
    return EXIT_FAILURE;
  }

  char *stabilization_presets_dir = concat_paths(assets_dir, "stabilization_data");

  if(!create_directory(stabilization_presets_dir)) {
    fprintf(stderr, "Error: Failed to create stabilization_data directory.\n");
    free_strings_array(resources, resource_count);
    free(assets_dir);
    free(lut3d_presets_dir);
    free(stabilization_presets_dir);
    return EXIT_FAILURE;
  }

  char *alpha_transition_dir = concat_paths(assets_dir, "alpha_transition");

  if(!create_directory(alpha_transition_dir)) {
    fprintf(stderr, "Error: Failed to create alpha_transition directory.\n");
    free_strings_array(resources, resource_count);
    free(assets_dir);
    free(lut3d_presets_dir);
    free(stabilization_presets_dir);
    free(alpha_transition_dir);
    return EXIT_FAILURE;
  }

  // Step 6: Copy assets to the output directory
  for(size_t i = 0; i < resource_count; ++i) {
    const char *destination = get_destination_path(resources[i], assets_dir);

    if(!destination) {
      continue; // Skip invalid paths
    }

    // Create the directory if it doesn't exist
    char *dest_dir = strdup(destination);

    if(!dest_dir) {
      perror("Failed to allocate memory for destination directory");
      continue;
    }

    last_slash = strrchr(dest_dir, '/');

    if(last_slash) {
      *last_slash = '\0'; // Null-terminate before the filename

      if(!create_directory(dest_dir)) {
        fprintf(stderr, "Error: Failed to create destination directory: %s\n", dest_dir);
      }
    }

    free(dest_dir);
    // Copy the file
    copy_file_to_directory_with_context(resources[i], assets_dir, proj_root_dir_path, input_file);
  }

  // Step 7: Copy and modify the project file
  // Project name should be the input project file's name
  const char *input_filename = strrchr(input_file, '/');

  if(!input_filename) {
    input_filename = input_file;
  }

  else {
    input_filename++;
  }

  // Ensure the output file has .mlt extension
  char *output_project_file = concat_paths(output_dir, input_filename);

  if(output_project_file) {
    size_t len = strlen(output_project_file);

    if(len < 4 || strcmp(output_project_file + len - 4, ".mlt") != 0) {
      char *new_output = realloc(output_project_file, len + 5);

      if(new_output) {
        output_project_file = new_output;
        strcat(output_project_file, ".mlt");
      }
    }
  }

  if(!copy_and_modify_project_file(input_file, output_project_file, assets_dir, proj_root_dir_path)) {
    fprintf(stderr, "Error: Failed to copy and modify the project file.\n");
    free_strings_array(resources, resource_count);
    free(assets_dir);
    free(output_project_file);
    free(input_file);
    free(output_dir);
    return EXIT_FAILURE;
  }

  else {
    printf("Project file %s generated successfully.\n", output_project_file);
  }

  // Clean up
  free_strings_array(resources, resource_count);
  free(assets_dir);
  free(output_project_file);
  free(input_file);
  free(output_dir);
  printf("Assets collected successfully.\n");
  free_file_mappings();
  return EXIT_SUCCESS;
}
