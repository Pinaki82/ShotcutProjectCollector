# Shotcut Project Collector

## Overview

This program helps organise MLT (Shotcut) project files by collecting all their dependencies into a single, organised structure. Think of it as a tool that makes your video projects more portable and easier to manage.

## What This Program Does

Imagine you have a video project with lots of files scattered all over your computer:

- Video clips
- Audio files
- Color grading files (LUTs)
- stabilisation data
- And many more...

This program gathers all these files into one place and updates your project file to point to the new locations. It's like cleaning up your room by putting everything in its proper place!

## How It Works (For Dummies)

### 1. Input and Output

- You give the program two things:
  - Your MLT project file (the .mlt file)
  - A directory where you want everything to be organised

### 2. The Magic Happens

The program does several important things:

#### A. Creates a New Structure

It creates a special folder called "assets" with three important subfolders:

- `assets/` - For your main media files (videos, images, etc.)
- `assets/LUT/` - For your colour grading files
- `assets/stabilization_data/` - For your video stabilisation files

#### B. Finds All Your Files

It looks through your project file to find all the files it needs:

- Video files (marked with `<property name="resource">`)
- Colour grading files (marked with `<property name="av.file">`)
- Stabilisation files (marked with `<property name="filename">`)

#### C. Handles Special Cases

Sometimes you might have files with the same name but in different folders. The program is smart enough to handle these "cousin" files by:

- Giving each cousin file its own special folder
- Making sure they don't get mixed up

### 3. The Result

After running the program, you'll have:

1. All your project files are neatly organised in the `assets` folder
2. A new project file that points to the right places
3. A portable project that you can easily share with others

## Technical Details (For Advanced Users)

### File Handling Process

1. **Input Validation**
   
   - Checks if you provided the correct number of arguments
   - Verifies that the input file exists
   - Ensures the output directory is valid

2. **Resource Processing**
   
   - Parses the MLT project file
   - Identifies all referenced files
   - Creates a mapping of all files and their relationships

3. **File Organisation**
   
   - Creates the necessary directory structure
   - Handles special cases like "cousin" files
   - Maintains proper file permissions

4. **Project File Update**
   
   - Updates all file paths to point to the new locations
   - Maintains the original project structure
   - Preserves all project settings

### Error Handling

The program includes robust error handling for:

- Missing files
- Permission issues
- Invalid paths
- Memory allocation failures

## Usage Examples

### Basic Usage

```bash
./shotcut_project_collector '/path/to/your/project.mlt' '/path/to/output/directory'
```

### Important Notes

- The input file's directory and output directory cannot be the same
- The program will create all necessary directories if they don't exist
- All file paths in the project file will be updated to use relative paths

## Maintenance Tips

### Common Issues

1. **Missing Files**
   
   - Check if all source files are accessible
   - Verify file permissions
   - Ensure network drives are mounted (if applicable)

2. **Path Issues**
   
   - Use absolute paths for input files
   - Make sure the output directory has proper write permissions

3. **Memory Issues**
   
   - The program uses dynamic memory allocation
   - Ensure sufficient system memory is available

### Debugging

The program includes logging functionality that can help diagnose issues:

- Check log files for error messages
- Use the debug prints to track file processing
- Verify file mappings for correct relationships

## Future Enhancements

### Potential Improvements

1. **Better Error Reporting**
   
   - More detailed error messages
   - Better logging of file operations

2. **Additional Features**
   
   - Support for more file types
   - Better handling of large projects
   - Improved performance for many files

3. **User Interface**
   
   - Add a progress indicator
   - Better error feedback
   - Option to preview changes before applying

## Contributing

### How to Help

1. **Testing**
   
   - Test with different project types
   - Check edge cases
   - Verify file handling

2. **Documentation**
   
   - Update usage examples
   - Add more detailed explanations
   - Create troubleshooting guides

3. **Code Improvements**
   
   - Enhance error handling
   - Optimise performance
   - Add new features

## License

This program is licensed under the MIT-0. See the LICENSE file for details.

## Acknowledgements

This program was developed with the help of multiple AI systems:

- Qwen 2.5 Turbo
- Claude Sonnet 3.7
- Codium Base

And was developed using both GVim (Tulu-C-IDE) and Windsurf IDEs.
