#include "stdafx.h"
#include "htziptools.h"
#include "direct.h"
#include <vector>
struct HT_ZIP_FILEINF{
	CString filePath; // 文件全路径
	CString fileName; // 文件相对路径
	HT_ZIP_FILEINF(){
		filePath = "";
		fileName = "";
	}
};
typedef std::vector<struct HT_ZIP_FILEINF> __FILEINFOS;

void pushToFileInfo(CString filePath, CString fileName, __FILEINFOS& infos){
	HT_ZIP_FILEINF info;
	info.fileName = fileName;
	info.filePath = filePath;
	infos.push_back(info);
}

void getAllFiles(CString directory, __FILEINFOS& infos){
	CFileFind _find;
	if (directory.Right(1) != "\\"){
		directory += "\\";
	}
	directory += "*.*";
	int res = _find.FindFile(directory);
	while(res){
		res = _find.FindNextFile();
		if (_find.IsDirectory()){
			if (!_find.IsDots()){
				getAllFiles(_find.GetFilePath(), infos);
			}
		}else{
			pushToFileInfo(_find.GetFilePath(), _find.GetFileName(), infos);
		}
	}
	_find.Close();
}

__FILEINFOS getFiles(char* directory){
	__FILEINFOS infos;
	CFileFind _find;
	// 判断该路径指向的是否是文件
	int res = _find.FindFile(directory);
	if (res){
		_find.FindNextFile();
		if (_find.IsDirectory()){
			if (!_find.IsDots()){
				// 递归
				getAllFiles(_find.GetFilePath(), infos);
			}
			// 遍历目录
		}else{
			pushToFileInfo(_find.GetFilePath(), _find.GetFileName(), infos);
		}
	}else{
		throw "文件没有找到";
	}
	_find.Close();
	return infos;
}

int fileExist(CString filePath){
	int ret = 0;
	if (filePath.Right(1) == "\\"){
		filePath = filePath.Left(filePath.GetLength() - 1);
	}
	CFileFind _find;
	ret = _find.FindFile(filePath);
	_find.Close();
	return ret;
}

int isDirectory(CString path){
	CFileFind _find;
	int ret = 1;
	if (_find.FindFile(path)){
		_find.FindNextFile();
		ret = _find.IsDirectory();
	}
	return ret;
}

long getFileLastAccessTime(CString filePath){
	long result = 0;
	CFileFind _find;
	if (_find.FindFile(filePath)){
		_find.FindNextFile();
		::FILETIME time;
		_find.GetLastAccessTime(&time);
		result = time.dwHighDateTime;
	}
	_find.Close();
	return result;
}

#define BUF_LEN 16384
#define MAX__PATH 1024 // 默认文件名长度

int compress(zipFile& file, CString showPath, CString path){
	char in[BUF_LEN];
	unsigned int readLen;

	int ret = HT_ZIP_RETURN_FLAG_OK;

	zip_fileinfo fileinfo;
	fileinfo.dosDate = getFileLastAccessTime(path);
	zipOpenNewFileInZip64(file, showPath, &fileinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 1);

	FILE* _file = fopen(path, "rb");
	do{
		readLen = fread(in, 1, BUF_LEN, _file);
		if (ferror(_file)){
			// 关闭文件
			zipCloseFileInZip(file);
			ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
			break;
		}
		// 写文件
		ret = zipWriteInFileInZip(file, in, readLen);
		TRACE("%d", ret);
		if (ret != Z_OK){
			zipCloseFileInZip(file);
			ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
			break;
		}
		zipCloseFileInZip(file);
	}while(!feof(_file));
	fclose(_file);
	return ret;
}

