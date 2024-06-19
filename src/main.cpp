#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unordered_set>
#include "fpdfview.h"
#include "fpdf_doc.h"
#include "fpdf_text.h"
#include "fpdf_edit.h"
#include "fpdf_save.h"
#include "turbojpeg.h"

std::unordered_set<FPDF_PAGEOBJECT> used_resource;

int writeBlock(FPDF_FILEWRITE* filewrite, const void* buffer, unsigned long size)
{
	// 打开文件
	FILE* Ofile = fopen("output.pdf", "ab");
	if (!Ofile) {
        // 文件打开失败
        return -1;
    }else{
        printf("open file successful\n");
    }

	// 将缓冲区中的数据写入文件
    size_t bytesWrite = fwrite(buffer, 1, size, Ofile);
    if (bytesWrite != size) {
        // 写入失败
        fclose(Ofile); 
        return -1;
    }
	fclose(Ofile);
	return 1;
}

static int CustomFileRead(void* param, unsigned long position, unsigned char* pBuf, unsigned long size) {
    const char* file_path = static_cast<const char*>(param);
    FILE *fd = fopen(file_path, "rb");
    if(fd == NULL){
        std::cout << "open error";
        return 0;
    }
    if(fseek(fd, position, SEEK_SET) != 0){
        std::cout << "seek error";
        fclose(fd);
        return 0;
    }
    size_t bytesRead = fread(pBuf, 1, size, fd);
    if (bytesRead != size) {
        std::cout << "read error";
        fclose(fd);
        return 0; 
    }
    //std::cout << "successful" << std::strlen((char*)pBuf);
    fclose(fd);
    return 1; 
}

void CompressJpgImage(FPDF_PAGE page, FPDF_PAGEOBJECT obj)
{
	// 获取图像的 Bitmap
    FPDF_BITMAP bitmap = FPDFImageObj_GetBitmap(obj);
    if (!bitmap) {
        std::cerr << "Failed to get bitmap from image object." << std::endl;
        return;
    }

    // 获取图像的原始数据
    int width = FPDFBitmap_GetWidth(bitmap);
    int height = FPDFBitmap_GetHeight(bitmap);
    int stride = FPDFBitmap_GetStride(bitmap);
    void* buffer = FPDFBitmap_GetBuffer(bitmap);
    unsigned long bufferlen = strlen((char*)buffer);
    std::cout << "before compress buffersize:" << std::endl;
    if(bufferlen == 0)
    {
        FPDFPage_RemoveObject(page,obj);
    }
	// 初始化 Turbo-JPEG
    tjhandle tjInstance = tjInitCompress();
    if (!tjInstance) {
        std::cerr << "Failed to initialize Turbo-JPEG." << std::endl;
        return;
    }
	unsigned char* compressedImage = nullptr;
    unsigned long compressedSize = 0;
    int quality = 50; // 压缩质量

    // 使用 Turbo-JPEG 进行压缩
    if (tjCompress2(tjInstance, (unsigned char*)buffer, width, stride, height, TJPF_BGR, &compressedImage, &compressedSize, TJSAMP_422, quality, 0) < 0) {
        std::cerr << "Failed to compress image with Turbo-JPEG: " << tjGetErrorStr() << std::endl;
        tjDestroy(tjInstance);
        return;
    }
	// 更新图片
	FPDF_FILEACCESS fileAccess;
	char filename[] = "saved_file";

	FILE *fd = fopen(filename,"wb");
    if (NULL == fd) {
        return;
    }
    fwrite(compressedImage,1,compressedSize,fd);
    fclose(fd);
	// 
	fileAccess.m_FileLen=compressedSize;
    fileAccess.m_Param=filename;
    fileAccess.m_GetBlock=&CustomFileRead;
    
	FPDFImageObj_LoadJpegFileInline(&page,0,obj,&fileAccess);
	//
	tjDestroy(tjInstance);
    tjFree(compressedImage);

	FPDFBitmap_Destroy(bitmap);
}

