#define APP_NAME "MVC COLLECTION ARCHIVE TOOL"
#define APP_VER "v0.9"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <conio.h>  // for _kbhit() and _getch() on Windows
#include "globals.h"
#include "mvccFileLogic.h"

using namespace std;
namespace fs = filesystem;

// Determines if we pause on successes, don't do this for command line uses //
bool goodPause = true;
std::string alias_file_name = "mvcc_file_names.cfg";


// Function to simulate DOS-like "pause" with a timeout
void timedPause(int timeoutSeconds) {
    std::cout << "Press any key to continue... (Timeout in " << timeoutSeconds << " seconds)" << std::endl;

    auto start = std::chrono::steady_clock::now();  // Start time
    while (true) {
        // Check if a key was pressed
        if (_kbhit()) {
            _getch();  // Consume the key press
            break;
        }

        // Check if the timeout has been reached
        auto now = std::chrono::steady_clock::now();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        if (elapsedSeconds >= timeoutSeconds) {
            std::cout << "\nTimeout reached after " << timeoutSeconds << " seconds." << std::endl;
            break;
        }
    }
}

// Draws text with an ascii frame around it //
void Frame(const Array& textArray, int Mod = 1, int Head = 1,
	int Foot = 1)
{             // Mod 0 = Normal bar | Mod 1 = Center, Long bar | Mod 2 = Short bar.
	string BoxPad = "    ";
	string FrameBar = "";
	unsigned int BoxLen = 60;

	int lines = textArray.size();

	// Adjust length of box based on Mod option //
	if (Mod > 1)
	{
		BoxLen -= 15;
		Mod = 0;
	}
	if (Mod)
	{
		BoxLen += 6;
		BoxPad = " ";
	}
	FrameBar.append(BoxLen, (char)205);

	// Draw stuff //
	if (Head)
	{
		printf("\n%s%c%s%c\n", BoxPad.c_str(), (char)201, FrameBar.c_str(), (char)187);
	}

	if (lines > 0)
	{
		for (const auto& a : textArray) {
			string line = a;
			if (line.size() > BoxLen - 2)
			{
				line = line.substr(0, BoxLen - 5) + "...";
			}
			line.insert(0, 1+(BoxLen-line.size()-1)*Mod/2, ' ');
			line.append(BoxLen-line.size(), ' ');
			std::cout << BoxPad << (char)186 << line << (char)186 << endl; // Print each string followed by a carriage return and newline
		}
	}

	if (Foot)
	{
		printf("%s%c%s%c\n", BoxPad.c_str(), (char)200, FrameBar.c_str(), (char)188);
	}
}

// A uniform way to exit the program, shows a message, cleans up files, says goodbye, etc. //
void f_quit(const Array& errorArray = {})
{
	// Show error //
	if (!errorArray.empty())
	{
		Frame(errorArray, 0);
	}

	if ((!errorArray.empty()) || (goodPause))
	{
		Frame({APP_NAME " " APP_VER,
			"Thanks to: TVI, mountainmanjed, Preppy, ChatGPT (lol)",
			"www.paxtez.zachd.com  |  Discord: Paxtez  |  Reddit: Paxtez"});
		timedPause(15);
	}
	exit(1);
}


// Loads the CFG file that contains the file aliases //
void f_loadFileAlias()
{
	if (!fs::exists(alias_file_name))
	{
		alias_file_name = "tools\\" + alias_file_name;
		if (!fs::exists(alias_file_name))
		{
			return;
		}
	}

	fstream file(alias_file_name);

    // Checking whether the file is open.
    if (!file.is_open())
	{
		return;
	}

	int count = 0;
	string sa;


	//cout << " ... LOADING ALIAS FILE : " ;
	// Read data from the file object and put it into a string.
	while (getline(file, sa)) {
		if (sa.find("=") == string::npos)
			continue;
		string source;
		std::transform(sa.begin(), sa.end(), sa.begin(), ::toupper);
		string num;
		if (sa.find(".") == string::npos)
		{
			source = "MVSC2";
			num = sa.substr(0, sa.find("="));
		}
		else
		{
			source = sa.substr(0, sa.find("."));
			num = sa.substr(sa.find(".") + 1, sa.find("=") - sa.find(".") - 1);
		}
		string code = source + "." + num;
		string alias = sa.substr(sa.find("=") + 1);

//cout << " sa: " << sa << " source: " << source << " num: " << num << " file: " << alias << endl;
		if ((code.size() == 0) || (alias.size() == 0))
			continue;


		file_alias[code] = alias;
		if (::file_alias_reverse.find(source + "." + alias) != ::file_alias_reverse.end())
		{
			f_quit({"ERROR: DUPLICATE FILE NAME FOUND IN ALIAS FILE",
					source + "." + alias,
					file_alias_reverse[source + "." + alias]});

		}
		else
			file_alias_reverse[source + "." + alias] = num;
		count++;
	}

	// Close the file object.
	file.close();

//	cout << " Found: " << count << " ..." << endl;

	/*
	//For debugging:
	for (const auto& pair : file_alias) {
        std::cout << pair.first << " is " << pair.second << std::endl;
    }*/
	
/*
	//For debugging:
	for (const auto& pair : file_alias_reverse) {
        std::cout << pair.first << " is " << pair.second << std::endl;
    }*/

	return;
}



// Main program //
int main(int argc, char* argv[])
{
	Array usageError = {
        "This tool will extract or create the AFS/IBIS files",
        "used in the MVC Collection.",
        "",
		"TO EXTRACT:",
		" Drag the mvsc2.21D3D8A7 file on this .exe",
		" OR with a command line:",
		"  \"mvccArchiveTool.exe mvsc2.21D3D8A7 targetFolder\\\"",
        "",
		"TO CREATE:",
		" Drag the mvsc2.21D3D8A7.dir folder on this .exe",
		" OR with a command line:",
		"  \"mvccArchiveTool.exe mvsc2.21D3D8A7.dir filename.afs\"",
    };

	if (argc < 2)
	{
		f_quit(usageError);
	}

	string source = argv[1];
	string target = "";

	if (argc > 2)
	{
		goodPause = false;
		if (string(argv[2]) != "-Q")
		{
			target = argv[2];
		}
	}

	// Load the cfg file that has the file aliases //
	f_loadFileAlias();

	// Check if the main thing is a folder to create an Archive //
	if (fs::is_directory(source))
	{
		f_createArchive(source, target);
	}
	// If the main thing is a file
	else if (fs::is_regular_file(source))
	{
		f_extractFile(source, target);
	}
	else
	{
		f_quit(usageError);
	}


	f_quit();
    return 0;
}