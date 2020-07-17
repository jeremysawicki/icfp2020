#include "FileUtils.hpp"

using std::string;
using std::vector;

bool getFileSize(FILE* f, size_t* pSize)
{
    long pos = ftell(f);
    if (pos < 0)
    {
        return false;
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        return false;
    }
    long size = ftell(f);
    if (size < 0)
    {
        return false;
    }
    if (fseek(f, pos, SEEK_SET) != 0)
    {
        return false;
    }

    if (pSize)
    {
        *pSize = (size_t)size;
    }
    return true;
}

bool getFileSize(const string& fileName, size_t* pSize)
{
    FILE* f = fopen(fileName.c_str(), "rb");
    if (!f) return false;
    bool result = getFileSize(f, pSize);
    fclose(f);
    return result;
}

namespace
{
    template<class Data>
    bool readFileImpl(FILE* f, Data* pData)
    {
        size_t size = 0;
        if (getFileSize(f, &size))
        {
            pData->resize(size);
            size_t pos = 0;
            while (pos < size)
            {
                if (feof(f) || ferror(f))
                {
                    pData->resize(0);
                    return false;
                }
                size_t bytesRead = fread(&(*pData)[pos], 1, size - pos, f);
                if (bytesRead <= 0)
                {
                    pData->resize(0);
                    return false;
                }
                pos += bytesRead;
            }
        }
        else
        {
            char buf[1024];
            size_t pos = 0;
            while (true)
            {
                if (feof(f))
                {
                    break;
                }
                if (ferror(f))
                {
                    pData->resize(0);
                    return false;
                }
                size_t bytesRead = fread(buf, 1, 1024, f);
                if (bytesRead <= 0)
                {
                    break;
                }
                pData->resize(pos + bytesRead);
                memcpy(&(*pData)[pos], buf, bytesRead);
                pos += bytesRead;
            }
        }
        return true;
    }
}

bool readFile(FILE* f, vector<uint8_t>* pData)
{
    return readFileImpl(f, pData);
}

bool readFile(FILE* f, vector<char>* pData)
{
    return readFileImpl(f, pData);
}

bool readFile(FILE* f, string* pData)
{
    return readFileImpl(f, pData);
}

bool readFile(const string& fileName, vector<uint8_t>* pData)
{
    FILE* f = fopen(fileName.c_str(), "rb");
    if (!f) return false;
    bool result = readFile(f, pData);
    fclose(f);
    return result;
}

bool readFile(const string& fileName, vector<char>* pData)
{
    FILE* f = fopen(fileName.c_str(), "rb");
    if (!f) return false;
    bool result = readFile(f, pData);
    fclose(f);
    return result;
}

bool readFile(const string& fileName, string* pData)
{
    FILE* f = fopen(fileName.c_str(), "rb");
    if (!f) return false;
    bool result = readFile(f, pData);
    fclose(f);
    return result;
}

namespace
{
    bool writeFileImpl(FILE* f, const char* data, size_t size)
    {
        if (size > 0)
        {
            if (fwrite(data, 1, size, f) != size)
            {
                return false;
            }
        }
        return true;
    }
}

bool writeFile(FILE* f, const vector<uint8_t>& data)
{
    return writeFileImpl(f, (const char*)data.data(), data.size());
}

bool writeFile(FILE* f, const vector<char>& data)
{
    return writeFileImpl(f, data.data(), data.size());
}

bool writeFile(FILE* f, const string& data)
{
    return writeFileImpl(f, data.data(), data.size());
}

bool writeFile(const string& fileName, const vector<uint8_t>& data)
{
    FILE* f = fopen(fileName.c_str(), "wb");
    if (!f) return false;
    bool result = writeFile(f, data);
    fclose(f);
    return result;
}

bool writeFile(const string& fileName, const vector<char>& data)
{
    FILE* f = fopen(fileName.c_str(), "wb");
    if (!f) return false;
    bool result = writeFile(f, data);
    fclose(f);
    return result;
}

bool writeFile(const string& fileName, const string& data)
{
    FILE* f = fopen(fileName.c_str(), "wb");
    if (!f) return false;
    bool result = writeFile(f, data);
    fclose(f);
    return result;
}

namespace
{
    void getLinesImpl(const char* data, size_t size, vector<string>* pLines)
    {
        vector<string>& lines = *pLines;

        bool gotCR = false;
        bool newLine = true;

        for (size_t i = 0; i < size; i++)
        {
            char ch = data[i];

            if (gotCR && ch == '\n')
            {
                gotCR = false;
                continue;
            }

            if (newLine)
            {
                lines.resize(lines.size() + 1);
                newLine = false;
            }

            string& line = lines.back();

            switch (ch)
            {
            case '\r':
                gotCR = true;
                newLine = true;
                break;
            case '\n':
                newLine = true;
                break;
            default:
                line.push_back(ch);
            }
        }
    }
}

void getLines(const vector<uint8_t>& data, vector<string>* pLines)
{
    getLinesImpl((const char*)data.data(), data.size(), pLines);
}

void getLines(const vector<char>& data, vector<string>* pLines)
{
    getLinesImpl(data.data(), data.size(), pLines);
}

void getLines(const string& data, vector<string>* pLines)
{
    getLinesImpl(data.data(), data.size(), pLines);
}

bool readLines(FILE* f, vector<string>* pLines)
{
    vector<string>& lines = *pLines;
    lines.clear();

    bool gotCR = false;
    bool newLine = true;

    char buf[1024];
    while (true)
    {
        if (feof(f))
        {
            break;
        }
        if (ferror(f))
        {
            lines.clear();
            return false;
        }
        size_t bytesRead = fread(buf, 1, 1024, f);
        if (bytesRead <= 0)
        {
            break;
        }
        for (size_t i = 0; i < bytesRead; i++)
        {
            char ch = buf[i];

            if (gotCR && ch == '\n')
            {
                gotCR = false;
                continue;
            }

            if (newLine)
            {
                lines.resize(lines.size() + 1);
                newLine = false;
            }

            string& line = lines.back();

            switch (ch)
            {
            case '\r':
                gotCR = true;
                newLine = true;
                break;
            case '\n':
                newLine = true;
                break;
            default:
                line.push_back(ch);
            }
        }
    }
    return true;
}

bool readLines(const string& fileName, vector<string>* pLines)
{
    FILE* f = fopen(fileName.c_str(), "rb");
    if (!f) return false;
    readLines(f, pLines);
    fclose(f);
    return true;
}
