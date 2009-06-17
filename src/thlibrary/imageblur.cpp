/*
 *   This is an adaptation of Jani Huhtanen Exponential blur code.
 *
 *   Copyright 2007 Jani Huhtanen <jani.huhtanen@tut.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QImage>
#include <cmath>

#include "imageblur.h"

/* ============================================================================
 *  PUBLIC Constructor/Destructors
 */
ImageBlur::ImageBlur (QImage *image, int aprec, int zprec) {
	m_image = image;
	m_aprec = aprec;
	m_zprec = zprec;
}

ImageBlur::~ImageBlur() {
	m_image = NULL;
}

/* ============================================================================
 *  PUBLIC STATIC Methods
 */
void ImageBlur::expblur (QImage *image, int aprec, int zprec, int radius) {
	ImageBlur imageBlur(image, aprec, zprec);
	imageBlur.expblur(radius);
}

/* ============================================================================
 *  PUBLIC Methods
 */
void ImageBlur::expblur (int radius) {
	if (radius < 1)
		return;

	/* Calculate the alpha such that 90% of
	 * the kernel is within the radius.
	 * (Kernel extends to infinity)
	 */
	int alpha = (int)((1 << m_aprec) * (1.0f - std::exp(-2.3f / (radius + 1.f))));

	for (int row = 0; row < m_image->height(); ++row)
		blurrow(row, alpha);

	for (int col = 0; col < m_image->width(); ++col)
		blurcol(col, alpha);
}

/* ============================================================================
 *  PRIVATE Methods
 */
void ImageBlur::blurcol (int col, int alpha) {
	int zR, zG, zB, zA;

	QRgb *ptr = (QRgb *)m_image->bits();
	ptr += col;

	zR = *((unsigned char *)ptr    ) << m_zprec;
	zG = *((unsigned char *)ptr + 1) << m_zprec;
	zB = *((unsigned char *)ptr + 2) << m_zprec;
	zA = *((unsigned char *)ptr + 3) << m_zprec;

	for (int index = m_image->width();
		 index < (m_image->height() - 1) * m_image->width();
		 index += m_image->width())
	{
		blurinner((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
	}

	for (int index = (m_image->height() - 2) * m_image->width();
		 index >= 0;
		 index -= m_image->width())
	{
		blurinner((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
	}
}

void ImageBlur::blurrow (int line, int alpha) {
	int zR, zG, zB, zA;

	QRgb *ptr = (QRgb *)m_image->scanLine(line);

	zR = *((unsigned char *)ptr    ) << m_zprec;
	zG = *((unsigned char *)ptr + 1) << m_zprec;
	zB = *((unsigned char *)ptr + 2) << m_zprec;
	zA = *((unsigned char *)ptr + 3) << m_zprec;

	for (int index = 1; index < m_image->width(); ++index)
		blurinner((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);

	for (int index = m_image->width() - 2; index >= 0; --index)
		blurinner((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
}

void ImageBlur::blurinner (	unsigned char *bptr,
							int &zR, int &zG, int &zB, int &zA,
							int alpha)
{
	int R, G, B, A;
	R = *bptr;
	G = *(bptr + 1);
	B = *(bptr + 2);
	A = *(bptr + 3);

	zR += (alpha * ((R << m_zprec) - zR)) >> m_aprec;
	zG += (alpha * ((G << m_zprec) - zG)) >> m_aprec;
	zB += (alpha * ((B << m_zprec) - zB)) >> m_aprec;
	zA += (alpha * ((A << m_zprec) - zA)) >> m_aprec;

	*bptr =     zR >> m_zprec;
	*(bptr+1) = zG >> m_zprec;
	*(bptr+2) = zB >> m_zprec;
	*(bptr+3) = zA >> m_zprec;
}

