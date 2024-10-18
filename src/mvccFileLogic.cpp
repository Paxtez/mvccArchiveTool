#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include "mvccFileLogic.h"

using namespace std;
namespace fs = filesystem;
const size_t ibis_size = 0x40;

std::map<std::string, std::string> file_alias;
std::map<std::string, std::string> file_alias_reverse;

// Function to get the file size
uint32_t getFileSize(const std::string& filepath) {
    return static_cast<uint32_t>(fs::file_size(filepath));
}

// Function to pad the file to the nearest 0x800 boundary
void padToBoundary(std::ofstream& outputFile, uint32_t boundary, int shift = 0) {
    std::streampos currentPos = (outputFile.tellp());
	currentPos -= shift;
    uint32_t paddingSize = static_cast<uint32_t>(boundary - (currentPos % boundary));
    if (paddingSize < boundary) {  // Padding is required only if not aligned
        std::vector<char> padding(paddingSize, 0);
        outputFile.write(padding.data(), paddingSize);
    }
}

// JEDs code to read the date and turn it into an int? //
static uint32_t read32le(const std::vector<uint8_t>& data, size_t loc) {
	return data[loc] |
		   (data[loc + 1] << 8) |
		   (data[loc + 2] << 16) |
		   (data[loc + 3] << 24);
}

// JEDS code to save a file (modified) //
static bool saveFile(uint32_t start, uint32_t size, const std::string& dir, const std::string& fname,const std::vector<uint8_t>& afs) {
	//if (size == 0) return false;

	std::string file_name = dir + "\\" + fname;

	std::ofstream newFile(file_name, std::ios::binary);
	if (newFile.is_open()) {
		newFile.write(reinterpret_cast<const char*>(afs.data() + start), size);
		newFile.close();
		return true;
	}
	return false;
}

// Looks up a filename from the alias array //
std::string f_getFilename(int number, std::string source)
{
    string extension = "BIN";

	// base name
	string num = std::to_string(number).insert(0, 3 - std::to_string(number).length(), '0');
	string name;

	string key = source + "." + num;
	if (::file_alias.find(key) != ::file_alias.end())
	{
		name = ::file_alias[key];
	}
	else
		name = "F" + num;

	return name + "." + extension;
}

std::string readBytes(std::ifstream& file, int offset, const int &bytes) {
    // Move the file pointer to the specified offset
    file.seekg(offset);

    // Check if the seek was successful
    if (!file.good()) {
        throw std::runtime_error("Failed to seek to the specified offset.");
    }

    // Create a buffer to hold the bytes read
    char buffer[bytes + 1]; // +1 for the null terminator
    file.read(buffer, bytes); // Read bytes from the file

    // Check if the read was successful
    if (!file.good() && !file.eof()) {
        throw std::runtime_error("Failed to read the specified number of bytes.");
    }

    // Null-terminate the string (only do this if we've read any bytes)
    buffer[file.gcount()] = '\0'; // Use gcount() to avoid overwriting past the buffer if fewer bytes were read


//	cout << " READ: " << std::string(buffer) << endl;
    // Create a string from the buffer
    return std::string(buffer);
}