int ht_zipCompress(char* directory, char* zipfile, int replaceFlag){
	int ret = HT_ZIP_RETURN_FLAG_OK;	
	try{
		__FILEINFOS infos = getFiles(directory); // 获取要压缩的文件列表
		CFile file;
		int openMode = APPEND_STATUS_CREATE;
		if (fileExist(zipfile)){
			if (replaceFlag = HT_ZIP_FILE_REPLACE){
				file.Remove(zipfile);
			}else if(replaceFlag = HT_ZIP_FILE_APPEND){
				openMode = APPEND_STATUS_ADDINZIP;
			}else{
				return HT_ZIP_RETURN_FLAG_FILE_IS_EXIST; // 文件存在，并且不允许替换或追加
			}
		}
		
		if(infos.size() <= 0){
			return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
		}
		// 打开
		zipFile _zipfile = zipOpen64(zipfile, openMode);
		for (int i = 0; i < infos.size(); i ++){
			CString showfilePath = infos[i].filePath;
			CString filePath = infos[i].filePath;
			// 从路径中移除目标路径
			showfilePath.Replace(directory, "");
			// 如果显示文件名是\起始，则去掉
			if (showfilePath.Left(1) == "\\"){
				showfilePath = showfilePath.Right(showfilePath.GetLength() - 1);
			}
			showfilePath.Replace("\\", "/");
			ret = compress(_zipfile, showfilePath, filePath) ;
			if (ret != HT_ZIP_RETURN_FLAG_OK){
				zipClose(_zipfile, NULL);
				// 移除已经压缩的文件，并结束压缩操作
				file.Remove(zipfile);
				return ret;
			}
		}
		zipClose(_zipfile, NULL);
	}catch(...){
		ret = HT_ZIP_RETURN_FLAG_ERROR;
	}

	return ret;
}

bool __makeDirs(CString path){
	bool ret = true;
	// 循环创建文件夹
	int pos = path.Find("\\");
	if (pos > 0){
		do{		
			CString subFullPath = path.Mid(0, pos + 1);
			// 判断文件是否存在，如果存在判断是否是文件夹
			if (subFullPath.Right(2) != ":\\"){// 非驱动根目录
				if (fileExist(subFullPath)){
					if (!isDirectory(subFullPath)){// 指定子目录是文件
						ret = false;
						break;
					}
					// 文件夹存在
				}else{
					// 创建文件夹
					if (!CreateDirectory(subFullPath,NULL)){
						ret = false;
						break;
					}
				}
			}
			/// 进入文件夹
			pos = path.Find("\\", pos + 1);
		}while(pos > 0);
	}
	return ret;
}

/*
const int HT_ZIP_RETURN_FLAG_OK = 0;
const int HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND = -1;
const int HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE = -2;
const int HT_ZIP_RETURN_FLAG_IO_ERROR = -3;
const int HT_ZIP_RETURN_FLAG_ERROR = -4;
*/
int ht_zipExtract(char* zipfile, char* outDirectory, int replaceFlag){
	CFile cfile;

	int ret = HT_ZIP_RETURN_FLAG_OK;
	if (!fileExist(zipfile)){
		return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
	}
	CString u_dir;
	u_dir.Format("%s", outDirectory);
	if(u_dir.Right(1) != "\\"){
		u_dir += "\\";
	}
	if (fileExist(outDirectory)){
		if (!isDirectory(outDirectory)){
			// 输出目录为文件。
			return HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE;
		}
	}else{
		// 创建文件夹
		if (!__makeDirs(u_dir)){
			return HT_ZIP_RETURN_FLAG_IO_ERROR;
			// 创建文件夹失败！
		}
	}
	try{
		unzFile unzfile = unzOpen64(zipfile); // 打开文件
		char in[BUF_LEN]; // 文件读取缓冲区
		unsigned int readLen; // 读入文件长度
		if(unzfile != NULL){
			unz_global_info64 unzInfo;
			if (unzGetGlobalInfo64(unzfile, &unzInfo) == UNZ_OK){				
				// 分步解压缩文件
				for(int i = 0, j = unzInfo.number_entry; i < j && ret == HT_ZIP_RETURN_FLAG_OK; i ++){
					char szFilePathA[MAX_PATH];
					unz_file_info64 unzfileinfo;
					unzGetCurrentFileInfo64(unzfile, &unzfileinfo, szFilePathA, sizeof(szFilePathA), NULL, 0, NULL, 0); // 获取文件
					// 修改文件路径信息
					CString outFilePath;
					outFilePath.Format("%s%s", u_dir, szFilePathA);
					TRACE(outFilePath);
					outFilePath.Replace("/", "\\");
					if (!__makeDirs(outFilePath)){
						ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
						break;
					}
					// 判断文件是否存在，如果存在，根据设置状态进行处理
					if (fileExist(outFilePath)){
						if (replaceFlag == HT_ZIP_FILE_SKIP){
							unzGoToNextFile(unzfile);
							continue;
						}
						cfile.Remove(outFilePath); // 替换，先删除
					}
					// 读取压缩文件块
					unzOpenCurrentFile(unzfile);
					FILE* wf = fopen(outFilePath, "wb");
					while(1){
						readLen = unzReadCurrentFile(unzfile, in, BUF_LEN);
						if (readLen < 0){
							ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
							// 读文件错误
							break;
						}
						if (readLen == 0){
							break;
						}
						fwrite(in, 1, readLen, wf);
						if (ferror(wf)){
							ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
							break; // 写文件错误
						}
					}
					fclose(wf);
					unzCloseCurrentFile(unzfile);
					unzGoToNextFile(unzfile);
				}
			}else{
				ret = HT_ZIP_RETURN_FLAG_IO_ERROR; // 读取压缩文件头信息失败！
			}
			unzClose(unzfile);
		}else{
			ret = HT_ZIP_RETURN_FLAG_IO_ERROR;// 打开压缩文件失败
		}
		
	}catch(...){
		ret = HT_ZIP_RETURN_FLAG_ERROR;
	}
	return ret;
}


