#include <iostream>
#include <map>
#include <ctime>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "md5.h"
#include "base64.h"

using namespace std;

string URL = "openapi.xfyun.cn";
string APPID = "";
string API_KEY = "";
string AUE = "raw";
string AUTH_ID = "2894c985bf8b1111c6728db79d3479ae";
string DATA_TYPE = "text";
string SAMPLE_RATE = "16000";
string SCENE = "main_box";
string RESULT_LEVEL = "complete";
string LAT = "39.938838";
string LNG = "116.368624";
string PERS_PARAM = "{\\\"auth_id\\\":\\\"2894c985bf8b1111c6728db79d3479ae\\\"}";
string FILE_PATH = "test.txt";

static map<string, string>* buildHeader()
{
    time_t t_curTime = time(NULL);
    string curTime = to_string(t_curTime);

    string param = "{\"result_level\":\"" + RESULT_LEVEL + "\",\"auth_id\":\"" + AUTH_ID + "\",\"data_type\":\"" + DATA_TYPE + "\",\"sample_rate\":\"" + SAMPLE_RATE + "\",\"scene\":\"" + SCENE + "\",\"lat\":\"" + LAT + "\",\"lng\":\"" + LNG + "\"}";
    string paramBase64 = base64_encode(reinterpret_cast<const unsigned char*>(param.c_str()), param.length());;
    string de_paramBase64 = base64_decode(paramBase64);
    cout << "paramBase64: " << paramBase64 << endl;
    cout << "de_paramBase64: " << de_paramBase64 << endl;

    MD5 md5;
    string checkSum;
    checkSum += API_KEY;
    checkSum += curTime;
    checkSum += paramBase64;
    // sprintf(checkSum, "%s%s%s", API_KEY, curTime, paramBase64);
    // string checkSum = md5(API_KEY + curTime + paramBase64);
    md5.update(checkSum);
    checkSum = md5.toString();
    cout << "checkSum: " << checkSum << endl;

    map<string,string> *header = new map<string,string>;
    header->insert(make_pair("X-CurTime", curTime));
    header->insert(make_pair("X-Param", paramBase64));
    header->insert(make_pair("X-Appid", APPID));
    header->insert(make_pair("X-CheckSum", checkSum));

    // header["X-CurTime"] = curTime;
    return header;
}

// static void readFile(string filePath, string& rData)
// {
//     char szData[1024];
//     memset(szData, 0, sizeof(szData));
//     ifstream infile;
//     infile.open(filePath, ios::binary | ios::in);
//     if(!infile.is_open())
//         cout << "Open file failure" << endl;
//     cout << "Reading from the file" << endl;
//     while(!infile.eof())
//     {
//         // infile >> szData;
//         infile.getline(szData, sizeof(szData));
//     }
//     cout << szData << endl;
//     rData = szData;
//     infile.close();
// }

// static char* readFile(string filePath)
// {
//     char szData[1024];
//     memset(szData, 0, sizeof(szData));
//     ifstream infile;
//     infile.open(filePath, ios::binary | ios::in);
//     if(!infile.is_open())
//         cout << "Open file failure" << endl;
//     cout << "Reading from the file" << endl;
//     while(!infile.eof())
//     {
//         // infile >> szData;
//         infile.getline(szData, sizeof(szData));
//     }
//     cout << szData << endl;
//     char *pDate = new char[sizeof(szData)];
//     strcpy(pDate, szData);
//     infile.close();
//     return pDate;
// }

static unsigned char* readFile(string filePath)
{
    unsigned char byteData[2048];
    memset(byteData, 0, sizeof(byteData));
    ifstream infile;
    infile.open(filePath, ios::binary | ios::in);
    if(!infile.is_open())
        cout << "Open file failure" << endl;
    cout << "Reading from the file" << endl;
    while(!infile.eof())
    {
        infile >> byteData;
    }
    cout << byteData << endl;
    // reinterpret_cast<const char*>(byteData)
    unsigned char *pDate = new unsigned char[sizeof(byteData)];
    // unsigned char *pDate = (unsigned char *)calloc(sizeof(byteData),1);
    // cout << "sizeof(byteData): " << strlen(reinterpret_cast<const char*>(byteData)) << endl;
    memcpy(pDate, byteData, sizeof(byteData));
    infile.close();
    return pDate;
}

