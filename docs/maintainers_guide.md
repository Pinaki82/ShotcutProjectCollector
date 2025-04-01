# Shotcut Project Collector - Maintainers Guide

## 1. Introduction

This guide provides a detailed overview of the Shotcut Project Collector, including its architecture, core components, and development best practices. It is intended for developers maintaining or extending the project.

[Shotcut](https://www.shotcut.org/) does not have a built-in assets collector as of 2025.04.01. The project is necessary as long as Shotcut does not come with this feature built in. They are encouraged to take ideas from this project and incorporate a built-in asset collector. The MIT-0 license for this project permits anyone to use it without citing the original code author.

## 2. Architecture

### Core Components

The program consists of the following main components:

- **main.c**: Entry point for the application and argument handling
- **file_utils.c**: File processing and manipulation functions
- **parser.c**: MLT project file parsing and processing
- **logging.c**: Logging functionality

### File Structure

```
shotcut_project_collector/
├── src/
│   ├── main.c             # Program entry point
│   ├── file_utils.c       # File processing functions
│   ├── parser.c           # MLT file parsing
│   └── logging.c          # Logging functionality
│   ├── file_utils.h
│   ├── parser.h
│   └── logging.h
└── docs/
    └── maintainers_guide.md
```

## 3. Memory Management

### Key Concepts

1. **String Allocation**
   
   - All file paths are dynamically allocated
   - Resource lists use dynamic arrays
   - File mappings are stored in a global array

2. **Memory Cleanup**
   
   - Always use `free()` for allocated memory
   - Use `free_file_mappings()` to clean up file mapping data
   - Check return values of memory allocation functions

### Example

```c
char *path = malloc(length);
if (!path) {
    perror("Failed to allocate memory");
    return NULL;
}
```

## 4. Error Handling

### Input Validation

```c
if (argc != 3) {
    fprintf(stderr, "Usage: %s '<input_mlt_file>' '<output_directory>'\n", argv[0]);
    return EXIT_FAILURE;
}
```

### File Operations

- Check return values of `fopen()`, `fread()`, `fwrite()`
- Verify directory creation with `create_directory()`
- Handle file copy failures in `copy_file_to_directory()`

## 5. File Processing

The program handles three types of files:

1. **Resource Files**
   
   - Identified by `<property name="resource">` tags
   - Copied to `assets/` directory
   - Updated paths in project file

2. **LUT Files**
   
   - Identified by `<property name="av.file">` tags
   - Copied to `assets/LUT/` directory
   - Special handling for colour grading files

3. **Stabiliser Files**
   
   - Identified by `<property name="filename">` tags
   - Copied to `assets/stabilization_data/` directory
   - Used for video stabilisation data

## 6. Path Handling

### Key Features

1. **Absolute vs Relative Paths**
   
   - Converts relative paths to absolute using `project_root`
   - Maintains proper path separators

2. **Path Construction**

```c
char *assets_dir = concat_paths(output_dir, "assets");
char *lut_dir = concat_paths(assets_dir, "LUT");
```

3. **Path Validation**
   - Checks for valid directory separators
   - Verifies path existence
   - Handles trailing slashes

## 7. Resource Types

The program handles three main types of resources:

1. **Media Files**
   
   - Video files (webm, mp4, mov, avi, mxf)
   - Audio files (wav, mp3, ogg)
   - Image and vector files (png, jpg, svg)

2. **LUT Files**
   
   - Colour grading files (cube, lut)
   - Special handling for 3D LUTs

3. **Stabiliser Files**
   
   - Video stabilisation data
   - Motion tracking files (Planned)

## 8. Directory Structure

The program creates the following directory structure:

```
output_directory/
├── assets/
│   ├── video.webm
│   ├── audio.mp3
│   └── image.png
├── assets/LUT/
│   └── color_grading.cube
└── assets/stabilization_data/
    └── stabilization.txt
```

## 9. Project File Modification

The program modifies the MLT project file in several ways:

1. **Resource Path Updates**
   
   - Replaces absolute paths with relative paths
   - Maintains XML structure
   - Preserves all other project settings

2. **Backup System**
   
   - Does not alter the original project file and used assets

## 10. Common Issues

1. **Memory Leaks**
   
   - Always pair `malloc()` with `free()`
   - Use `free_fileMappings()` before exit
   - Check for null pointers

2. **Path Resolution**
   
   - Handle both absolute and relative paths
   - Check for circular references

3. **File Copy Failures**
   
   - Verify source file existence
   - Handle large file transfers

## 11. Debugging

### Tools

1. **Logging (for future use)**
   
   - Use `logging.h` functions to get debug output in detail for maintenance purposes
   - Enable debug statements for detailed output
   - Check log files for error messages

2. **Memory Debugging**
   
   - Use `valgrind` or [Dr. Memory](https://drmemory.org/) for memory leaks
   - Check memory allocation patterns
   - Verify proper cleanup

3. **Path Debugging**
   
   - Print all path transformations
   - Verify directory creation
   - Check the reflected path transformations in the generated project file

## 12. Performance Considerations

1. **Memory Usage**
   
   - Use efficient data structures
   - Minimise memory allocations
   - Implement proper cleanup

2. **File Operations**
   
   - Avoids copying existing files.

## 13. Testing

- Use a comprehensive testing framework
- Use manual checking of the assets and generated projects in the practical scenarios

## 14. Code Style

- Follow standard C coding conventions
- Use consistent indentation
- Include comments for complex logic

## 15. Contributing Guidelines

- Fork the repository
- Create feature branches
- Write clear commit messages
- Submit pull requests

## 16. Conclusion

This guide provides a comprehensive overview of the Shotcut Project Collector's architecture, functionality, and best practices for maintainers. By following these guidelines, developers can effectively maintain and enhance the project.
