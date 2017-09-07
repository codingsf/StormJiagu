#ifndef _HTZIPTOOLS_H
#define _HTZIPTOOLS_H
#include "zip.h"
#include "unzip.h"

const int HT_ZIP_FILE_APPEND = 0;// 如果文件存在追加（压缩）
const int HT_ZIP_FILE_REPLACE = 1; // 如果文件存在替换（压缩解压缩）
const int HT_ZIP_FILE_EXIT = 2;// 如果文件存在退出（压缩）
const int HT_ZIP_FILE_SKIP = 3;// 如果文件存在跳过（解压缩）
const int HT_ZIP_RETURN_FLAG_OK = 0;
const int HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND = -1;
const int HT_ZIP_RETURN_FLAG_FILE_IS_EXIST = -2;
const int HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE = -2;
const int HT_ZIP_RETURN_FLAG_IO_ERROR = -3;
const int HT_ZIP_RETURN_FLAG_ERROR = -4;


/**********************************************************************************
 * 压缩文件或文件夹（包括子目录）
 * directory，待压缩的目录或文件
 * zipfile，输出的文件
 * replaceFlag，如果输出文件存在的处理方式，设置为HT_ZIP_REPLACE将替换原有文件，如果
 *              设置为HT_ZIP_APPEND将在原有的基础上追加新文件，其他为停止压缩操作
 * 返回-1：表示待压缩文件不存在，或文件夹为空
 * 返回-2：表示输出文件已存在，并且设置replaceFlag为HT_ZIP_FILE_EXIT
 * 返回-3：表示文件读写发生错误
 * 返回-4：表示未知错误
 * 返回 0：表示操作成功
 **********************************************************************************/
int ht_zipCompress(char* directory, char* zipfile, int replaceFlag = 0);


/**********************************************************************************
 * 解压缩文件到指定目录
 * zipfile，要解压缩的文件
 * outDirectory，输出目录，如果目录不存在，自动创建
 * replaceFlag，如果输出文件存在的处理方式，设置为HT_ZIP_REPLACE将替换原有文件，如果
 *              设置为HT_ZIP_FILE_SKIP跳过该文件，其他参数皆作为替换
 * 返回-1：表示待解压缩文件不存在 
 * 返回-2：表示输出目录是一个文件
 * 返回-3：表示文件读写发生错误
 * 返回-4：表示未知错误 
 * 返回 0：表示操作成功
 **********************************************************************************/
int ht_zipExtract(char* zipfile, char* outDirectory, int replaceFlag = 1);

// oneFile要提取的文件
// saveFileName 要保存的文件名
int ht_zipExtractOneFile(char* zipfile, char* outDirectory, char* oneFile ,char* saveFileName,int replaceFlag = 1);

#endif