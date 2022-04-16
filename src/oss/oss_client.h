#ifndef __OSS_OSS_H__
#define __OSS_OSS_H__

#include <string>
#include <vector>

#include "pico/logging.h"

class OSSClient
{
public:
    OSSClient(const std::string& bucketName, const std::string& config_file = "oss.ini");
    ~OSSClient();

    bool upload_file(const std::string& fileName, const std::string& filePath);
    bool upload_data(const std::string& fileName, const std::string& data);
    bool download_file(const std::string& fileName, const std::string& filePath);
    bool download_data(const std::string& fileName, std::string& data);
    bool delete_file(const std::string& fileName);
    bool list_files(const std::string& prefix, std::vector<std::string>& fileNames);

    bool getFileSize(const std::string& fileName, uint64_t& fileSize);

    bool getBucketName(std::string& bucketName);

private:
    bool init(const std::string& config_file);


private:
    std::string m_accessId;
    std::string m_accessKey;
    std::string m_endpoint;

    std::string m_bucketName;
};

#endif