static int httpSocket(string URL, string query)
{
    char szWrBuf[4096];
    char szRdBuf[40960];
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in addr;
    char *ip = NULL;
    const char* pURL = URL.c_str();
    // cout << "pURL: " << pURL << endl;
    
    struct hostent* host = gethostbyname(pURL);
    if(!host)
    {
        perror("gethostbyname");
        return -1;
    }
    ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
    cout << "ip: " << ip << endl;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd){
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, ip, (void *)&addr.sin_addr.s_addr); //inet_addr()

    if(connect(fd, (struct sockaddr *)&addr, len)){
        perror("connect");
        return -1;
    }

    memset(szWrBuf, 0, sizeof(szWrBuf));
    memcpy(szWrBuf, query.c_str(), query.length()+1);

    int nRetWr = write(fd, szWrBuf, strlen(szWrBuf)+1);
    if(-1 == nRetWr){
        perror("write");
        return -1;
    }    
    printf("send_content: %s\n", szWrBuf);
  
    memset(szRdBuf, 0, sizeof(szRdBuf));

    int nRecvLen = sizeof(szRdBuf);
    char* pRdBuf = (reinterpret_cast<char*>(&szRdBuf));
    while(nRecvLen > 0)
    {
        int nRetRd = read(fd, pRdBuf, 4096);
        if(-1 == nRetRd){
            perror("read");
            return -1;
        }
        if(0 == nRetRd){
            cout << "client has closed the connection" << endl;
            break;
        }
        nRecvLen -= nRetRd;
        pRdBuf += nRetRd;
        // printf("receive_content: %s\n" , szRdBuf);
    }
    
    printf("receive_content: %s\n", szRdBuf); 
    close(fd);

    return 0;
}

static int httpPost(string URL, map<string, string>* header, unsigned char *data)
{
    stringstream streambuf;
    streambuf << "POST /v2/aiui HTTP/1.1\r\n";
    streambuf << "HOST: " << URL << "\r\n";
    for(map<string,string>::iterator iter = header->begin(); iter != header->end(); ++iter)
    {
        string key = iter->first;
        string value = iter->second;
        // cout << "key: " << key << endl;
        // cout << "value: " << value << endl;
        streambuf << key << ": " << value << "\r\n";
    }
    streambuf << "User-Agent: Mozilla/5.0\r\n";
    streambuf << "Content-Type: application/x-www-form-urlencoded\r\n";
    streambuf << "Content-Length: " << strlen(reinterpret_cast<const char*>(data)) << "\r\n"; //
    streambuf << "Connection: keep-alive";
    streambuf << "\r\n\r\n";
    streambuf << data;
    // cout << "streambuf: " << streambuf.str() << endl;
    // cout << "data length: " << sizeof(data) << endl;

    int nRet = httpSocket(URL, streambuf.str());
    streambuf.clear();

    return nRet;
}

void printMD5(const string& str, MD5& md5)
{
    cout << "MD5(" << str << ") = " << md5.toString() << endl;
}

int main(void)
{
    unsigned char *data;
    data = readFile(FILE_PATH);
    cout << data << endl;

    map<string, string>* header = buildHeader();

    if(-1 == httpPost(URL, header, data))
        cout << "post failed" << endl;

#if 0
    // map<string,string>::iterator iter = header->find("X-CurTime");
    // cout << "key:value " << iter->first << ":" << iter->second << endl;
    // for(map<string,string>::iterator iter = header->begin(); iter != header->end(); ++iter)
    // {
    //     string key = iter->first;
    //     string value = iter->second;
    //     cout << "key: " << key << endl;
    //     cout << "value: " << value << endl;
    // }

    const char* pURL = URL.c_str();
    cout << "pURL: " << pURL << endl;
    char *ip = NULL;
    struct hostent* host = gethostbyname(pURL);
    if(!host)
    {
        perror("gethostbyname");
        return -1;
    }
    for(int i=0; host->h_addr_list[i]; i++){
        printf("IP addr %d: %s\n", i+1, inet_ntoa( *(struct in_addr*)host->h_addr_list[i] ) );
    }
    ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
    cout << "ip: " << ip << endl;

    // MD5 md5;
    // md5.update("abc");
    // printMD5("abc", md5);

    // string text = "天津天气状况";
    // string text_base64_en = base64_encode(reinterpret_cast<const unsigned char*>(text.c_str()), text.length());
    // string text_base64_de = base64_decode(text_base64_en);
    // cout << text_base64_en << endl;
    // cout << text_base64_de << endl;

    // time_t curTime = time(NULL);
    // string sCurTime = to_string(curTime);
    // cout << "sCurTime: " << sCurTime << endl;
#endif

    delete header;
    delete data;
    // free(data);

    return 0;
}