// oneFile:要提取的文件
// saveFileName:要保存的文件名
int ht_zipExtractOneFile(char* zipfile, char* outDirectory, char* oneFile,char* saveFileName,int replaceFlag){
	CFile	cfile;
	int ret = HT_ZIP_RETURN_FLAG_OK;
	if (!fileExist(zipfile)){
		return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
	}
	CString u_dir;
	u_dir.Format("%s", outDirectory);
	if(u_dir.Right(1) != "\\"){
		u_dir += "\\";
	}
	if (fileExist(outDirectory)){
		if (!isDirectory(outDirectory)){
			// 输出目录为文件。
			return HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE;
		}
	}else{
		// 创建文件夹
		if (!__makeDirs(u_dir)){
			return HT_ZIP_RETURN_FLAG_IO_ERROR;
			// 创建文件夹失败！
		}
	}
	try{
		unzFile unzfile = unzOpen64(zipfile); // 打开文件
		char in[BUF_LEN]; // 文件读取缓冲区
		unsigned int readLen; // 读入文件长度
		if(unzfile != NULL){
			unz_global_info64 unzInfo;
			if (unzGetGlobalInfo64(unzfile, &unzInfo) == UNZ_OK){				
				// 分步解压缩文件
				for(int i = 0, j = unzInfo.number_entry; i < j && ret == HT_ZIP_RETURN_FLAG_OK; i ++){
					char szFilePathA[MAX_PATH];
					unz_file_info64 unzfileinfo;
					unzGetCurrentFileInfo64(unzfile, &unzfileinfo, szFilePathA, sizeof(szFilePathA), NULL, 0, NULL, 0); // 获取文件

					if(strncmp(szFilePathA,oneFile,strlen(oneFile))==0)
					{
						OutputDebugString("Find classes.dex");
						// 修改文件路径信息
						CString outFilePath;
						outFilePath.Format("%s%s", u_dir, saveFileName);
						TRACE(outFilePath);
						outFilePath.Replace("/", "\\");
						if (!__makeDirs(outFilePath)){
							ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
							break;
						}
						// 判断文件是否存在，如果存在，根据设置状态进行处理
						if (fileExist(outFilePath)){
							if (replaceFlag == HT_ZIP_FILE_SKIP){
								unzGoToNextFile(unzfile);
								continue;
							}
							cfile.Remove(outFilePath); // 替换，先删除
						}
						// 读取压缩文件块
						unzOpenCurrentFile(unzfile);
						FILE* wf = fopen(outFilePath, "wb");
						while(1){
							readLen = unzReadCurrentFile(unzfile, in, BUF_LEN);
							if (readLen < 0){
								ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
								// 读文件错误
								break;
							}
							if (readLen == 0){
								break;
							}
							fwrite(in, 1, readLen, wf);
							if (ferror(wf)){
								ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
								break; // 写文件错误
							}
						}
						fclose(wf);
						unzCloseCurrentFile(unzfile);
						unzGoToNextFile(unzfile);
						break;
					}
					unzGoToNextFile(unzfile);
				}//for

			}else{
				ret = HT_ZIP_RETURN_FLAG_IO_ERROR; // 读取压缩文件头信息失败！
			}
			unzClose(unzfile);
		}else{
			ret = HT_ZIP_RETURN_FLAG_IO_ERROR;// 打开压缩文件失败
		}
		
	}catch(...){
		ret = HT_ZIP_RETURN_FLAG_ERROR;
	}
	return ret;
}
