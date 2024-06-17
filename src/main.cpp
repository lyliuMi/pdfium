#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unordered_set>
#include "fpdfview.h"
#include "fpdf_doc.h"
#include "fpdf_text.h"
#include "fpdf_edit.h"
#include "fpdf_save.h"

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

void RemovedunusedResource(FPDF_PAGE page, FPDF_PAGEOBJECT obj)
{
	// 删除当前pdf未使用的元素(对象)
	if( false == (FPDFPage_RemoveObject(page, obj)))
	{
		std::cerr << "remove fail" << std::endl;
		//exit(0);
	}
	else
	{
		std::cout << "removed unused resource" << std::endl;
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
        // 获取当前页的对象数
        int object_count = FPDFPage_CountObjects(page);
		for(int j = 0; j < object_count; j++)
		{
    		std::cout << "current obj = " << j << std::endl;
			// 得到当前页的当前对象
			FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, j);
			// 利用unordered_set底层哈希表原理删除重复元素
			if(DeleteReusableElement(page, obj))
				continue;

			if(FPDF_PAGEOBJ_UNKNOWN == FPDFPageObj_GetType(obj))
			{
				RemovedunusedResource(page, obj);
			}
			else if(FPDF_PAGEOBJ_TEXT == FPDFPageObj_GetType(obj))
			{

			}
			else if(FPDF_PAGEOBJ_PATH == FPDFPageObj_GetType(obj))
			{
				
			}
			else if(FPDF_PAGEOBJ_IMAGE == FPDFPageObj_GetType(obj))
			{
				
			}
			else if(FPDF_PAGEOBJ_SHADING == FPDFPageObj_GetType(obj))
			{
				
			}
			else if(FPDF_PAGEOBJ_FORM == FPDFPageObj_GetType(obj))
			{
				
			}
		}
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
