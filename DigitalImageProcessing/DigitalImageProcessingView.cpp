
// DigitalImageProcessingView.cpp: CDigitalImageProcessingView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "DigitalImageProcessing.h"
#endif

#include "DigitalImageProcessingDoc.h"
#include "DigitalImageProcessingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDigitalImageProcessingView

IMPLEMENT_DYNCREATE(CDigitalImageProcessingView, CView)

BEGIN_MESSAGE_MAP(CDigitalImageProcessingView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_IMG_LOAD_BMP, &CDigitalImageProcessingView::OnImgLoadBmp)
END_MESSAGE_MAP()

// CDigitalImageProcessingView 생성/소멸

CDigitalImageProcessingView::CDigitalImageProcessingView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	rgbBuffer = nullptr;

}

CDigitalImageProcessingView::~CDigitalImageProcessingView()
{
	if (rgbBuffer != nullptr) {
		for (int i = 0; i < imgHeight; i++)
			delete[] rgbBuffer[i];
		delete[] rgbBuffer;
	}
}

BOOL CDigitalImageProcessingView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CDigitalImageProcessingView 그리기

void CDigitalImageProcessingView::OnDraw(CDC* pDC)
{
	CDigitalImageProcessingDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	if (rgbBuffer != NULL) { //이미지가 들어있는 경우에 출력
		for (int i = 0; i < imgHeight; i++) { //이미지 사이즈 만큼 돌면서 한 픽셀씩 출력
			for (int j = 0; j < imgWidth; j++) {
				POINT p; //출력할 픽셀의 위치 지정
				p.x = j;
				p.y = i;
				pDC->SetPixel(p, RGB(rgbBuffer[i][j].rgbRed, rgbBuffer[i][j].rgbGreen, rgbBuffer[i][j].rgbBlue));
				//픽셀 위치 p에 RGB 값을 출력
			}
		}
	}
}


// CDigitalImageProcessingView 인쇄

BOOL CDigitalImageProcessingView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CDigitalImageProcessingView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CDigitalImageProcessingView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CDigitalImageProcessingView 진단

#ifdef _DEBUG
void CDigitalImageProcessingView::AssertValid() const
{
	CView::AssertValid();
}

void CDigitalImageProcessingView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDigitalImageProcessingDoc* CDigitalImageProcessingView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDigitalImageProcessingDoc)));
	return (CDigitalImageProcessingDoc*)m_pDocument;
}
#endif //_DEBUG


// CDigitalImageProcessingView 메시지 처리기


void CDigitalImageProcessingView::OnImgLoadBmp()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	OnBmpLoad();
}


void CDigitalImageProcessingView::OnBmpLoad()
{
	// TODO: 여기에 구현 코드 추가.
	//1. 파일 다이얼로그로부터 BMP 파일 입력
	CFileDialog dlg(TRUE, ".bmp", NULL, NULL, "Bitmap File (*.bmp)|*.bmp||");
	if (IDOK != dlg.DoModal())
		return;
	CString filename = dlg.GetPathName();
	if (rgbBuffer != NULL) { //이미 할당된 경우, 메모리 해제
		for (int i = 0; i < imgHeight; i++)
			delete[] rgbBuffer[i];
		delete[] rgbBuffer;
	}

	//2. 파일을 오픈하여 영상 정보 획득
	CFile file;
	file.Open(filename, CFile::modeRead);
	file.Read(&bmpHeader, sizeof(BITMAPFILEHEADER));
	file.Read(&bmpInfo, sizeof(BITMAPINFOHEADER));
	imgWidth = bmpInfo.biWidth;
	imgHeight = bmpInfo.biHeight;

	//3. 이미지를 저장할 버퍼 할당 (2차원 배열) [이미지 높이 * 이미지 너비 만큼 할당]
	rgbBuffer = new RGBQUAD*[imgHeight];
	for (int i = 0; i < imgHeight; i++) {
		rgbBuffer[i] = new RGBQUAD[imgWidth];
	}

	//4. 이미지의 너비가 4의 배수인지 체크
	//BMP조건 가로는 4byte씩 이어야 한다.
	//한 픽셀이 3바이트(R, G, B)씩이니까, 가로(m_width) * 3이 4의 배수인가 아닌가를 알아야한다.
	//b4byte : 4byte배수인지 아닌지를 안다.
	//upbyte : 4byte배수에 모자라는 바이트다.
	bool b4byte = false;
	int upbyte = 0;
	if ((imgWidth * 3) % 4 == 0) {
		//4의 배수로 떨어지는 경우
		b4byte = true;
		upbyte = 0;
	}
	else {
		//4의 배수로 떨어지지않는 경우
		b4byte = false;
		upbyte = 4 - (imgWidth * 3) % 4;
	}

	//5. 픽셀 데이터를 파일로부터 읽어옴
	BYTE data[3];
	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			file.Read(&data, 3);
			//이미지가 거꾸로 저장되어 있기 때문에 거꾸로 읽어옴
			rgbBuffer[imgHeight - i - 1][j].rgbBlue = data[0];
			rgbBuffer[imgHeight - i - 1][j].rgbGreen = data[1];
			rgbBuffer[imgHeight - i - 1][j].rgbRed = data[2];
		}
		if (b4byte == false) {
			//가로가 4byte 배수가 아니면 쓰레기 값을 읽는다
			file.Read(&data, upbyte);
		}
	}
	file.Close(); //파일 닫기
	Invalidate(TRUE); //화면 갱신
}
