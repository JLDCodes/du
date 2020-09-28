/*
/Name of Program: du - Disk utility 
/Coder: Jean-Luc
/Date: April 20 2020
/Purpose: console application that shows disk usage (based on the UNIX command)
*/
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <algorithm>
#include <unordered_set>
using namespace std;
using namespace std::filesystem;

vector<pair<int, string>> scan(path const& f, int clusterSize, vector<pair<int, string>> folders, bool specified);
double roundWhole(double n);
string human_format(int bytes);
void printVector(vector<pair<int, string>> folders, bool h);
vector<pair<int, string>> formatVector(vector<pair<int, string>> folders, bool s, bool n, bool z, bool b, bool r, int blockSize);
vector<pair<int, string>> recursiveFileSearcher(path const& p, int clusterSize, int& fileSize, vector<pair<int, string>> folders, bool specified);
int maxLength(vector<pair<int, string>> folders);


int main(int argc, char* argv[]) {

	vector<string> argVec;
	//convert argv to a c++ vector
	for (int i = 1; i < argc; ++i) {
		argVec.push_back(argv[i]);
	}
	int const INITIAL_BLOCKSIZE = 4096;
	int blockSize = INITIAL_BLOCKSIZE;
	vector <pair<int, string>> folders;
	bool specified = false;
	// No args case
	if (argVec.empty())
	{
		folders = scan(current_path(), blockSize, folders, specified);
		int length = maxLength(folders);
		for (size_t i = 0; i < folders.size(); i++)
		{
			cout << left << setw(length) << folders[i].first << setw(3) << ' ' << folders[i].second << endl;
		}
		return EXIT_SUCCESS;
	}

	path curPath = current_path();
	vector<path> pathV;
	bool bs = false;
	string help = "\ndu (c) 2020, Jean-Luc Desjardins\n===========================================================\nVersion 1.1.0\n\nA disk usage utility inspired by the UNIX du command.\n\nUsage: du [-skhb] [--help] [--version] [--block-size=dddd] [folder]*\n\nExamples:\n  du\n    > display the sum of the cluster sizes of each directory\n      starting the cwd\n\n  du folder\n    > display the sum of the cluster sizes of each directory\n      starting with 'folder'\n\n  du -h\n    > display the results in a human readable form\n\n  du -s\n    > display only the final summary\n\n  du -b\n    > display in bytes\n\n  du -k\n    > cluster size is 1024\n\n  du -z\n    > display the list sorted by size\n\n  du -n\n    > display the list sorted by name\n\n  du -r\n    > display the list in reverse order\n\n  du --block-size=dddd\n    > set the cluster size to the specified integer > 0\n\n  du --help\n    > displays the help\n\n  du --version \n    > displays version number in the format d.d.d";
	// for the -- arguments
	for (auto& data : argVec) {

		if (data == "--version") {
			cout << "1.1.0" << endl;
			return EXIT_SUCCESS;
		}
		else if (data == "--help") {
			cout << help << endl;
			return EXIT_SUCCESS;
		}
		else if (data.substr(0, 13) == "--block-size=") {
			bs = true;
			string blockSizeEnd = data.substr(13, data.size() - 13);
			for (size_t i = 0; i < blockSizeEnd.size(); ++i) {
				if (isalpha(blockSizeEnd[i])) {
					cout << "Error: block-size value is invalid <x>" << endl;
					return EXIT_FAILURE;
				}
			}
			blockSize = stoi(blockSizeEnd);
		}
		else if (data.at(0) != '-') {
			curPath = data;
			pathV.push_back(data);
		}

	}
	// if the path has changed then specified path was given
	if (curPath != current_path()) {
		specified = true;
	}

	// for the switches 
	bool b = false;
	bool h = false;
	bool z = false;
	bool n = false;
	bool k = false;
	bool s = false;
	bool r = false;
	if (argVec[0][0] == '-') {
		if (argVec[0][1] != '-') {
			string availablSwitches = "skhznrb";
			string switches = argVec.front().substr(1);
			for (size_t i = 0; i < switches.length(); i++)
			{

				auto it = availablSwitches.find(switches[i]);

				if (it == string::npos)
				{
					cout << "Error: unknown switches: <" << switches[i] << ">" << std::endl;
					return EXIT_FAILURE;
				}
			}
			for (unsigned i = 0; i < argVec[0].size(); ++i) {

				if (argVec[0][i] == 'h') {
					if (b == true) {
						cout << "Error: cannot use both -b and -h" << endl;
						return EXIT_FAILURE;
					}
					h = true;
				}
				if (argVec[0][i] == 'b') {
					if (h == true) {
						cout << "Error: cannot use both -b and -h" << endl;
						return EXIT_FAILURE;
					}
					b = true;
				}
				if (argVec[0][i] == 'z') {
					if (n == true) {
						cout << "Error: -n and -z switches are incompatible." << endl;
						return EXIT_FAILURE;
					}
					z = true;
				}
				if (argVec[0][i] == 'n') {
					if (z == true) {
						cout << "Error: -n and -z switches are incompatible." << endl;
						return EXIT_FAILURE;
					}
					n = true;
				}
				if (argVec[0][i] == 'k') {
					if (bs == true) {
						cout << "Error: -k and --block-size are incompatible." << endl;
						return EXIT_FAILURE;
					}
					k = true;
					blockSize = 1024;
				}
				if (argVec[0][i] == 's') {
					s = true;
				}
				if (argVec[0][i] == 'r') {
					r = true;
				}

			}
			if (argVec.size() > 0) {
				if (bs == true) {
					for (size_t i = 0; i < argVec.size(); ++i) {

						if (argVec[0][i] == 'k') {
							if (bs == true) {
								cout << "Error: -k and --block-size are incompatible." << endl;
								return EXIT_FAILURE;
							}
						}
					}
				}
			}
		}
	}

	if (pathV.size() == 0) {
		folders = scan(curPath, blockSize, folders, specified);
		folders = formatVector(folders, s, n, z, b, r, blockSize);
		printVector(folders, h);
	}
	else {
		vector <pair<int, string>> copyFolders;
		for (auto& data : pathV) {
			// scan each path and add to copy vector
			folders = scan(data, blockSize, folders, specified);
			folders = formatVector(folders, s, n, z, b, r, blockSize);
			// populate copied vector 
			for (auto& n : folders) {
				copyFolders.push_back(n);
			}
			folders.clear();
			
		}
		printVector(copyFolders, h);

		//print copied vector 
	}





}