// Extract the IBIS / AFS file //
bool f_extractFile(std::string source_file, string target = "")
{
	if (!fs::exists(source_file))
	{
		std::cout << " Unable to locate file:" << std::endl << source_file << std::endl;
		return false;
	}
	string source = "MVSC2";

	fs::path filePath = source_file;
	string filename = filePath.filename().string();

	if (filename.find(".") != string::npos)
	{
		source = filename.substr(0, filename.find("."));
		std::transform(source.begin(), source.end(), source.begin(), ::toupper);
	}


	string path = fs::absolute(source_file).string();

	if (path.rfind("\\") != string::npos)
	{
		path = path.substr(0, path.rfind("\\") + 1);
	}

//cout << "PATH: " << path << endl;

	cout << " ... EXTRACTING FILE: " << source_file << "-" << source << " ... " << endl;


	// Open the file //
    std::ifstream file(source_file, std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "Unable to open file:" << std::endl << source_file << std::endl;
		return false;
	}


	// Grab the bytes after the normal IBIS header to see if it is a the IBIS/AFS file //
	string check = readBytes(file, 0, 4);

	if (check != "IBIS")
	{
		std::cout << " ... This does not appear to be a MVCC IBIS/AFS file! ... " << std::endl;
		return false;
	}

	// Grab the bytes after the normal IBIS header to see if it is a the IBIS/AFS file //
	check = readBytes(file, ibis_size, 3);
	if (check == "AFS")
	{
		source = "MVSC2";
	}

	// If this is marvel shift the reading //
	if (source == "MVSC2")
	{
		file.seekg(ibis_size);
	}
	else
		file.seekg(0);

	// Store the contents of the file as a vector //
    std::vector<uint8_t> afsfile((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    uint32_t file_count = read32le(afsfile, 4);
	uint32_t files_extracted = 0;

	string folder = path + filename + ".dir";

	if (target != "")
	{
		folder = target;
	}

    // Create the output directory if it doesn't exist
	if (!fs::exists(folder)) {
        fs::create_directory(folder);
    }

	if (!fs::exists(folder)) {
		std::cout << " ... ERROR CREATING FOLDER " << folder << " ..." << std::endl;
		return false;
    }

    for (uint32_t i = 0; i < file_count; ++i) {
        uint32_t file_loc = read32le(afsfile, 8 + (i * 8));
        uint32_t file_size = read32le(afsfile, 12 + (i * 8));

		std::string this_file = f_getFilename(i, source);

		// For debugging //
/*
        std::cout << "   File: " << i << ", Loc: 0x" << std::hex << file_loc
                << ", Size: 0x" << file_size << std::dec << ", Name: " << this_file << endl;
				*/



        if (saveFile(file_loc, file_size, folder, this_file, afsfile))
		{
			files_extracted++;
		}
		else
		{
			std::cout << " ... ERROR CREATING FILE " << this_file << " ..." << std::endl;
			return false;
		}
    }

	cout << " ... EXTRACTED: " << files_extracted << " " << source << " FILE(S) TO " << folder << " ... " << endl;

	return true;
}

bool f_createIBIS(string path, string target, std::vector<AFSFileEntry> files)
{
	std::ofstream afsFile(target, std::ios::binary | std::ios::trunc);
    if (!afsFile) {
		std::cout << " ... Error creating Archive file: " << target << " ..." << std::endl;
		return false;
    }

	// Create IBIS Header //
    afsFile.write("IBIS", 4);

	// Write the number of files
    uint32_t fileCount = static_cast<uint32_t>(files.size());
    afsFile.write(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
    afsFile.seekp(ibis_size);
    // Write the actual file data and calculate offsets
    for (auto& fileEntry : files) {
        // Update the offset for the current file
        fileEntry.offset = static_cast<uint32_t>(afsFile.tellp());
        // Read the file data and write it to the AFS file
        std::ifstream inputFile(fileEntry.filename, std::ios::binary);
        if (!inputFile) {
			std::cout << "ERROR: Reading file: " << fileEntry.filename << std::endl;
			return false;
        }

        std::vector<char> buffer(fileEntry.size);
        inputFile.read(buffer.data(), fileEntry.size);
        afsFile.write(buffer.data(), fileEntry.size);
        inputFile.close();

    }

    // Go back and write the file table (offsets, sizes, and filenames)
    afsFile.seekp(0x8, ios::beg);
    for (const auto& fileEntry : files) {
        afsFile.write(reinterpret_cast<const char*>(&fileEntry.offset), sizeof(fileEntry.offset));
        afsFile.write(reinterpret_cast<const char*>(&fileEntry.size), sizeof(fileEntry.size));
		// For debugging //
		//cout << fileEntry.filename << " 0x" << int_to_hex(fileEntry.offset) <<  " 0x" << int_to_hex(fileEntry.size) << endl;
    }

    afsFile.close();

	return true;
}

// Creates the IBIS/AFS archive used for mvc2 //
bool f_createAFS(string path, string target, std::vector<AFSFileEntry> files)
{
	std::ofstream afsFile(target, std::ios::binary | std::ios::trunc);
    if (!afsFile) {
		std::cout << " ... Error creating Archive file ..." << std::endl;
		return false;
    }

	// Create IBIS Header //
	afsFile.write("IBIS", 4);
	int outerFileCount = 1;
	afsFile.write((char*)&outerFileCount, 4);
	afsFile.write((char*)&ibis_size, 4);

	// Create AFS header //
	afsFile.seekp(ibis_size);
    afsFile.write("AFS\0", 4);

	// Write the number of files
    uint32_t fileCount = static_cast<uint32_t>(files.size());
    afsFile.write(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));

    // Seek to where the file data is stored //
    afsFile.seekp(0x80000 + ibis_size);

    // Write the actual file data and calculate offsets
    for (auto& fileEntry : files) {
        // Update the offset for the current file
        fileEntry.offset = static_cast<uint32_t>(afsFile.tellp());
		fileEntry.offset -= ibis_size;
        // Read the file data and write it to the AFS file
        std::ifstream inputFile(fileEntry.filename, std::ios::binary);
        if (!inputFile) {
			std::cout << " ... Error reading file: " << fileEntry.filename << " ..." << std::endl;
			return false;
        }

        std::vector<char> buffer(fileEntry.size);
        inputFile.read(buffer.data(), fileEntry.size);
        afsFile.write(buffer.data(), fileEntry.size);
        inputFile.close();

		padToBoundary(afsFile, 0x800, ibis_size);
    }
	uint32_t totalSize = static_cast<uint32_t>(afsFile.tellp()) - ibis_size;

    // Go back and write the file table (offsets, sizes, and filenames)
    afsFile.seekp(0x8 + ibis_size, ios::beg);
    for (const auto& fileEntry : files) {
        afsFile.write(reinterpret_cast<const char*>(&fileEntry.offset), sizeof(fileEntry.offset));
        afsFile.write(reinterpret_cast<const char*>(&fileEntry.size), sizeof(fileEntry.size));
	//	cout << fileEntry.filename << " 0x" << int_to_hex(fileEntry.offset) <<  " 0x" << int_to_hex(fileEntry.size) << endl;
    }


	// Write the total file size //
	afsFile.seekp(0xC, ios::beg);
	afsFile.write(reinterpret_cast<const char*>(&totalSize), sizeof(totalSize));

	// Oddly the filesize is also here //
	afsFile.seekp(0x80038, ios::beg);
	afsFile.write(reinterpret_cast<const char*>(&totalSize), sizeof(totalSize));

	// And some hex values //
	std::vector<unsigned char> data = {0xE0, 0xA6, 0x00, 0x00};
	afsFile.write(reinterpret_cast<const char*>(data.data()), data.size());

    afsFile.close();

	return true;
}

// Does the setup for creation of the IBIS / AFS archives
// Path is the folder, optional target says the path and filename of the target //
bool f_createArchive(string path, string target = "")
{
	// The file name of the new archive //
	string targetFile;

	// A map containing the files in the path //
	map<int, std::string> file_listing;

	// Contains the name of the game mvsc2.blahblah = MVSC2
	string source = "";

	fs::path filePath = path;

	string filename = filePath.filename().string();

	if (filename.find(".") != string::npos)
	{
		source = filename.substr(0, filename.find("."));
		std::transform(source.begin(), source.end(), source.begin(), ::toupper);
	}


	if (source == "") {
		std::cout << " ... Unable to determine source from path: " << filename << " ..." << std::endl;
		return false;
	}

	// Create listing of all the files in the path //
    for (const auto & entry : fs::directory_iterator(path))
	{
		fs::path fp = entry;
		string filename = fp.filename().string();
		std::transform(filename.begin(), filename.end(), filename.begin(), ::toupper);
		int num = -1;

		// Old style file name //
		if (filename[0] == 'F' && isdigit(filename.at(1)) && isdigit(filename.at(2)) && isdigit(filename.at(3)))
		{
			num = stoi(filename.substr(1,3));
		}
		// Look up the filename in the reverse alias array //
		else if (filename.find(".") != string::npos)
		{
			string key = source + "." + filename.substr(0, filename.find("."));

			if (::file_alias_reverse.find(key) != ::file_alias_reverse.end())
			{
				num = stoi(::file_alias_reverse[key]);
			}
		}

		if (num == -1)
			continue;

		if (file_listing.find(num) != file_listing.end())
		{
			std::cout << " ERROR: Duplicate file number: " << std::to_string(num) + " found!" << std::endl << filename << " & " << file_listing[num] << std::endl;
			return false;
		}
		file_listing[num] = filename;
	}

	if (file_listing.size() == 0)
	{
		std::cout << " ... ERROR: No Files found! ..." << std::endl;
		return false;
	}

	// Verify all the files are there //
	// Todo maybe not extract 0 bytes and add them? //
	int mapSize = file_listing.size();

	for (int x = 0; x < mapSize; x++)
	{
		if (file_listing.find(x) == file_listing.end())
		{
			std::cout << "ERROR: Missing file number:" << std::to_string(x) << "!" << std::endl;
			return false;
		}
	}

    // Create a vector of AFSFileEntry to store file information
    std::vector<AFSFileEntry> files;

    // Populate the AFS file entries from the file_listing map
    for (const auto& [index, filename] : file_listing) {
        AFSFileEntry fileEntry;
        fileEntry.filename = path + "\\" + filename;
        fileEntry.size = getFileSize(path + "\\" + filename); // Calculate file size dynamically
        fileEntry.offset = 0; // Will be calculated later
        files.push_back(fileEntry);
    }

	// Figure out what we should call the new file //
	if (target != "")
		targetFile = target;
	else
	{
		// Remove the .dir which we added //
		targetFile = path;
		if (targetFile.find(".dir") != string::npos)
		{
			targetFile = targetFile.substr(0, targetFile.find(".dir"));
		}

		// Don't replace the original file //
		if (fs::exists(targetFile))
		{
			targetFile = targetFile + ".new";
		}
	}

	cout << " ... CREATING:" << targetFile << " ..." << endl << " ... FROM " << path << " (" << files.size() << " FILES) ..." << endl;
	if (source == "MVSC2")
		return f_createAFS(path, targetFile, files);

	return f_createIBIS(path, targetFile, files);
}
