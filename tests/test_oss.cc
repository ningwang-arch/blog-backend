#include <fstream>

#include "pico/pico.h"
#include "src/oss/oss_client.h"
#include "src/util.h"

OSSClient ossClient("pico-img", "/home/eclipse/code/vscode/blog/oss.ini");



void test_upload_file() {
    std::string fileName = "CMakeLists.txt";
    std::string filePath = "./CMakeLists.txt";

    if (!ossClient.upload_file(fileName, filePath)) {
        LOG_ERROR("upload file %s failed", filePath.c_str());
        exit(1);
    }
}

void test_upload_data() {
    std::string fileName = "CMakeLists.txt";
    std::string data;
    std::ifstream in(fileName.c_str(), std::ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("open file %s failed", fileName.c_str());
        exit(1);
    }

    in.seekg(0, std::ios::end);
    data.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&data[0], data.size());

    if (!ossClient.upload_data(fileName, data)) {
        LOG_ERROR("upload file %s failed", fileName.c_str());
        exit(1);
    }
}

void test_download_file() {
    std::string fileName = "CMakeLists.txt";
    std::string filePath = "./d.txt";

    if (!ossClient.download_file(fileName, filePath)) {
        LOG_ERROR("download file %s failed", fileName.c_str());
        exit(1);
    }
}

void test_download_data() {
    std::string fileName = ".vimrc";
    std::string data;

    if (!ossClient.download_data(fileName, data)) {
        LOG_ERROR("download file %s failed", fileName.c_str());
        exit(1);
    }

    std::cout << data << std::endl;
}

void test_delete_file() {
    std::string fileName = "CMakeLists.txt";

    if (!ossClient.delete_file(fileName)) {
        LOG_ERROR("delete file %s failed", fileName.c_str());
        exit(1);
    }
}

void test_list_files() {
    std::vector<std::string> files;

    if (!ossClient.list_files("", files)) {
        LOG_ERROR("list files failed");
        exit(1);
    }

    for (auto& file : files) { std::cout << file << std::endl; }
}

void test_get_file_size() {
    std::string fileName = ".vimrc";
    uint64_t size = 0;

    if (!ossClient.getFileSize(fileName, size)) {
        LOG_ERROR("get file size failed");
        exit(1);
    }

    std::cout << size << std::endl;
}


int main(int argc, char const* argv[]) {
    // test_upload_file();
    // test_upload_data();
    // test_download_file();
    // test_download_data();
    // test_delete_file();
    // test_list_files();
    test_get_file_size();
    return 0;
}
