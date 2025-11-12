# SDK Documentation

This directory contains the Alif Semiconductor SDK technical documentation, which provides detailed information about connectivity features, modules, and Zephyr integration for Alif hardware.

## Prerequisites

See [alif/doc/README.md](../README.md) for complete prerequisites and installation instructions.

## Building the Documentation

### HTML Output

To build the documentation in HTML format:

```bash
cd alif/doc/sdk
make html
```

The output will be in `build/html/index.html`.

For a single-page HTML version:

```bash
make singlehtml
```


### Cleaning Build Artifacts

To remove all build artifacts:

```bash
make clean
```

## Documentation Structure

The SDK documentation is organized as follows:

- **connectivity/** - Bluetooth, BLE audio, and wireless connectivity documentation
- **modules/** - SDK modules and components
- **zephyr_integration/** - Zephyr RTOS integration details
- **images/** - Diagrams and figures
- **conf.py** - Sphinx configuration
- **links.txt** - External link definitions
- **substitutions.txt** - Text substitutions and variables

## Cross-References to Zephyr

This documentation uses Sphinx intersphinx to cross-reference the Zephyr RTOS documentation.

### Local Zephyr Documentation

If you have built the Zephyr documentation locally, the SDK documentation will automatically use it for cross-references. To build the local Zephyr documentation:

```bash
cd zephyr/doc
make html
```

The SDK documentation will look for the built documentation at `zephyr/doc/_build/html/objects.inv`. If found, all Zephyr cross-references will point to your local build.

### Fallback to Online Documentation

If local Zephyr documentation is not found, the SDK documentation will automatically fall back to the online Zephyr documentation (version specified in `conf.py`).

This allows you to build the SDK documentation without building Zephyr documentation first, while still providing working cross-references.

**Note:** The online Zephyr documentation is the upstream vanilla version and does not include Alif-specific modifications, board definitions, or drivers that are present in the Alif Zephyr SDK repository. For complete and accurate cross-references to Alif-specific features, build the local Zephyr documentation.

## Notes

- The documentation uses the `-W` flag (warnings as errors)
- All external links are defined in `links.txt` for consistency
- Common text substitutions are defined in `substitutions.txt`
