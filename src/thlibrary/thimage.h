#ifndef _THIMAGE_H_
#define _THIMAGE_H_

#include <QImage>

class THImage : public QImage {
	public:
		THImage (const QSize& size, Format format);
		THImage (int width, int height, Format format);
		THImage (uchar *data, int width, int height, Format format);
		THImage (const uchar *data, int width, int height, Format format);
		THImage (uchar *data, int width, int height, int bytesPerLine, Format format);
		THImage (const uchar *data, int width, int height, int bytesPerLine, Format format);
		THImage (const QString& fileName, const char *format = 0);
		THImage (const char *fileName, const char *format = 0);
		THImage (const QImage& image);
		~THImage();

	public:
		void expblur(int aprec, int zprec, int radius);
		void shadowBlur (int radius, const QColor& color);
};

#endif /* !_THIMAGE_H_ */