/**
 * function Name: recursiveFileSearcher
 * Purpose: this function will go to a path and scan everything inside, including subfolders
 * Accepts: a path, two ints that are the cluster size and the file size, a vector of pairs int and string, and a bool for weather it was a specified folder
 * Returns: a vector of pair that holds the file size and file name
 * Date: April 20 2020
 */
vector<pair<int, string>> recursiveFileSearcher(path const& p, int clusterSize, int& fileSize, vector<pair<int, string>> folders, bool specified) {
	int subFileSize = 0;
	for (auto e : recursive_directory_iterator(p, directory_options::skip_permission_denied)) {
		if (is_regular_file(e.path())) {
			subFileSize += (int)(ceil((double)file_size(e.path()) / (double)clusterSize));
		}
	}
	if (specified == true) {
		folders.push_back(make_pair(subFileSize, p.string()));
	}
	else {
		folders.push_back(make_pair(subFileSize, ".\\" + p.filename().string()));
	}
	return folders;

}


/**
 * function Name: scan
 * Purpose: this function will go to a path and scan everything inside
 * Accepts: a path, two ints that are the cluster size and the file size, a vector of pairs int and string, and a bool for weather it was a specified folder
 * Returns: a vector of pair that holds the file size and file name
 * Date: April 20 2020
 */
