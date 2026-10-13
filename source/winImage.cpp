#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#include <limits>
#include <vector>
#include "winImage.h"

template<class T>
class C_COM_PTR {
public:
	C_COM_PTR() : m_pointer( NULL ) {}
	~C_COM_PTR() {
		if( m_pointer != NULL ) {
			m_pointer->Release();
		}
	}

	T* operator->() const {
		return m_pointer;
	}

	T* get() const {
		return m_pointer;
	}

	T** address() {
		if( m_pointer != NULL ) {
			m_pointer->Release();
			m_pointer = NULL;
		}
		return &m_pointer;
	}

private:
	C_COM_PTR( const C_COM_PTR& );
	C_COM_PTR& operator=( const C_COM_PTR& );

	T* m_pointer;
};

class C_COM_INITIALIZER {
public:
	explicit C_COM_INITIALIZER( bool ownsInitialization )
		: m_ownsInitialization( ownsInitialization ) {}

	~C_COM_INITIALIZER() {
		if( m_ownsInitialization ) {
			CoUninitialize();
		}
	}

private:
	bool m_ownsInitialization;
};

char* winimg_load_dib( const char* fileName )
{
	const HRESULT comResult = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if( FAILED( comResult ) && comResult != RPC_E_CHANGED_MODE ) {
		return NULL;
	}
	C_COM_INITIALIZER comInitializer( SUCCEEDED( comResult ) );

	const int pathLength = MultiByteToWideChar( CP_ACP, 0, fileName, -1, NULL, 0 );
	if( pathLength == 0 ) {
		return NULL;
	}

	std::vector<wchar_t> widePath( pathLength );
	if( MultiByteToWideChar( CP_ACP, 0, fileName, -1, &widePath[0], pathLength ) == 0 ) {
		return NULL;
	}

	C_COM_PTR<IWICImagingFactory> factory;
	IWICImagingFactory** factoryAddress = factory.address();
	if( FAILED( CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS( factoryAddress ) ) ) ) {
		return NULL;
	}

	C_COM_PTR<IWICBitmapDecoder> decoder;
	if( FAILED( factory->CreateDecoderFromFilename(
		&widePath[0],
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		decoder.address() ) ) ) {
		return NULL;
	}

	C_COM_PTR<IWICBitmapFrameDecode> frame;
	if( FAILED( decoder->GetFrame( 0, frame.address() ) ) ) {
		return NULL;
	}

	UINT width, height;
	if( FAILED( frame->GetSize( &width, &height ) ) ||
		width == 0 || height == 0 ||
		width > static_cast<UINT>( ( std::numeric_limits<UINT>::max() - 3 ) / 3 ) ||
		width > static_cast<UINT>( std::numeric_limits<LONG>::max() ) ||
		height > static_cast<UINT>( std::numeric_limits<LONG>::max() ) ) {
		return NULL;
	}

	const UINT stride = ( width * 3 + 3 ) & ~3U;
	if( height > std::numeric_limits<UINT>::max() / stride ) {
		return NULL;
	}
	const UINT imageSize = stride * height;
	if( imageSize > std::numeric_limits<SIZE_T>::max() - sizeof( BITMAPINFOHEADER ) ) {
		return NULL;
	}

	C_COM_PTR<IWICFormatConverter> converter;
	if( FAILED( factory->CreateFormatConverter( converter.address() ) ) ||
		FAILED( converter->Initialize(
			frame.get(),
			GUID_WICPixelFormat24bppBGR,
			WICBitmapDitherTypeNone,
			NULL,
			0.0,
			WICBitmapPaletteTypeCustom ) ) ) {
		return NULL;
	}

	const SIZE_T dibSize = sizeof( BITMAPINFOHEADER ) + imageSize;
	char* dib = static_cast<char*>( LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, dibSize ) );
	if( dib == NULL ) {
		return NULL;
	}

	BITMAPINFOHEADER* header = reinterpret_cast<BITMAPINFOHEADER*>( dib );
	header->biSize = sizeof( BITMAPINFOHEADER );
	header->biWidth = static_cast<LONG>( width );
	header->biHeight = static_cast<LONG>( height );
	header->biPlanes = 1;
	header->biBitCount = 24;
	header->biCompression = BI_RGB;
	header->biSizeImage = imageSize;

	BYTE* pixels = reinterpret_cast<BYTE*>( dib + sizeof( BITMAPINFOHEADER ) );
	if( FAILED( converter->CopyPixels( NULL, stride, imageSize, pixels ) ) ) {
		LocalFree( dib );
		return NULL;
	}

	for( UINT top = 0, bottom = height - 1; top < bottom; ++top, --bottom ) {
		BYTE* topRow = pixels + top * stride;
		BYTE* bottomRow = pixels + bottom * stride;
		for( UINT byteIndex = 0; byteIndex < stride; ++byteIndex ) {
			const BYTE value = topRow[ byteIndex ];
			topRow[ byteIndex ] = bottomRow[ byteIndex ];
			bottomRow[ byteIndex ] = value;
		}
	}

	return dib;
}
