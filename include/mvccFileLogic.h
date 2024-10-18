#ifndef MVCC_FILE_LOGIC_H
#define MVCC_FILE_LOGIC_H

struct AFSFileEntry {
    std::string filename;
    uint32_t size;
    uint32_t offset;
};
uint32_t getFileSize(const std::string& filepath);
void padToBoundary(std::ofstream& outputFile, uint32_t boundary, int shift);
static uint32_t read32le(const std::vector<uint8_t>& data, size_t loc);
static bool saveFile(uint32_t start, uint32_t size, const std::string& dir, const std::string& fname,const std::vector<uint8_t>& afs);

bool f_createArchive(std::string path, std::string target);
bool f_createAFS(std::string path, std::string target, std::vector<AFSFileEntry> files);
bool f_createIBIS(std::string path, std::string target, std::vector<AFSFileEntry> files);
bool f_extractFile(std::string source_file, std::string target);
std::string readBytes(std::ifstream& file, int offset, const int &bytes);
std::string f_getFilename(int number, std::string source);

#endif // MVCC_FILE_LOGIC_H
