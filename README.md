# MVCC Archive Tool

This tool is designed to extract or create AFS/IBIS files used in the **Marvel vs. Capcom Collection**.

## Features
- **Extract AFS/IBIS files** from the MVC Collection.
- **Create AFS/IBIS files** from directories for use in the MVC Collection.

## Usage

### Extracting Files
To extract files from an AFS/IBIS archive:

- **Drag and Drop**: Simply drag the `mvsc2.21D3D8A7` file onto the `mvccArchiveTool.exe`.
- **Command Line**:
  ```bash
  mvccArchiveTool.exe mvsc2.21D3D8A7 targetFolder\
  ```

### Creating Files
To create an AFS/IBIS archive from a directory:

- **Drag and Drop**: Drag the `mvsc2.21D3D8A7.dir` folder onto the `mvccArchiveTool.exe`.
- **Command Line**:
  ```bash
  mvccArchiveTool.exe mvsc2.21D3D8A7.dir filename.afs
  ```

## Requirements
- Windows OS

## License
This project is licensed under the MIT License.