vector<pair<int, string>> scan(path const& f, int clusterSize, vector<pair<int, string>> folders, bool specified) {
	directory_iterator d(f);	// first entry of folder 'f'
	directory_iterator e;		// virtual match to the 'end' of any folder
	int fileSize = 0;
	int subFileSize = 0;
	while (d != e) {
		if (is_regular_file(d->path())) {
			fileSize += (int)(ceil((double)file_size(d->path()) / (double)clusterSize));
		}
		else {
			folders = recursiveFileSearcher(d->path(), clusterSize, subFileSize, folders, specified);
			fileSize += folders.back().first;
		}
		++d;
	}
	if (specified == true) {
		folders.push_back(make_pair(fileSize, f.string()));
	}
	else {
		folders.push_back(make_pair(fileSize, "."));
	}
	return folders;
}

/**
 * function Name: roundWhole
 * Purpose: this function will round to a whole
 * Accepts: a double to be rounded
 * Returns: a double that is rounded to 
 * Date: April 20 2020
 */
double roundWhole(double n) {
	double d = n * 1.0;
	int i = (int)(d + 0.5);
	d = (float)i / 1.0;
	return d;
}


/**
 * function Name: maxLength
 * Purpose: this function will take a vector and see how many digits its its largest number has
 * Accepts:a vector of pair int and string
 * Returns: an int that is the number of digits in the largest number of the vector
 * Date: April 20 2020
 */
int maxLength(vector<pair<int, string>> folders)
{
	int maxLength = 0;
	int currentLength = 0;
	for (auto i : folders)
	{
		currentLength = to_string(i.first).length();

		if (currentLength > maxLength)
			maxLength = currentLength;
	}
	return maxLength;
}


/**
 * function Name: maxLength
 * Purpose: this function will go to a path and scan everything inside
 * Accepts: a path, two ints that are the cluster size and the file size, a vector of pairs int and string, and a bool for weather it was a specified folder
 * Returns: a vector of pair that holds the file size and file name
 * Date: April 20 2020
 */
string human_format(int bytes) {
	const char* suffixes[7];
	suffixes[0] = "";
	suffixes[1] = "K";
	suffixes[2] = "M";
	suffixes[3] = "G";
	suffixes[4] = "T";
	suffixes[5] = "P";
	suffixes[6] = "E";
	int s = 0; // which suffix to use
	double count = bytes * 4096.0;
	string countStr;
	while (count >= 1024 && s < 7)
	{
		s++;
		count /= 1024;
	}
	if (count < 10 && count > 0) {

		count *= 10;
		count = (int)count;
		count /= 10;
		countStr = to_string(count).substr(0,3) + suffixes[s];
	}
	else {
		countStr = to_string((int)roundWhole(count)) + suffixes[s];
	}
	return countStr;
}

/**
 * function Name: formatVector
 * Purpose: this function will arrage the vector the way the user wants
 * Accepts: a vector of pair int and string, 6 bools representing the switches, and int for blocksize`
 * Returns: a vector of pair that is formated
 * Date: April 20 2020
 */
vector<pair<int, string>> formatVector(vector<pair<int, string>> folders, bool s, bool n, bool z, bool b, bool r, int blockSize) {
	if (s == true) {
		reverse(folders.begin(), folders.end());
		folders.resize(1);
	}
	if (n == true) {
		sort(folders.begin(), folders.end(), [](auto& left, auto& right) {
			return left.second < right.second;
			});
	}
	if (z == true) {
		sort(folders.begin(), folders.end());
	}
	if (r == true) {
		reverse(folders.begin(), folders.end());
	}
	if (b == true) {
		for (auto& data : folders) {
			data.first *= blockSize;
		}
	}
	return folders;
}

/**
 * function Name: printVector
 * Purpose: this function print a vector
 * Accepts: a vector of pair int and string, a bool representing the human switch
 * Returns: void function.
 * Date: April 20 2020
 */
void printVector(vector<pair<int, string>> folders, bool h) {
	

	int length = maxLength(folders);
	
	//cout << left << setw(length) << data.first << setw(3) << ' ' << data.second << endl;
	if (h == true) {
		if (length < 3) {
			length = 3;
		}
		for (auto& data : folders) {
			cout << left << setw(length) << human_format(data.first) << setw(3) << ' ' << data.second << endl;
		}
	}
	else {
		for (auto& data : folders) {
			cout << left << setw(length) << data.first << setw(3) << ' ' << data.second << endl;
		}
	}
}