void RemovedunusedResource(FPDF_PAGE page, FPDF_PAGEOBJECT obj)
{
	// 删除当前pdf未使用的元素(对象)
	if( false == (FPDFPage_RemoveObject(page, obj)))
	{
		//std::cerr << "remove fail" << std::endl;
		//exit(0);
	}
	else
	{
		//std::cout << "removed unused resource" << std::endl;
	}				
}

bool DeleteReusableElement(FPDF_PAGE page, FPDF_PAGEOBJECT obj)
{
	// 将obj放入集合中
	std::pair<std::unordered_set<FPDF_PAGEOBJECT>::iterator, bool> value = used_resource.insert(obj);
	if(!value.second)
	{
		// 插入失败 说明对象重复 需删除
		RemovedunusedResource(page, obj);
		return true;
	}
	return false;
}

void RemoveunuseCharacter(FPDF_PAGE page, FPDF_PAGEOBJECT obj)
{
    FPDF_TEXTPAGE textPage=FPDFText_LoadPage(page);
    FPDF_WCHAR content_buffer[256];
    unsigned long len=FPDFTextObj_GetText(obj,textPage,content_buffer,100);
    if(FPDFTextObj_GetTextRenderMode(obj)==FPDF_TEXTRENDERMODE_INVISIBLE ||len==2){
        FPDFPage_RemoveObject(page,obj);
    }
}

int main(int argc, char* argv[])
{
    FPDF_InitLibrary();
    std::cout << "PDFium of library can use!" << std::endl;
    // 加载pdf文件
    FPDF_DOCUMENT pdf_doc = FPDF_LoadDocument("test.pdf", NULL);
    if(pdf_doc == NULL)
    {
        std::cerr << "the pdf not exits" << std::endl;
        exit(0);
    }
    // 获取pdf文件的页数
    int page_count = FPDF_GetPageCount(pdf_doc);
    std::cout << "numbers of page = " << page_count << std::endl;
    for(int i = 0; i < page_count; i++)
    {
    	std::cout << "current page = " << i << std::endl;
        // 加载当前页
        FPDF_PAGE page = FPDF_LoadPage(pdf_doc, i); 
        if (!page) {
            continue;
        }
        // 获取当前页的对象数
        int object_count = FPDFPage_CountObjects(page);
		for(int j = 0; j < object_count; j++)
		{
			// 得到当前页的当前对象
			FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, j);
            if (!obj) {
                continue;
            }
            //std::cout << FPDFPageObj_GetObjNum(obj) << std::endl;
            unsigned int R,G,B,A;
            FPDFPageObj_GetFillColor(obj,&R,&G,&B,&A);
			// 利用unordered_set底层哈希表原理删除重复元素
			//if(DeleteReusableElement(page, obj))
				//continue;

			if(FPDF_PAGEOBJ_UNKNOWN == FPDFPageObj_GetType(obj))
			{
				RemovedunusedResource(page, obj);
			}
			else if(FPDF_PAGEOBJ_TEXT == FPDFPageObj_GetType(obj))
			{
                RemoveunuseCharacter(page, obj);
			}
			else if(FPDF_PAGEOBJ_PATH == FPDFPageObj_GetType(obj))
			{
				
			}
			else if(FPDF_PAGEOBJ_IMAGE == FPDFPageObj_GetType(obj))
			{
				std::cout << "begin compress!" << std::endl;
				CompressJpgImage(page, obj);
			}
			else if(FPDF_PAGEOBJ_SHADING == FPDFPageObj_GetType(obj))
			{
				
			}
			else if(FPDF_PAGEOBJ_FORM == FPDFPageObj_GetType(obj))
			{
				
			}
            FPDFPageObj_SetFillColor(obj,R,G,B,A);
		}
        FPDFPage_GenerateContent(page);
		FPDF_ClosePage(page);
    }

	FPDF_FILEWRITE filewrite;
	filewrite.version = 1;
	filewrite.WriteBlock = &writeBlock;
	FPDF_SaveAsCopy(pdf_doc, &filewrite, FPDF_NO_INCREMENTAL);

	FPDF_CloseDocument(pdf_doc);
    FPDF_DestroyLibrary();
    return 0;
}
