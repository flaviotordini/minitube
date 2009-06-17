#include "imageblur.h"
#include "thpainter.h"
#include "thimage.h"

/* ============================================================================
 *  PUBLIC Constructor/Destructors
 */
THImage::THImage (const QSize& size, Format format)
	: QImage(size, format)
{
}

THImage::THImage (int width, int height, Format format)
	: QImage(width, height, format)
{
}

THImage::THImage (uchar *data, int width, int height, Format format)
	: QImage(data, width, height, format)
{
}

THImage::THImage (const uchar *data, int width, int height, Format format)
	: QImage(data, width, height, format)
{
}

THImage::THImage (uchar *data, int width, int height, int bytesPerLine, Format format)
	: QImage(data, width, height, bytesPerLine, format)
{
}

THImage::THImage (const uchar *data, int width, int height, int bytesPerLine, Format format)
	: QImage(data, width, height, bytesPerLine, format)
{
}

THImage::THImage (const QString& fileName, const char *format)
	: QImage(fileName, format)
{
}

THImage::THImage (const char *fileName, const char *format)
	: QImage(fileName, format)
{
}

THImage::THImage (const QImage& image)
	: QImage(image)
{
}

THImage::~THImage() {
}

/* ============================================================================
 *  PUBLIC Methods
 */
void THImage::expblur(int aprec, int zprec, int radius) {
	ImageBlur::expblur(this, aprec, zprec, radius);
}
#include <QCoreApplication>

void THImage::shadowBlur (int radius, const QColor& color) {
	ImageBlur::expblur(this, 16, 7, radius);

	THPainter p(this);
	p.setCompositionMode(QPainter::CompositionMode_SourceIn);
	p.fillRect(rect(), color);
	p.end();
}


