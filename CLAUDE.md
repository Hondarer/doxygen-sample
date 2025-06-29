# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Doxygen documentation study project demonstrating how to generate documentation from C source code. The project includes:

- Simple calculator functions in C
- Doxygen configuration for documentation generation  
- Doxybook2 integration for markdown documentation
- Custom templates for Japanese documentation output

## Build and Documentation Commands

### Build the calculator
```bash
make calculator
```

### Generate documentation
```bash
make docs
```
This command:
1. Creates Doxygen XML output in `docs/doxygen/xml/`
2. Runs Doxybook2 to convert XML to markdown in `docs-src/doxybook2/`
3. Uses custom Japanese templates from `doxybook-templates/grouping_api_doc/`
4. Runs post-processing script to handle `!include` directives in markdown files

### Alternative documentation generation
```bash
./generate_docs.sh
```
Only generates basic Doxygen HTML output without Doxybook2 conversion.

### Clean build artifacts
```bash
make clean
```

## Architecture

### Source Code Structure
- `src/calculator.h` - Header with function declarations and UserInfo struct
- `src/calculator.c` - Implementation with Doxygen comments using `@ingroup public_api`

### Documentation Pipeline
1. **Doxygen**: Parses C source files and generates XML output based on `Doxyfile` configuration
2. **Doxybook2**: Converts Doxygen XML to markdown using `doxybook-config.json` and custom templates
3. **Post-processing**: `postprocess.sh` script handles `!include` directives to merge related content
4. **Templates**: Located in `doxybook-templates/` with Japanese customizations

### Key Configuration Files
- `Doxyfile` - Doxygen configuration (UTF-8 encoding, extracts all elements)
- `doxybook-config.json` - Doxybook2 settings (filters for `public_api` group only)
- `doxybook-templates/` - Custom Jinja2 templates for Japanese output formatting

### Template Structure
The template system uses:
- `nonclass_members_details.tmpl` - Main template for organizing output sections
- `member_details.tmpl` - Handles individual element formatting (functions, structs, etc.)
- `details.tmpl` - Common template for parameter lists, returns, warnings, etc.
- `postprocess.sh` - Post-processing script that handles `!include {filename}` directives by replacing them with file contents

### Post-processing Features
- **Include directives**: Templates can use `!include filename.md` to merge content from other generated files
- **File cleanup**: After processing, removes unnecessary files like `struct*.md`, `index_classes.md`, `index_namespaces.md`
- **Error handling**: Continues processing other files even if one file fails to process
- **Statistics reporting**: Shows count of processed files and successful inclusions

### Template Development Notes
- Use `!include` directives in templates to merge related content (e.g., struct details into group pages)
- The post-processing script supports both relative and absolute paths for included files
- Included content is inserted verbatim, replacing the `!include` line
- Debug output can be enabled by uncommenting `set -x` in `postprocess.sh`

## Development Notes

The project demonstrates Doxygen `@ingroup` usage to organize API documentation into logical groups. All public functions are tagged with `@ingroup public_api` for filtered documentation generation.