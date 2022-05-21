#include "oss_client.h"

#include <fstream>
#include <sstream>

#include "pico/pico.h"
#include "src/util.h"

OSSClient::OSSClient(const std::string& bucketName, const std::string& config_file) {
    m_accessId = "";
    m_accessKey = "";
    m_endpoint = "";

    m_bucketName = bucketName;

    std::string cf;

    if (config_file.empty()) {   // use default config file
        cf = "./conf/oss.ini";
    }
    else {
        cf = config_file;
    }


    if (!init(cf)) {
        LOG_ERROR("OSSClient init failed");
        exit(1);
    }
}

bool OSSClient::init(const std::string& config_file) {
    std::string accessId;
    std::string accessKey;
    std::string endpoint;

    std::string line;
    std::ifstream in(config_file.c_str());
    if (!in.is_open()) {
        LOG_ERROR("open config file %s failed", config_file.c_str());
        return false;
    }

    while (getline(in, line)) {
        if (line.empty()) { continue; }

        std::string::size_type pos = line.find("=");
        if (pos == std::string::npos) { continue; }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "AccessKeyId") { accessId = value; }
        else if (key == "AccessKeySecret") {
            accessKey = value;
        }
        else if (key == "Endpoint") {
            endpoint = value;
        }
        else {
        }
    }

    if (accessId.empty() || accessKey.empty() || endpoint.empty()) { return false; }

    m_accessId = accessId;
    m_accessKey = accessKey;
    m_endpoint = endpoint;

    return true;
}

OSSClient::~OSSClient() {}

bool OSSClient::upload_file(const std::string& fileName, const std::string& filePath) {
    std::ifstream in(filePath.c_str(), std::ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("open file %s failed", filePath.c_str());
        return false;
    }

    in.seekg(0, std::ios::end);
    std::string data;
    data.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&data[0], data.size());

    return upload_data(fileName, data);
}

bool OSSClient::upload_data(const std::string& fileName, const std::string& data) {

    std::string url = "http://" + m_bucketName + "." + m_endpoint + "/" + fileName;

    std::map<std::string, std::string> headers;
    headers["Date"] = get_gmt_time();
    headers["Authorization"] =
        get_auth_header("PUT", m_accessId, m_accessKey, "/" + m_bucketName + "/" + fileName);

    pico::HttpResponse::Ptr resp =
        pico::HttpConnection::doRequest(pico::HttpMethod::PUT, url, headers, data);

    if (!resp || (int)resp->get_status() != 200) {
        LOG_ERROR("upload file %s failed", fileName.c_str());
        return false;
    }

    return true;
}

bool OSSClient::download_file(const std::string& fileName, const std::string& filePath) {
    std::string data;
    if (!download_data(fileName, data)) { return false; }

    FILE* fp = fopen(filePath.c_str(), "wb");

    if (!fp) {
        LOG_ERROR("open file %s failed", filePath.c_str());
        return false;
    }

    fwrite(data.c_str(), 1, data.size(), fp);
    fclose(fp);

    return true;
}

bool OSSClient::download_data(const std::string& fileName, std::string& data) {
    std::string url = "http://" + m_bucketName + "." + m_endpoint + "/" + fileName;

    std::map<std::string, std::string> headers;
    headers["Date"] = get_gmt_time();
    headers["Authorization"] =
        get_auth_header("GET", m_accessId, m_accessKey, "/" + m_bucketName + "/" + fileName);

    pico::HttpResponse::Ptr resp = pico::HttpConnection::doGet(url, headers);

    if (!resp || (int)resp->get_status() != 200) {
        LOG_ERROR("download file %s failed", fileName.c_str());
        return false;
    }

    data = resp->get_body();

    return true;
}

bool OSSClient::delete_file(const std::string& fileName) {
    std::string url = "http://" + m_bucketName + "." + m_endpoint + "/" + fileName;

    std::map<std::string, std::string> headers;
    headers["Date"] = get_gmt_time();
    headers["Authorization"] =
        get_auth_header("DELETE", m_accessId, m_accessKey, "/" + m_bucketName + "/" + fileName);

    pico::HttpResponse::Ptr resp =
        pico::HttpConnection::doRequest(pico::HttpMethod::DELETE, url, headers);

    if (!resp || (int)resp->get_status() != 204) {
        LOG_ERROR("delete file %s failed", fileName.c_str());
        return false;
    }

    return true;
}

bool OSSClient::list_files(const std::string& prefix, std::vector<std::string>& fileNames) {
    std::string url = "http://" + m_bucketName + "." + m_endpoint + "/";

    std::map<std::string, std::string> headers;
    headers["Date"] = get_gmt_time();
    headers["Authorization"] =
        get_auth_header("GET", m_accessId, m_accessKey, "/" + m_bucketName + "/");

    pico::HttpResponse::Ptr resp =
        pico::HttpConnection::doRequest(pico::HttpMethod::GET, url, headers);

    if (!resp || (int)resp->get_status() != 200) {
        LOG_ERROR("list files failed");
        return false;
    }

    std::string body = resp->get_body();
    std::stringstream ss(body);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) { continue; }

        std::string::size_type pos = line.find("<Key>");
        if (pos == std::string::npos) { continue; }

        std::string key = line.substr(pos + 5);
        pos = key.find("</Key>");
        if (pos == std::string::npos) { continue; }

        key = key.substr(0, pos);
        if (key.find(prefix) == 0) { fileNames.push_back(key); }
    }

    return true;
}

bool OSSClient::getFileSize(const std::string& fileName, uint64_t& size) {
    std::string url = "http://" + m_bucketName + "." + m_endpoint + "/" + fileName;

    std::map<std::string, std::string> headers;
    headers["Date"] = get_gmt_time();
    headers["Authorization"] =
        get_auth_header("HEAD", m_accessId, m_accessKey, "/" + m_bucketName + "/" + fileName);

    pico::HttpResponse::Ptr resp =
        pico::HttpConnection::doRequest(pico::HttpMethod::HEAD, url, headers);

    if (!resp || (int)resp->get_status() != 200) {
        LOG_ERROR("get file size failed");
        return false;
    }

    std::string sizeStr = resp->get_header("Content-Length");
    if (sizeStr.empty()) {
        LOG_ERROR("get file size failed");
        return false;
    }

    size = std::stoull(sizeStr);

    return true;
}

bool OSSClient::getBucketName(std::string& bucketName) {
    bucketName = m_bucketName;
    return true;
}