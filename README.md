                                                                          **PDFium_compress**

#### 1.Day01

##### 1.1 PDFium简介

PDFium 是一个高效、开源的 PDF 渲染和处理库，它是由 Google 开发和维护的，并被广泛用于 Chromium 和其他应用程序中。PDFium 提供了一整套 API，用于查看、编辑和创建 PDF 文件，使其成为处理 PDF 文件的强大工具。

##### 1.2 PDFium构建

具体操作请看[pdfium.lib buildoperation](https://pdfium.googlesource.com/pdfium/)。

嫌麻烦的可以直接去下载别人构建好的[bblanchon/pdfium-binaries: 📰 Binary distribution of PDFium (github.com)](https://github.com/bblanchon/pdfium-binaries)。

##### 1.3 PDFiumDemo

这里简单介绍PDFium库函数的用法。

```c++
#include <iostream>
#include <string>
#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_edit.h"

int main()
{
	// 在使用PDFium库函数前，必须初始化库
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
    int object_count = 0;
    for(int i = 0; i < page_count; i++)
    {
        // 加载当前页
        FPDF_PAGE page = FPDF_LoadPage(pdf_doc, i);
        // a handle to the text page information structure.
        FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
        int counts = FPDFText_CountChars(text_page);
        for(int j = 0; j < counts; j++)
        {
            unsigned short buffer[2] = {0};
            FPDFText_GetText(text_page, j, 1, buffer);
            std::cout << (wchar_t)buffer[0] << " ";
        }
        std::cout << "next page!" << std::endl; 
        FPDFText_ClosePage(text_page);

        // 获取对象数
        object_count += FPDFPage_CountObjects(page);

    }
    std::cout << "pdf of obj = " << object_count << std::endl;
    FPDF_CloseDocument(pdf_doc);
    FPDF_DestroyLibrary();
    return 0;
}

```

PDFium库参考[API](https://developers.foxit.com/resources/pdf-sdk/c_api_reference_pdfium/index.html)。

#### 2.Day02

##### 2.1 PDF文件物理结构

先看一个简单的只有“hello, world!”内容的PDF文件。

~~~
%PDF-1.0 % Header从这里开始
%âãÏÓ
1 0 obj % Body从这里开始
<<
/Kids [2 0 R]
/Count 1
/Type /Pages
>>
endobj
2 0 obj
<<
25
/Rotate 0
/Parent 1 0 R
/Resources 3 0 R
/MediaBox [0 0 612 792]
/Contents [4 0 R]
/Type /Page
>>
endobj
3 0 obj
<<
/Font
<<
/F0
<<
/BaseFont /Times-Italic
/Subtype /Type1
/Type /Font
>>
>>
>>
endobj
4 0 obj
<<
/Length 65
>>
stream
1. 0. 0. 1. 50. 700. cm
BT
 /F0 36. Tf
 (Hello, World!) Tj
ET
endstream
endobj
5 0 obj
<<
/Pages 1 0 R
/Type /Catalog
>>
endobj
xref % 交叉引用表从这里开始
0 6
0000000000 65535 f
0000000015 00000 n
0000000074 00000 n
0000000192 00000 n
0000000291 00000 n
0000000409 00000 n
trailer % 预览块从这里开始
<<
/Root 5 0 R
/Size 6
>>
startxref
459
%%EOF
~~~

首先根据66行知道交叉引用表的地址，然后从trailer字典的 /Root 5 0 R中知道根节点是间接引用5号对象。

根节点/Root是可以得到页面根节点(页面列表)。 在46行可知页面根节点是间接引用1号对象。

从1号对象[kids]知道每个页面对象例如[2 0 R],这个PDF只有一个页面对象，

而一个页面对象有/Resources名称和/Contents名称，一个指向资源页面对象，如字体等，另外一个指向内容页面对象，其中包含字体和图像。

PDF文件物理结构的核心部分就如上所述，还有一些细节请自行体会。

##### 2.2 读取一个PDF文件

读取一个PDF文件流程简而言之就是：

1. 从文件开头读取PDF文件头，确认是否为PDF文档并检索版本号。
2. 现在通过从末尾向后搜索找到文件结束标记 文件。现在可以读取trailer字典，以及开头的字节偏移量检索交叉引用表。
3. 现在可以读取交叉引用表。我们现在知道每个对象的地址。
4. 在此阶段，可以读取和解析所有对象，或者我们可以离开此过程 直到实际需要每个对象，按需阅读。
5. 我们现在可以使用数据，提取页面，解析图形内容，提取元数据等。

#### 3.Day03

使用libturbojpeg.so库去压缩图片等。
